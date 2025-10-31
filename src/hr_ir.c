// rio-riovn-merged/src/hr_ir.c
// L1 HRIR Implementation - Homoiconic Reversible IR

#include "hr_ir.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// =============================================================================
// BUILT-IN OPERATIONS
// =============================================================================

const char* HRIR_OP_ADD = "add";
const char* HRIR_OP_SUBTRACT = "subtract";
const char* HRIR_OP_MULTIPLY = "multiply";
const char* HRIR_OP_DIVIDE = "divide";
const char* HRIR_OP_EQUAL = "equal";
const char* HRIR_OP_LESS = "less";
const char* HRIR_OP_GREATER = "greater";
const char* HRIR_OP_JUMP = "jump";
const char* HRIR_OP_JUMP_IF = "jump_if";
const char* HRIR_OP_PRINT = "print";
const char* HRIR_OP_READ = "read";
const char* HRIR_OP_STORE = "store";
const char* HRIR_OP_LOAD = "load";

// =============================================================================
// CELL CREATION API
// =============================================================================

HRIR_Cell* hr_ir_create_cell(const char* opcode, const char** args, size_t arg_count) {
    if (!opcode) return NULL;

    HRIR_Cell* cell = calloc(1, sizeof(HRIR_Cell));
    if (!cell) return NULL;

    cell->opcode = strdup(opcode);
    if (!cell->opcode) {
        free(cell);
        return NULL;
    }

    cell->arg_count = arg_count;
    if (arg_count > 0) {
        cell->args = calloc(arg_count + 1, sizeof(char*)); // +1 for NULL terminator
        if (!cell->args) {
            free(cell->opcode);
            free(cell);
            return NULL;
        }

        for (size_t i = 0; i < arg_count; i++) {
            if (args[i]) {
                cell->args[i] = strdup(args[i]);
                if (!cell->args[i]) {
                    // Cleanup on failure
                    for (size_t j = 0; j < i; j++) {
                        free(cell->args[j]);
                    }
                    free(cell->args);
                    free(cell->opcode);
                    free(cell);
                    return NULL;
                }
            }
        }
    }

    cell->is_reversible = true; // Default to reversible
    cell->executed = false;
    cell->result = NULL;

    return cell;
}

HRIR_Cell* hr_ir_create_inverse(HRIR_Cell* cell) {
    if (!cell || !cell->is_reversible) return NULL;

    // Create inverse based on operation
    const char* inverse_opcode = NULL;
    if (strcmp(cell->opcode, HRIR_OP_ADD) == 0) {
        inverse_opcode = HRIR_OP_SUBTRACT;
    } else if (strcmp(cell->opcode, HRIR_OP_SUBTRACT) == 0) {
        inverse_opcode = HRIR_OP_ADD;
    } else if (strcmp(cell->opcode, HRIR_OP_MULTIPLY) == 0) {
        inverse_opcode = HRIR_OP_DIVIDE;
    } else if (strcmp(cell->opcode, HRIR_OP_DIVIDE) == 0) {
        inverse_opcode = HRIR_OP_MULTIPLY;
    } else {
        // For complex operations, return NULL (not invertible)
        return NULL;
    }

    return hr_ir_create_cell(inverse_opcode, cell->args, cell->arg_count);
}

void hr_ir_set_cell_meta(HRIR_Cell* cell, const char* source_location,
                        uint32_t line_number, const char* canonical_path) {
    if (!cell) return;

    if (source_location) {
        cell->source_location = strdup(source_location);
    }
    cell->line_number = line_number;
    if (canonical_path) {
        cell->canonical_path = strdup(canonical_path);
    }
}

void hr_ir_free_cell(HRIR_Cell* cell) {
    if (!cell) return;

    free((void*)cell->opcode);
    if (cell->args) {
        for (size_t i = 0; i < cell->arg_count; i++) {
            free((void*)cell->args[i]);
        }
        free(cell->args);
    }

    free((void*)cell->source_location);
    free((void*)cell->canonical_path);

    // Note: result cleanup depends on result type
    if (cell->result) {
        free(cell->result);
    }

    if (cell->inverse) {
        hr_ir_free_cell(cell->inverse);
    }

    free(cell);
}

// =============================================================================
// PROGRAM MANAGEMENT API
// =============================================================================

HRIR_Program* hr_ir_create_program(const char* source_name) {
    HRIR_Program* program = calloc(1, sizeof(HRIR_Program));
    if (!program) return NULL;

    program->cells = calloc(16, sizeof(HRIR_Cell*)); // Initial capacity 16
    if (!program->cells) {
        free(program);
        return NULL;
    }

    program->capacity = 16;
    program->cell_count = 0;
    program->pc = 0;
    program->tape = NULL;
    program->tape_size = 0;
    program->next_id = 1;

    if (source_name) {
        program->source_name = strdup(source_name);
    }

    return program;
}

bool hr_ir_add_cell(HRIR_Program* program, HRIR_Cell* cell) {
    if (!program || !cell) return false;

    // Expand capacity if needed
    if (program->cell_count >= program->capacity) {
        size_t new_capacity = program->capacity * 2;
        HRIR_Cell** new_cells = realloc(program->cells, new_capacity * sizeof(HRIR_Cell*));
        if (!new_cells) return false;

        program->cells = new_cells;
        program->capacity = new_capacity;
    }

    // Assign ID and add cell
    cell->id = program->next_id++;
    program->cells[program->cell_count++] = cell;

    // Ensure reversible cells have an inverse available for validation/debugging
    if (cell->is_reversible && cell->inverse == NULL) {
        HRIR_Cell* inv = hr_ir_create_inverse(cell);
        if (inv) {
            cell->inverse = inv; // Note: inverse->inverse is not set to avoid recursion
        }
    }

    return true;
}

HRIR_Cell* hr_ir_get_cell_by_id(HRIR_Program* program, uint32_t id) {
    if (!program) return NULL;

    for (size_t i = 0; i < program->cell_count; i++) {
        if (program->cells[i]->id == id) {
            return program->cells[i];
        }
    }
    return NULL;
}

HRIR_Cell* hr_ir_get_cell(HRIR_Program* program, size_t index) {
    if (!program || index >= program->cell_count) return NULL;
    return program->cells[index];
}

void hr_ir_free_program(HRIR_Program* program) {
    if (!program) return;

    for (size_t i = 0; i < program->cell_count; i++) {
        hr_ir_free_cell(program->cells[i]);
    }
    free(program->cells);

    if (program->tape) {
        for (size_t i = 0; i < program->tape_size; i++) {
            free(program->tape[i]);
        }
        free(program->tape);
    }

    free((void*)program->source_name);
    free(program);
}

// =============================================================================
// SERIALIZATION API
// =============================================================================

char* hr_ir_serialize_program(HRIR_Program* program) {
    if (!program) return NULL;

    // Simple JSON serialization
    size_t buffer_size = 4096;
    char* json = calloc(1, buffer_size);
    if (!json) return NULL;

    size_t pos = 0;
    pos += snprintf(json + pos, buffer_size - pos, "{\n");
    pos += snprintf(json + pos, buffer_size - pos, "  \"source_name\": \"%s\",\n",
                   program->source_name ? program->source_name : "");
    pos += snprintf(json + pos, buffer_size - pos, "  \"cell_count\": %zu,\n", program->cell_count);
    pos += snprintf(json + pos, buffer_size - pos, "  \"cells\": [\n");

    for (size_t i = 0; i < program->cell_count; i++) {
        HRIR_Cell* cell = program->cells[i];
        pos += snprintf(json + pos, buffer_size - pos, "    {\n");
        pos += snprintf(json + pos, buffer_size - pos, "      \"id\": %u,\n", cell->id);
        pos += snprintf(json + pos, buffer_size - pos, "      \"opcode\": \"%s\",\n", cell->opcode);
        pos += snprintf(json + pos, buffer_size - pos, "      \"args\": [");

        for (size_t j = 0; j < cell->arg_count; j++) {
            pos += snprintf(json + pos, buffer_size - pos, "\"%s\"", cell->args[j]);
            if (j < cell->arg_count - 1) pos += snprintf(json + pos, buffer_size - pos, ", ");
        }

        pos += snprintf(json + pos, buffer_size - pos, "],\n");
        pos += snprintf(json + pos, buffer_size - pos, "      \"is_reversible\": %s,\n",
                       cell->is_reversible ? "true" : "false");
        pos += snprintf(json + pos, buffer_size - pos, "      \"executed\": %s\n",
                       cell->executed ? "true" : "false");
        pos += snprintf(json + pos, buffer_size - pos, "    }");

        if (i < program->cell_count - 1) {
            pos += snprintf(json + pos, buffer_size - pos, ",");
        }
        pos += snprintf(json + pos, buffer_size - pos, "\n");
    }

    pos += snprintf(json + pos, buffer_size - pos, "  ]\n");
    pos += snprintf(json + pos, buffer_size - pos, "}\n");

    return json;
}

// =============================================================================
// RUNTIME EXECUTION API
// =============================================================================

HRIR_Runtime* hr_ir_create_runtime(HRIR_Program* program) {
    if (!program) return NULL;

    HRIR_Runtime* runtime = calloc(1, sizeof(HRIR_Runtime));
    if (!runtime) return NULL;

    runtime->program = program;
    runtime->checkpoint = 0;
    runtime->steps_executed = 0;
    runtime->rollbacks = 0;
    runtime->last_error = NULL;

    return runtime;
}

bool hr_ir_step(HRIR_Runtime* runtime) {
    if (!runtime || !runtime->program) return false;

    if (runtime->program->pc >= runtime->program->cell_count) {
        return false; // Program complete
    }

    HRIR_Cell* cell = runtime->program->cells[runtime->program->pc];

    // Simple execution simulation
    // In a real implementation, this would execute the actual operation
    cell->executed = true;
    cell->result = strdup("executed"); // Placeholder result

    runtime->steps_executed++;
    runtime->program->pc++;

    return true;
}

bool hr_ir_run(HRIR_Runtime* runtime) {
    if (!runtime) return false;

    while (hr_ir_step(runtime)) {
        // Continue until completion or error
    }

    return runtime->program->pc >= runtime->program->cell_count;
}

bool hr_ir_undo(HRIR_Runtime* runtime) {
    if (!runtime || !runtime->program || runtime->program->pc == 0) {
        return false;
    }

    runtime->program->pc--;
    HRIR_Cell* cell = runtime->program->cells[runtime->program->pc];

    // Undo execution
    cell->executed = false;
    if (cell->result) {
        free(cell->result);
        cell->result = NULL;
    }

    runtime->steps_executed--;
    runtime->rollbacks++;

    return true;
}

bool hr_ir_checkpoint(HRIR_Runtime* runtime) {
    if (!runtime) return false;

    runtime->checkpoint = runtime->program->pc;
    return true;
}

bool hr_ir_rollback(HRIR_Runtime* runtime) {
    if (!runtime) return false;

    while (runtime->program->pc > runtime->checkpoint) {
        if (!hr_ir_undo(runtime)) return false;
    }

    return true;
}

size_t hr_ir_get_pc(HRIR_Runtime* runtime) {
    return runtime && runtime->program ? runtime->program->pc : 0;
}

bool hr_ir_is_complete(HRIR_Runtime* runtime) {
    return runtime && runtime->program &&
           runtime->program->pc >= runtime->program->cell_count;
}

void hr_ir_free_runtime(HRIR_Runtime* runtime) {
    if (!runtime) return;

    // Note: program is owned by caller, don't free here
    free((void*)runtime->last_error);
    free(runtime);
}

// =============================================================================
// COMPILER INTEGRATION API
// =============================================================================

HRIR_Cell* hr_ir_from_send_operation(const char* target, const char* selector,
                                    const char** args, size_t arg_count) {
    if (!target || !selector) return NULL;

    // Map selector to HRIR opcode
    const char* opcode = NULL;
    if (strcmp(selector, "add") == 0) opcode = HRIR_OP_ADD;
    else if (strcmp(selector, "subtract") == 0) opcode = HRIR_OP_SUBTRACT;
    else if (strcmp(selector, "multiply") == 0) opcode = HRIR_OP_MULTIPLY;
    else if (strcmp(selector, "divide") == 0) opcode = HRIR_OP_DIVIDE;
    else if (strcmp(selector, "output") == 0) opcode = HRIR_OP_PRINT;
    else return NULL; // Unknown operation

    return hr_ir_create_cell(opcode, args, arg_count);
}

HRIR_Cell* hr_ir_from_d_term_operation(const char* operation,
                                      const char** args, size_t arg_count) {
    if (!operation) return NULL;

    HRIR_Cell* cell = hr_ir_create_cell(operation, args, arg_count);
    if (cell) {
        cell->is_reversible = false; // D-term operations are not reversible
    }
    return cell;
}

// =============================================================================
// DEBUGGING & INSPECTION API
// =============================================================================

HRIR_Stats hr_ir_get_stats(HRIR_Program* program) {
    HRIR_Stats stats = {0};

    if (!program) return stats;

    stats.total_cells = program->cell_count;

    for (size_t i = 0; i < program->cell_count; i++) {
        HRIR_Cell* cell = program->cells[i];
        if (cell->is_reversible) {
            stats.r_term_cells++;
        } else {
            stats.d_term_cells++;
        }
        if (cell->executed) {
            stats.executed_cells++;
        }
    }

    return stats;
}

void hr_ir_dump_program(HRIR_Program* program) {
    if (!program) {
        printf("HRIR Program: NULL\n");
        return;
    }

    printf("HRIR Program: %s\n", program->source_name ? program->source_name : "<unnamed>");
    printf("  Cells: %zu\n", program->cell_count);
    printf("  PC: %zu\n", program->pc);

    for (size_t i = 0; i < program->cell_count; i++) {
        HRIR_Cell* cell = program->cells[i];
        printf("  [%zu] %s(", i, cell->opcode);
        for (size_t j = 0; j < cell->arg_count; j++) {
            printf("%s", cell->args[j]);
            if (j < cell->arg_count - 1) printf(", ");
        }
        printf(") %s %s\n",
               cell->is_reversible ? "[R]" : "[D]",
               cell->executed ? "[EXEC]" : "[PENDING]");
    }
}

void hr_ir_dump_runtime(HRIR_Runtime* runtime) {
    if (!runtime) {
        printf("HRIR Runtime: NULL\n");
        return;
    }

    printf("HRIR Runtime:\n");
    printf("  Steps executed: %zu\n", runtime->steps_executed);
    printf("  Rollbacks: %zu\n", runtime->rollbacks);
    printf("  Checkpoint: %zu\n", runtime->checkpoint);

    if (runtime->program) {
        hr_ir_dump_program(runtime->program);
    }
}

// =============================================================================
// ERROR HANDLING
// =============================================================================

HRIR_Error hr_ir_get_last_error(HRIR_Runtime* runtime) {
    // Simplified - no error tracking implemented yet
    return HRIR_SUCCESS;
}

const char* hr_ir_get_error_message(HRIR_Error error) {
    switch (error) {
        case HRIR_SUCCESS: return "Success";
        case HRIR_ERROR_INVALID_CELL: return "Invalid cell";
        case HRIR_ERROR_INVALID_PROGRAM: return "Invalid program";
        case HRIR_ERROR_EXECUTION_FAILED: return "Execution failed";
        case HRIR_ERROR_MEMORY_ALLOCATION: return "Memory allocation failed";
        case HRIR_ERROR_INVALID_OPERATION: return "Invalid operation";
        case HRIR_ERROR_IRREVERSIBLE_OPERATION: return "Operation is irreversible";
        case HRIR_ERROR_CHECKPOINT_NOT_FOUND: return "Checkpoint not found";
        default: return "Unknown error";
    }
}
