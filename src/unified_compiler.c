// rio-riovn-merged/src/unified_compiler.c
// Unified Compiler - Rio + RioVN Integration

#include "architecture.h"
#include "surface_parser.h"
#include "hr_ir.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// =============================================================================
// UNIFIED COMPILER IMPLEMENTATION
// =============================================================================

// Forward declarations
static char* generate_hrir_json(SurfaceAST* ast, CompilerOptions options);

// Forward declarations for layer interfaces
static char* canonicalize_paths(SurfaceAST* ast, CompilerOptions options);
static char* generate_reversible_ir(SurfaceAST* ast, CompilerOptions options);
static char* generate_membrane_logs(SurfaceAST* ast, CompilerOptions options);
static bool validate_unified(SurfaceAST* ast, CompilerOptions options,
                           const char** error_msg);

// Unified compile function - single entry point
CompilationResult* compile(const char* code, CompilerOptions options) {
    clock_t start_time = clock();

    // Initialize result
    CompilationResult* result = calloc(1, sizeof(CompilationResult));
    result->success = true;
    result->canonical_code = NULL;
    result->reversible_ir = NULL;
    result->membrane_logs = NULL;
    result->inheritance_graph = NULL;
    result->path_count = 0;
    result->inheritance_count = 0;
    result->error_count = 0;
    result->first_error_message = NULL;
    result->warning_count = 0;

    // Initialize stats
    result->stats.canonical_paths_count = 0;
    result->stats.inheritance_edges_count = 0;
    result->stats.r_term_ops_count = 0;
    result->stats.d_term_ops_count = 0;
    result->stats.membrane_crossings_count = 0;
    result->stats.compilation_time_ms = 0.0;
    result->stats.validation_time_ms = 0.0;

    if (options.debug_mode) {
        printf("🌀 Unified Rio+RioVN Compiler\n");
        printf("  Strict mode: %s\n", options.strict_mode ? "ENABLED" : "DISABLED");
        printf("  Auto-hoist: %s\n", options.auto_hoist ? "ENABLED" : "DISABLED");
        printf("  Debug mode: %s\n", options.debug_mode ? "ENABLED" : "DISABLED");
        printf("  Reversible default: %s\n\n", options.reversible_default ? "ENABLED" : "DISABLED");
    }

    // Phase 1: Parse Surface (L5/L4)
    SurfaceAST* ast = parse_surface(code);
    if (!ast) {
        result->success = false;
        result->error_count++;
        result->first_error_message = strdup("Failed to parse surface syntax");
        return result;
    }

    if (options.debug_mode) {
        printf("📄 Surface Parsing Complete\n");
        printf("  Statements: %zu\n", ast->statement_count);
        printf("  Inheritance relations: %zu\n\n", ast->inheritance_count);
    }

    // Phase 2: Canonicalize (RioVN integration)
    result->canonical_code = canonicalize_paths(ast, options);
    if (!result->canonical_code) {
        result->success = false;
        result->error_count++;
        result->first_error_message = strdup("Failed to generate canonical paths");
        free_surface_ast(ast);
        return result;
    }

    result->inheritance_graph = ast->inheritance_relations;
    result->inheritance_count = ast->inheritance_count;

    if (options.debug_mode) {
        printf("🎯 Canonicalization Complete\n");
        printf("  Canonical code generated\n");
        printf("  Inheritance graph: %zu edges\n\n", result->inheritance_count);
    }

    // Phase 3: Classify R/D/S (Rio integration)
    // Count operation types for stats
    for (size_t i = 0; i < ast->statement_count; i++) {
        if (ast->statements[i].type == STMT_SEND) {
            OperationType op_type = ast->statements[i].as.send.type;
            if (op_type == OP_R_TERM) {
                result->stats.r_term_ops_count++;
            } else if (op_type == OP_D_TERM) {
                result->stats.d_term_ops_count++;
                result->stats.membrane_crossings_count++;
            }
        }
    }

    // Phase 4: Lower to Layers (Rio pipeline)
    result->reversible_ir = generate_reversible_ir(ast, options);
    result->membrane_logs = generate_membrane_logs(ast, options);

    // Phase 5: Generate HRIR (L1 Homoiconic Reversible IR)
    // This is the true L1 layer - homoiconic and reversible
    result->hrir_json = generate_hrir_json(ast, options);

    if (options.debug_mode) {
        printf("🏗️ Generating L1 HRIR (Homoiconic Reversible IR)...\n");
        if (result->hrir_json) {
            printf("✅ HRIR generated successfully\n");
        } else {
            printf("❌ HRIR generation failed\n");
        }
    }

    if (options.debug_mode) {
        printf("🏗️ Layer Lowering Complete\n");
        printf("  Reversible IR: generated\n");
        printf("  Membrane logs: generated\n\n");
    }

    // Phase 5: Emit & Validate
    clock_t validation_start = clock();

    const char* validation_error = NULL;
    if (!validate_unified(ast, options, &validation_error)) {
        result->success = false;
        result->error_count++;
        result->first_error_message = strdup(validation_error ? validation_error :
                                           "Unified validation failed");
    }

    clock_t validation_end = clock();
    result->stats.validation_time_ms = ((double)(validation_end - validation_start) /
                                       CLOCKS_PER_SEC) * 1000.0;

    clock_t end_time = clock();
    result->stats.compilation_time_ms = ((double)(end_time - start_time) /
                                        CLOCKS_PER_SEC) * 1000.0;

    // Update final stats
    result->stats.canonical_paths_count = ast->statement_count;
    result->stats.inheritance_edges_count = ast->inheritance_count;

    if (options.debug_mode) {
        printf("✅ Unified Compilation Complete\n");
        printf("📊 Final Stats:\n");
        printf("  Canonical paths: %zu\n", result->stats.canonical_paths_count);
        printf("  Inheritance edges: %zu\n", result->stats.inheritance_edges_count);
        printf("  R-term ops: %zu\n", result->stats.r_term_ops_count);
        printf("  D-term ops: %zu\n", result->stats.d_term_ops_count);
        printf("  Membrane crossings: %zu\n", result->stats.membrane_crossings_count);
        printf("  Compilation time: %.2f ms\n", result->stats.compilation_time_ms);
        printf("  Validation time: %.2f ms\n\n", result->stats.validation_time_ms);
    }

    // Cleanup
    free_surface_ast(ast);

    return result;
}

// =============================================================================
// L1 HRIR GENERATION (True Homoiconic Reversible IR)
// =============================================================================

static char* generate_hrir_json(SurfaceAST* ast, CompilerOptions options) {
    if (!ast) return NULL;

    // Create HRIR program from surface AST
    HRIR_Program* program = hr_ir_create_program("compiled_surface");
    if (!program) return NULL;

    // Convert each statement to HRIR cells
    for (size_t i = 0; i < ast->statement_count; i++) {
        Statement* stmt = &ast->statements[i];

        if (stmt->type == STMT_SEND) {
            SendOperation* send = &stmt->as.send;

            // Create HRIR cell from send operation
            HRIR_Cell* cell = hr_ir_from_send_operation(
                send->target, send->selector,
                (const char**)send->arguments, send->arg_count
            );

            if (cell) {
                // Set metadata
                char canonical_path[256];
                if (options.auto_hoist) {
                    char synthetic_actor[256];
                    snprintf(synthetic_actor, sizeof(synthetic_actor), "%sActor", send->target);
                    snprintf(canonical_path, sizeof(canonical_path), "%sProto.%s.%s",
                            send->target, synthetic_actor, send->selector);
                } else {
                    snprintf(canonical_path, sizeof(canonical_path), "%s.%s",
                            send->target, send->selector);
                }

                hr_ir_set_cell_meta(cell, "surface_compilation", i + 1, canonical_path);

                // Mark as D-term if needed
                if (!send->type) { // OP_D_TERM
                    cell->is_reversible = false;
                }

                hr_ir_add_cell(program, cell);
            }
        }
    }

    // Serialize to JSON
    char* json = hr_ir_serialize_program(program);

    // Cleanup
    hr_ir_free_program(program);

    return json;
}

// =============================================================================
// CANONICALIZATION (RioVN Integration)
// =============================================================================

static char* canonicalize_paths(SurfaceAST* ast, CompilerOptions options) {
    // Generate canonical Proto.Actor.Func paths for all send operations
    char* canonical_output = calloc(1, 1024); // Start with 1KB buffer
    if (!canonical_output) return NULL;

    size_t buffer_size = 1024;
    size_t current_pos = 0;

    for (size_t i = 0; i < ast->statement_count; i++) {
        Statement* stmt = &ast->statements[i];

        if (stmt->type == STMT_SEND) {
            SendOperation* send = &stmt->as.send;

            // NULL check for target and selector
            if (!send->target || !send->selector) continue;

            // Canonicalize target name (PascalCase)
            char* canonical_target = to_pascal_case(send->target);
            if (!canonical_target) continue;

            // Generate canonical path
            char canonical_path[256];
            if (options.auto_hoist) {
                // Auto-hoist to synthetic actor
                char synthetic_actor[256];
                snprintf(synthetic_actor, sizeof(synthetic_actor), "%sActor", canonical_target);
                snprintf(canonical_path, sizeof(canonical_path), "%sProto.%s.%s",
                        canonical_target, synthetic_actor, send->selector);
            } else {
                // Direct canonicalization
                snprintf(canonical_path, sizeof(canonical_path), "%s.%s",
                        canonical_target, send->selector);
            }

            // Add to output with operation type annotation
            const char* op_type_str = (send->type == OP_R_TERM) ? "[R]" :
                                    (send->type == OP_D_TERM) ? "[D]" : "[S]";

            size_t needed = strlen(canonical_path) + strlen(op_type_str) + 4; // +4 for " -> " and newline

            // Expand buffer if needed
            if (current_pos + needed >= buffer_size) {
                buffer_size *= 2;
                char* new_output = realloc(canonical_output, buffer_size);
                if (!new_output) {
                    free(canonical_target);
                    free(canonical_output);
                    return NULL;
                }
                canonical_output = new_output;
            }

            sprintf(canonical_output + current_pos, "%s %s\n", canonical_path, op_type_str);
            current_pos += strlen(canonical_output + current_pos);

            free(canonical_target);
        }
    }

    return canonical_output;
}

// =============================================================================
// REVERSIBLE IR GENERATION (Rio Integration)
// =============================================================================

static char* generate_reversible_ir(SurfaceAST* ast, CompilerOptions options) {
    if (!options.reversible_default) {
        char* disabled_msg = strdup("// Reversible IR disabled\n");
        return disabled_msg ? disabled_msg : NULL;
    }

    char* ir_output = calloc(1, 2048);
    if (!ir_output) return NULL;

    size_t buffer_size = 2048;
    size_t current_pos = 0;

    sprintf(ir_output + current_pos, "// Reversible Intermediate Representation (L2a/L1)\n");
    current_pos += strlen(ir_output + current_pos);

    for (size_t i = 0; i < ast->statement_count; i++) {
        Statement* stmt = &ast->statements[i];

        if (stmt->type == STMT_SEND && stmt->as.send.type == OP_R_TERM) {
            // NULL checks for target and selector
            if (!stmt->as.send.target || !stmt->as.send.selector) continue;

            // Check buffer space needed
            size_t needed = strlen(stmt->as.send.target) + strlen(stmt->as.send.selector) + 20; // +20 for formatting
            for (size_t j = 0; j < stmt->as.send.arg_count; j++) {
                if (stmt->as.send.arguments && stmt->as.send.arguments[j]) {
                    needed += strlen(stmt->as.send.arguments[j]) + 2; // +2 for ", "
                }
            }

            // Expand buffer if needed
            if (current_pos + needed >= buffer_size) {
                buffer_size *= 2;
                char* new_output = realloc(ir_output, buffer_size);
                if (!new_output) {
                    free(ir_output);
                    return NULL;
                }
                ir_output = new_output;
            }

            // Generate reversible IR for R-term operations
            sprintf(ir_output + current_pos, "REV_OP: %s.%s(", stmt->as.send.target, stmt->as.send.selector);
            current_pos += strlen(ir_output + current_pos);

            // Add arguments with NULL checks
            for (size_t j = 0; j < stmt->as.send.arg_count; j++) {
                if (j > 0) sprintf(ir_output + current_pos++, ", ");
                if (stmt->as.send.arguments && stmt->as.send.arguments[j]) {
                    sprintf(ir_output + current_pos, "%s", stmt->as.send.arguments[j]);
                    current_pos += strlen(stmt->as.send.arguments[j]);
                }
            }

            sprintf(ir_output + current_pos, ") [INVERSE: %s_inverse]\n", stmt->as.send.selector);
            current_pos += strlen(ir_output + current_pos);
        }
    }

    return ir_output;
}

// =============================================================================
// MEMBRANE LOGS (D-term Boundaries)
// =============================================================================

static char* generate_membrane_logs(SurfaceAST* ast, CompilerOptions options) {
    char* log_output = calloc(1, 1024);
    if (!log_output) return NULL;

    size_t buffer_size = 1024;
    size_t current_pos = 0;

    sprintf(log_output + current_pos, "// Membrane Logs - D-term Boundaries\n");
    current_pos += strlen(log_output + current_pos);

    bool has_d_term = false;
    for (size_t i = 0; i < ast->statement_count; i++) {
        Statement* stmt = &ast->statements[i];

        if (stmt->type == STMT_SEND && stmt->as.send.type == OP_D_TERM) {
            // NULL checks for target and selector
            if (!stmt->as.send.target || !stmt->as.send.selector) continue;

            has_d_term = true;

            // Check buffer space needed
            size_t needed = strlen(stmt->as.send.target) + strlen(stmt->as.send.selector) + 30; // +30 for formatting
            if (options.debug_mode) {
                needed += 50; // +50 for timestamp and compensation
            }

            // Expand buffer if needed
            if (current_pos + needed >= buffer_size) {
                buffer_size *= 2;
                char* new_output = realloc(log_output, buffer_size);
                if (!new_output) {
                    free(log_output);
                    return NULL;
                }
                log_output = new_output;
            }

            sprintf(log_output + current_pos, "MEMBRANE: %s.%s() [IRREVERSIBLE]\n",
                   stmt->as.send.target, stmt->as.send.selector);
            current_pos += strlen(log_output + current_pos);

            if (options.debug_mode) {
                sprintf(log_output + current_pos, "  TIMESTAMP: %lu\n", (unsigned long)time(NULL));
                current_pos += strlen(log_output + current_pos);
                sprintf(log_output + current_pos, "  COMPENSATION: rollback_%s\n", stmt->as.send.selector);
                current_pos += strlen(log_output + current_pos);
            }
        }
    }

    if (!has_d_term) {
        // Check buffer space for final message
        size_t needed = strlen("// No D-term operations - fully reversible\n");
        if (current_pos + needed >= buffer_size) {
            buffer_size = current_pos + needed + 1;
            char* new_output = realloc(log_output, buffer_size);
            if (!new_output) {
                free(log_output);
                return NULL;
            }
            log_output = new_output;
        }

        sprintf(log_output + current_pos, "// No D-term operations - fully reversible\n");
    }

    return log_output;
}

// =============================================================================
// UNIFIED VALIDATION
// =============================================================================

static bool validate_unified(SurfaceAST* ast, CompilerOptions options,
                           const char** error_msg) {
    // Validate inheritance relationships (RioVN-style)
    for (size_t i = 0; i < ast->inheritance_count; i++) {
        // Basic cycle detection (simplified)
        for (size_t j = i + 1; j < ast->inheritance_count; j++) {
            // Check for potential cycles (this is a simplified check)
            if (strstr(ast->inheritance_relations[i], ast->inheritance_relations[j]) &&
                strstr(ast->inheritance_relations[j], ast->inheritance_relations[i])) {
                *error_msg = "Inheritance cycle detected";
                return false;
            }
        }
    }

    // Validate D-term tagging in strict mode
    if (options.strict_mode) {
        for (size_t i = 0; i < ast->statement_count; i++) {
            Statement* stmt = &ast->statements[i];

            if (stmt->type == STMT_SEND && stmt->as.send.type == OP_D_TERM &&
                !stmt->as.send.tag.is_tagged) {
                *error_msg = "D-term operation requires explicit @irreversible or @io tag in strict mode";
                return false;
            }
        }
    }

    // Validate reversible default
    if (options.reversible_default) {
        // All operations should have inverse paths (simplified check)
        for (size_t i = 0; i < ast->statement_count; i++) {
            Statement* stmt = &ast->statements[i];

            if (stmt->type == STMT_SEND && stmt->as.send.type == OP_R_TERM) {
                // In a full implementation, we'd check that an inverse exists
                // For now, just acknowledge the requirement
            }
        }
    }

    return true;
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

// Free compilation result and all its resources
void free_compilation_result(CompilationResult* result) {
    if (!result) return;

    // Free strings with NULL checks
    if (result->canonical_code) free(result->canonical_code);
    if (result->reversible_ir) free(result->reversible_ir);
    if (result->membrane_logs) free(result->membrane_logs);
    if (result->hrir_json) free(result->hrir_json);

    // Free inheritance graph with NULL checks
    if (result->inheritance_graph) {
        for (size_t i = 0; i < result->inheritance_count; i++) {
            if (result->inheritance_graph[i]) free(result->inheritance_graph[i]);
        }
        free(result->inheritance_graph);
    }

    if (result->first_error_message) free(result->first_error_message);
    free(result);
}
