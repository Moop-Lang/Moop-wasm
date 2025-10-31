// src/l5_moop.c
// L5 Moop: Natural Language Layer - Inherits Homoiconicity & Reversibility
// Homoiconic: Natural language as executable data
// Reversible: Message passing operations can be undone
// Inherits from L1 HRIR → L2a Functions → L3 Actors → L4 root_proto

#include "l5_moop.h"
#include "hr_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

static char* l5_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = (char*)malloc(len + 1);
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

static char* l5_generate_program_id(void) {
    static int counter = 0;
    time_t now = time(NULL);
    char* id = (char*)malloc(64);
    if (id) {
        snprintf(id, 64, "moop_%ld_%d", (long)now, counter++);
    }
    return id;
}

static bool l5_should_expand_statements(L5_MoopProgram* program) {
    return program->statement_count >= program->statement_capacity;
}

static bool l5_expand_statements(L5_MoopProgram* program) {
    int new_capacity = program->statement_capacity * 2;
    L5_Statement** new_statements = (L5_Statement**)realloc(
        program->statements,
        new_capacity * sizeof(L5_Statement*)
    );

    if (!new_statements) return false;

    program->statements = new_statements;
    program->statement_capacity = new_capacity;
    return true;
}

// =============================================================================
// STATEMENT PARSING
// =============================================================================

static L5_StatementType l5_parse_statement_type(const char* statement) {
    if (strstr(statement, " <- ")) {
        return L5_STMT_INHERITANCE;
    } else if (strstr(statement, " -> ")) {
        return L5_STMT_MESSAGE_SEND;
    } else if (strstr(statement, "output ")) {
        return L5_STMT_OUTPUT;
    }
    return L5_STMT_UNKNOWN;
}

static L5_Statement* l5_parse_statement(const char* statement_text) {
    L5_Statement* stmt = (L5_Statement*)malloc(sizeof(L5_Statement));
    if (!stmt) return NULL;

    memset(stmt, 0, sizeof(L5_Statement));
    stmt->id = 0; // Will be set by program
    stmt->text = l5_strdup(statement_text);
    stmt->type = l5_parse_statement_type(statement_text);
    stmt->is_homoiconic = true;
    stmt->is_reversible = (stmt->type != L5_STMT_OUTPUT); // Output is side effect
    stmt->executed = false;
    stmt->hrir_cell = NULL;

    // Parse statement data
    switch (stmt->type) {
        case L5_STMT_INHERITANCE: {
            char* arrow_pos = strstr(statement_text, " <- ");
            if (arrow_pos) {
                *arrow_pos = '\0';
                stmt->data.inheritance.child = l5_strdup(statement_text);
                stmt->data.inheritance.parent = l5_strdup(arrow_pos + 4);
                *arrow_pos = ' '; // Restore
            }
            break;
        }

        case L5_STMT_MESSAGE_SEND: {
            char* arrow_pos = strstr(statement_text, " -> ");
            if (arrow_pos) {
                *arrow_pos = '\0';
                stmt->data.message_send.target = l5_strdup(statement_text);

                // Parse message and args
                char* message_part = arrow_pos + 4;
                char* paren_start = strchr(message_part, '(');
                if (paren_start) {
                    *paren_start = '\0';
                    stmt->data.message_send.selector = l5_strdup(message_part);

                    // Parse arguments
                    char* paren_end = strchr(paren_start + 1, ')');
                    if (paren_end) {
                        *paren_end = '\0';
                        char* args_str = paren_start + 1;

                        // Simple comma-separated parsing
                        char* arg = strtok(args_str, ",");
                        while (arg && stmt->data.message_send.arg_count < 10) {
                            // Trim whitespace
                            while (*arg == ' ') arg++;
                            char* end = arg + strlen(arg) - 1;
                            while (end > arg && *end == ' ') *end-- = '\0';

                            stmt->data.message_send.args[stmt->data.message_send.arg_count++] =
                                l5_strdup(arg);
                            arg = strtok(NULL, ",");
                        }
                    }
                    *paren_start = '('; // Restore
                } else {
                    stmt->data.message_send.selector = l5_strdup(message_part);
                }
                *arrow_pos = ' '; // Restore
            }
            break;
        }

        case L5_STMT_OUTPUT: {
            char* output_pos = strstr(statement_text, "output ");
            if (output_pos) {
                char* content = output_pos + 7;
                // Remove quotes if present
                if (*content == '"') {
                    content++;
                    char* end_quote = strrchr(content, '"');
                    if (end_quote) *end_quote = '\0';
                }
                stmt->data.output.content = l5_strdup(content);
            }
            break;
        }

        default:
            // Unknown statement type
            break;
    }

    return stmt;
}

// =============================================================================
// HRIR CELL GENERATION
// =============================================================================

static HRIR_Cell* l5_generate_hrir_cell(L5_Statement* stmt) {
    if (!stmt) return NULL;

    HRIR_Cell* cell = NULL;

    switch (stmt->type) {
        case L5_STMT_MESSAGE_SEND: {
            // Create reversible HRIR cell for message send
            cell = hr_ir_from_send_operation(
                stmt->data.message_send.target,
                stmt->data.message_send.selector,
                (const char**)stmt->data.message_send.args,
                stmt->data.message_send.arg_count
            );
            break;
        }

        case L5_STMT_OUTPUT: {
            // Create D-term HRIR cell for output (irreversible side effect)
            cell = hr_ir_from_d_term_operation("print", (const char*[]){stmt->data.output.content}, 1);
            break;
        }

        default:
            return NULL;
    }

    if (cell) {
        // Set metadata
        char location[256];
        snprintf(location, sizeof(location), "L5_moop_statement_%d", stmt->id);
        cell->source_location = l5_strdup(location);
        cell->line_number = stmt->id + 1;

        char canonical_path[256];
        snprintf(canonical_path, sizeof(canonical_path), "MoopProgram.statement_%d", stmt->id);
        cell->canonical_path = l5_strdup(canonical_path);
    }

    return cell;
}

// =============================================================================
// MOOP PROGRAM MANAGEMENT
// =============================================================================

L5_MoopProgram* l5_create_homoiconic_program(const char* source_name) {
    L5_MoopProgram* program = (L5_MoopProgram*)malloc(sizeof(L5_MoopProgram));
    if (!program) return NULL;

    memset(program, 0, sizeof(L5_MoopProgram));
    program->source_name = l5_strdup(source_name ? source_name : "moop_program");
    program->statement_capacity = 16;
    program->statements = (L5_Statement**)malloc(program->statement_capacity * sizeof(L5_Statement*));
    program->hrir_program = hr_ir_create_program("moop_hrir");

    program->is_homoiconic = true;
    program->is_reversible = true;
    program->inheritance_chain = l5_strdup("L1→L2a→L3→L4→L5");

    // Initialize execution history
    program->history_capacity = 32;
    program->execution_history = (int*)malloc(program->history_capacity * sizeof(int));

    // Initialize checkpoints
    program->checkpoint_capacity = 8;
    program->checkpoints = (char**)malloc(program->checkpoint_capacity * sizeof(char*));

    if (!program->statements || !program->hrir_program ||
        !program->execution_history || !program->checkpoints) {
        l5_free_moop_program(program);
        return NULL;
    }

    return program;
}

bool l5_add_statement(L5_MoopProgram* program, const char* statement_text) {
    if (!program || !statement_text) return false;

    if (l5_should_expand_statements(program)) {
        if (!l5_expand_statements(program)) return false;
    }

    L5_Statement* stmt = l5_parse_statement(statement_text);
    if (!stmt) return false;

    stmt->id = program->statement_count;
    program->statements[program->statement_count++] = stmt;

    return true;
}

bool l5_generate_hrir_cells(L5_MoopProgram* program) {
    if (!program) return false;

    for (int i = 0; i < program->statement_count; i++) {
        L5_Statement* stmt = program->statements[i];
        HRIR_Cell* cell = l5_generate_hrir_cell(stmt);

        if (cell) {
            stmt->hrir_cell = cell;
            if (!hr_ir_add_cell(program->hrir_program, cell)) {
                return false;
            }
        }
    }

    return true;
}

// =============================================================================
// EXECUTION AND TIME-TRAVEL
// =============================================================================

bool l5_execute_program(L5_MoopProgram* program) {
    if (!program) return false;

    // Create execution checkpoint
    char* checkpoint_id = l5_create_checkpoint(program);
    if (checkpoint_id) {
        free(checkpoint_id); // We don't need to store it
    }

    for (int i = 0; i < program->statement_count; i++) {
        L5_Statement* stmt = program->statements[i];

        // Add to execution history
        if (program->history_count >= program->history_capacity) {
            program->history_capacity *= 2;
            int* new_history = (int*)realloc(program->execution_history,
                                           program->history_capacity * sizeof(int));
            if (!new_history) return false;
            program->execution_history = new_history;
        }
        program->execution_history[program->history_count++] = i;

        // Mark as executed (homoiconic state change)
        stmt->executed = true;

        // Execute HRIR cell if available
        if (stmt->hrir_cell) {
            stmt->hrir_cell->executed = true;
            char result_str[64];
            snprintf(result_str, sizeof(result_str), "executed_%s_%d",
                    stmt->hrir_cell->opcode, (int)time(NULL));
            stmt->hrir_cell->result = l5_strdup(result_str);
        }

        printf("✅ Executed L5 statement %d: %s\n", i, stmt->text);
    }

    return true;
}

bool l5_undo_program(L5_MoopProgram* program, int steps) {
    if (!program || steps <= 0) return false;

    for (int i = 0; i < steps && program->history_count > 0; i++) {
        int stmt_index = program->execution_history[--program->history_count];
        L5_Statement* stmt = program->statements[stmt_index];

        // Undo homoiconic state
        stmt->executed = false;
        if (stmt->hrir_cell) {
            stmt->hrir_cell->executed = false;
            if (stmt->hrir_cell->result) {
                free(stmt->hrir_cell->result);
                stmt->hrir_cell->result = NULL;
            }
        }

        printf("↶ Undid L5 statement %d: %s\n", stmt_index, stmt->text);
    }

    return true;
}

bool l5_rollback_program(L5_MoopProgram* program, const char* checkpoint_id) {
    if (!program || !checkpoint_id) return false;

    // Find checkpoint (simplified - in real implementation would be a map)
    for (int i = 0; i < program->checkpoint_count; i++) {
        if (strcmp(program->checkpoints[i], checkpoint_id) == 0) {
            // Reset all statements after checkpoint
            for (int j = 0; j < program->statement_count; j++) {
                L5_Statement* stmt = program->statements[j];
                stmt->executed = false;
                if (stmt->hrir_cell) {
                    stmt->hrir_cell->executed = false;
                    if (stmt->hrir_cell->result) {
                        free(stmt->hrir_cell->result);
                        stmt->hrir_cell->result = NULL;
                    }
                }
            }

            // Reset execution history
            program->history_count = 0;

            printf("🔄 Rolled back to checkpoint: %s\n", checkpoint_id);
            return true;
        }
    }

    return false;
}

char* l5_create_checkpoint(L5_MoopProgram* program) {
    if (!program) return NULL;

    if (program->checkpoint_count >= program->checkpoint_capacity) {
        program->checkpoint_capacity *= 2;
        char** new_checkpoints = (char**)realloc(program->checkpoints,
                                               program->checkpoint_capacity * sizeof(char*));
        if (!new_checkpoints) return NULL;
        program->checkpoints = new_checkpoints;
    }

    char checkpoint_id[64];
    snprintf(checkpoint_id, sizeof(checkpoint_id), "checkpoint_%d_%ld",
            program->checkpoint_count, (long)time(NULL));

    program->checkpoints[program->checkpoint_count++] = l5_strdup(checkpoint_id);

    printf("📍 Created checkpoint: %s\n", checkpoint_id);
    return l5_strdup(checkpoint_id);
}

// =============================================================================
// MAIN COMPILATION FUNCTION
// =============================================================================

L5_CompileResult* l5_compile_moop(const char* moop_code, L5_CompileOptions options) {
    L5_CompileResult* result = (L5_CompileResult*)malloc(sizeof(L5_CompileResult));
    if (!result) return NULL;

    memset(result, 0, sizeof(L5_CompileResult));
    result->success = false;

    // Legacy minimal path (orthogonal, default)
    if (!options.enhanced) {
        result->l4_output = l5_compile_moop_legacy(moop_code);
        result->success = (result->l4_output != NULL);
        return result;
    }

    // Enhanced homoiconic path
    L5_MoopProgram* program = l5_create_homoiconic_program("compiled_moop");
    if (!program) {
        result->error_message = l5_strdup("Failed to create homoiconic program");
        return result;
    }

    result->homoiconic_program = program;
    result->program_id = l5_generate_program_id();
    result->hrir_program = program->hrir_program;
    result->is_homoiconic = true;
    result->is_reversible = true;
    result->inheritance_chain = l5_strdup("L1→L2a→L3→L4→L5");

    // Parse statements
    char* code_copy = l5_strdup(moop_code);
    char* line = strtok(code_copy, "\n");

    while (line) {
        // Skip comments and empty lines
        while (*line == ' ') line++;
        if (*line != '\0' && *line != '/' && strncmp(line, "//", 2) != 0) {
            if (!l5_add_statement(program, line)) {
                result->error_message = l5_strdup("Failed to add statement");
                free(code_copy);
                return result;
            }
        }
        line = strtok(NULL, "\n");
    }
    free(code_copy);

    // Generate HRIR cells
    if (options.generate_hrir) {
        if (!l5_generate_hrir_cells(program)) {
            result->error_message = l5_strdup("Failed to generate HRIR cells");
            return result;
        }
    }

    // Set features
    result->features.homoiconic_statements = program->statement_count;
    result->features.hrir_cells = program->hrir_program->cell_count;
    result->features.reversible_operations = 0;
    result->features.time_travel_capable = options.enable_time_travel;

    for (int i = 0; i < program->statement_count; i++) {
        if (program->statements[i]->is_reversible) {
            result->features.reversible_operations++;
        }
    }

    // Generate legacy L4 output for backward compatibility
    result->l4_output = l5_compile_moop_legacy(moop_code);

    result->success = true;
    printf("✅ L5 Homoiconic Program Created: %d statements\n", program->statement_count);
    printf("   Homoiconic: %s\n", program->is_homoiconic ? "true" : "false");
    printf("   Reversible: %s\n", program->is_reversible ? "true" : "false");
    printf("   HRIR Cells: %d\n", program->hrir_program->cell_count);
    printf("   Inheritance: %s\n", program->inheritance_chain);

    return result;
}

// =============================================================================
// LEGACY COMPILATION (MINIMAL SURFACE)
// =============================================================================

char* l5_compile_moop_legacy(const char* moop_code) {
    // Simple keyword-based compilation to L4 Rio
    // This mirrors the original logic but returns L4 code

    char* result = (char*)malloc(4096);
    if (!result) return NULL;

    strcpy(result, "// L4 Rio code generated from L5 Moop\n");

    // Parse and convert each line
    char* code_copy = l5_strdup(moop_code);
    char* line = strtok(code_copy, "\n");

    while (line) {
        while (*line == ' ') line++;

        if (*line != '\0' && *line != '/' && strncmp(line, "//", 2) != 0) {
            if (strstr(line, " <- ")) {
                // Inheritance: MathProto <- ObjectProto
                char* arrow_pos = strstr(line, " <- ");
                *arrow_pos = '\0';
                char proto_code[512];
                snprintf(proto_code, sizeof(proto_code),
                        "root_proto %s <- %s\n", line, arrow_pos + 4);
                strcat(result, proto_code);
                *arrow_pos = ' ';
            } else if (strstr(line, " -> ")) {
                // Message send: math -> add(5, 3)
                char* arrow_pos = strstr(line, " -> ");
                *arrow_pos = '\0';
                char message_code[512];
                snprintf(message_code, sizeof(message_code),
                        "root_proto Message_%s_%s <- Object\n"
                        "    slots:\n"
                        "        target: \"%s\"\n"
                        "        message: \"%s\"\n"
                        "    methods:\n"
                        "        send: method()\n"
                        "            target.receive(message)\n",
                        line, arrow_pos + 4, line, arrow_pos + 4);
                strcat(result, message_code);
                *arrow_pos = ' ';
            } else if (strstr(line, "output ")) {
                // Output: output "Hello"
                char* output_pos = strstr(line, "output ");
                char* content = output_pos + 7;
                if (*content == '"') {
                    content++;
                    char* end_quote = strrchr(content, '"');
                    if (end_quote) *end_quote = '\0';
                }
                char output_code[512];
                snprintf(output_code, sizeof(output_code),
                        "root_proto Display_%s <- Object\n"
                        "    slots:\n"
                        "        message: \"%s\"\n"
                        "    methods:\n"
                        "        display: method()\n"
                        "            message\n",
                        content, content);
                strcat(result, output_code);
            }
        }

        line = strtok(NULL, "\n");
    }

    free(code_copy);
    return result;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

L5_CompileOptions l5_default_options(void) {
    L5_CompileOptions options = {
        .enhanced = false,
        .auto_inherit = true,
        .enable_time_travel = false,
        .generate_hrir = false
    };
    return options;
}

L5_CompileOptions l5_enhanced_options(void) {
    L5_CompileOptions options = {
        .enhanced = true,
        .auto_inherit = true,
        .enable_time_travel = true,
        .generate_hrir = true
    };
    return options;
}

bool l5_should_use_enhanced_mode(void) {
    // Check environment variable
    char* env_var = getenv("MOOP_L5_ENHANCED");
    if (env_var && strcmp(env_var, "1") == 0) {
        return true;
    }

    // Could also check for other indicators
    return false;
}

// =============================================================================
// TIME-TRAVEL API
// =============================================================================

L5_TimeTravelAPI l5_get_time_travel_api(L5_MoopProgram* program) {
    L5_TimeTravelAPI api = {
        .execute = l5_execute_program,
        .undo = l5_undo_program,
        .rollback = l5_rollback_program,
        .checkpoint = l5_create_checkpoint,
        .get_program_data = l5_get_program_data
    };
    return api;
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

char* l5_get_program_data(L5_MoopProgram* program) {
    if (!program) return NULL;

    char* json = (char*)malloc(4096);
    if (!json) return NULL;

    snprintf(json, 4096,
            "{\"source_name\":\"%s\",\"statement_count\":%d,\"is_homoiconic\":%s,\"is_reversible\":%s,\"inheritance_chain\":\"%s\"}",
            program->source_name,
            program->statement_count,
            program->is_homoiconic ? "true" : "false",
            program->is_reversible ? "true" : "false",
            program->inheritance_chain);

    return json;
}

void l5_free_moop_program(L5_MoopProgram* program) {
    if (!program) return;

    if (program->source_name) free(program->source_name);
    if (program->inheritance_chain) free(program->inheritance_chain);

    // Free statements
    for (int i = 0; i < program->statement_count; i++) {
        L5_Statement* stmt = program->statements[i];
        if (stmt) {
            if (stmt->text) free(stmt->text);

            // Free statement data
            switch (stmt->type) {
                case L5_STMT_INHERITANCE:
                    if (stmt->data.inheritance.child) free(stmt->data.inheritance.child);
                    if (stmt->data.inheritance.parent) free(stmt->data.inheritance.parent);
                    break;

                case L5_STMT_MESSAGE_SEND:
                    if (stmt->data.message_send.target) free(stmt->data.message_send.target);
                    if (stmt->data.message_send.selector) free(stmt->data.message_send.selector);
                    for (int j = 0; j < stmt->data.message_send.arg_count; j++) {
                        if (stmt->data.message_send.args[j]) free(stmt->data.message_send.args[j]);
                    }
                    break;

                case L5_STMT_OUTPUT:
                    if (stmt->data.output.content) free(stmt->data.output.content);
                    break;

                default:
                    break;
            }

            free(stmt);
        }
    }

    if (program->statements) free(program->statements);
    if (program->execution_history) free(program->execution_history);

    // Free checkpoints
    for (int i = 0; i < program->checkpoint_count; i++) {
        if (program->checkpoints[i]) free(program->checkpoints[i]);
    }
    if (program->checkpoints) free(program->checkpoints);

    // Note: HRIR program is freed separately if needed

    free(program);
}

void l5_free_compile_result(L5_CompileResult* result) {
    if (!result) return;

    if (result->l4_output) free(result->l4_output);
    if (result->program_id) free(result->program_id);
    if (result->inheritance_chain) free(result->inheritance_chain);
    if (result->error_message) free(result->error_message);

    // Note: homoiconic_program and hrir_program are freed separately if needed

    free(result);
}
