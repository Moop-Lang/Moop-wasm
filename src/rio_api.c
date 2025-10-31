// rio-riovn-merged/src/rio_api.c
// Embeddable C API Implementation for Rio+RioVN

#include "rio_api.h"
#include "surface_parser.h"
#include "unified_compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// INTERNAL STRUCTURES (Opaque to API users)
// =============================================================================

struct RioVM {
    bool verbose;
    char* last_error;
    // Future: Add VM state, registries, etc.
};

struct RioResult {
    bool success;
    char* error_message;
    RioErrorCode error_code;
    char* canonical_code;
    char* reversible_ir;
    char* membrane_logs;
    char* json_output;
    char* hrir_json;
    RioAST* ast;
    RioInheritanceMap* inheritance_map;
    RioStats stats;
};

struct RioAST {
    SurfaceAST* internal_ast;
};

struct RioInheritanceMap {
    char** relations;
    size_t count;
    // Future: Add efficient lookup structures
};

// =============================================================================
// VM LIFECYCLE
// =============================================================================

RioVM* rio_create_vm(void) {
    RioVM* vm = calloc(1, sizeof(RioVM));
    if (!vm) return NULL;

    vm->verbose = false;
    vm->last_error = NULL;

    return vm;
}

void rio_destroy_vm(RioVM* vm) {
    if (!vm) return;

    free(vm->last_error);
    free(vm);
}

// =============================================================================
// COMPILATION OPTIONS
// =============================================================================

RioCompileOptions rio_default_options(void) {
    return (RioCompileOptions){
        .strict_mode = false,
        .auto_hoist = true,
        .debug_mode = false,
        .reversible_default = true,
        .json_output = false
    };
}

// =============================================================================
// COMPILATION API
// =============================================================================

RioResult* rio_compile_string(RioVM* vm, const char* source, RioCompileOptions options) {
    if (!vm || !source) {
        if (vm) {
            free(vm->last_error);
            vm->last_error = strdup("Invalid parameters to rio_compile_string");
        }
        return NULL;
    }

    RioResult* result = calloc(1, sizeof(RioResult));
    if (!result) {
        free(vm->last_error);
        vm->last_error = strdup("Memory allocation failed");
        return NULL;
    }

    // Initialize result
    result->success = true;
    result->error_code = RIO_SUCCESS;
    result->error_message = NULL;
    result->canonical_code = NULL;
    result->reversible_ir = NULL;
    result->membrane_logs = NULL;
    result->json_output = NULL;
    result->hrir_json = NULL;
    result->ast = NULL;
    result->inheritance_map = NULL;

    // Parse source
    SurfaceAST* ast = parse_surface(source);
    if (!ast) {
        result->success = false;
        result->error_code = RIO_ERROR_PARSE_FAILED;
        result->error_message = strdup("Failed to parse source code");
        return result;
    }

    // Create AST wrapper
    result->ast = calloc(1, sizeof(RioAST));
    if (result->ast) {
        result->ast->internal_ast = ast;
    }

    // Create inheritance map
    result->inheritance_map = calloc(1, sizeof(RioInheritanceMap));
    if (result->inheritance_map) {
        result->inheritance_map->relations = calloc(ast->inheritance_count, sizeof(char*));
        result->inheritance_map->count = ast->inheritance_count;
        for (size_t i = 0; i < ast->inheritance_count; i++) {
            result->inheritance_map->relations[i] = strdup(ast->inheritance_relations[i]);
        }
    }

    // Generate canonical code (simplified)
    result->canonical_code = strdup("// Canonical code generation not yet implemented");

    // Generate JSON output if requested
    if (options.json_output) {
        // Simplified JSON generation
        char json_buffer[2048];
        snprintf(json_buffer, sizeof(json_buffer),
                "{\n"
                "  \"statement_count\": %zu,\n"
                "  \"inheritance_count\": %zu\n"
                "}",
                ast->statement_count, ast->inheritance_count);
        result->json_output = strdup(json_buffer);
    }

    // Set basic stats
    result->stats.canonical_paths_count = ast->statement_count;
    result->stats.inheritance_edges_count = ast->inheritance_count;
    result->stats.r_term_ops_count = 0; // Simplified
    result->stats.d_term_ops_count = 0; // Simplified
    result->stats.membrane_crossings_count = 0; // Simplified
    result->stats.compilation_time_ms = 0.1; // Simplified
    result->stats.validation_time_ms = 0.05; // Simplified

    if (vm->verbose) {
        printf("Rio API: Compiled string successfully\n");
    }

    return result;
}

RioResult* rio_compile_file(RioVM* vm, const char* filename, RioCompileOptions options) {
    if (!vm || !filename) {
        if (vm) {
            free(vm->last_error);
            vm->last_error = strdup("Invalid parameters to rio_compile_file");
        }
        return NULL;
    }

    // Read file
    FILE* file = fopen(filename, "r");
    if (!file) {
        free(vm->last_error);
        vm->last_error = strdup("Cannot open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(size + 1);
    if (!source) {
        fclose(file);
        free(vm->last_error);
        vm->last_error = strdup("Memory allocation failed");
        return NULL;
    }

    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);

    // Compile the source
    RioResult* result = rio_compile_string(vm, source, options);
    free(source);

    return result;
}

// =============================================================================
// RESULT ACCESS API
// =============================================================================

bool rio_result_success(const RioResult* result) {
    return result && result->success;
}

const char* rio_result_error_message(const RioResult* result) {
    return result ? result->error_message : NULL;
}

RioErrorCode rio_result_error_code(const RioResult* result) {
    return result ? result->error_code : RIO_SUCCESS;
}

const char* rio_result_canonical_code(const RioResult* result) {
    return result ? result->canonical_code : NULL;
}

const char* rio_result_reversible_ir(const RioResult* result) {
    return result ? result->reversible_ir : NULL;
}

const char* rio_result_membrane_logs(const RioResult* result) {
    return result ? result->membrane_logs : NULL;
}

const char* rio_result_json_output(const RioResult* result) {
    return result ? result->json_output : NULL;
}

const char* rio_result_hrir_json(const RioResult* result) {
    return result ? result->hrir_json : NULL;
}

size_t rio_result_inheritance_count(const RioResult* result) {
    return result && result->inheritance_map ? result->inheritance_map->count : 0;
}

const char* rio_result_inheritance_relation(const RioResult* result, size_t index) {
    if (!result || !result->inheritance_map ||
        index >= result->inheritance_map->count) {
        return NULL;
    }
    return result->inheritance_map->relations[index];
}

size_t rio_result_statement_count(const RioResult* result) {
    return result && result->ast && result->ast->internal_ast ?
           result->ast->internal_ast->statement_count : 0;
}

RioStats rio_result_stats(const RioResult* result) {
    return result ? result->stats : (RioStats){0};
}

// =============================================================================
// HOMOICONICITY API (AST Access)
// =============================================================================

RioAST* rio_result_get_ast(const RioResult* result) {
    return result ? result->ast : NULL;
}

size_t rio_ast_node_count(const RioAST* ast) {
    return ast && ast->internal_ast ? ast->internal_ast->statement_count : 0;
}

RioASTNode rio_ast_get_node(const RioAST* ast, size_t index) {
    RioASTNode empty_node = {0};

    if (!ast || !ast->internal_ast ||
        index >= ast->internal_ast->statement_count) {
        return empty_node;
    }

    Statement* stmt = &ast->internal_ast->statements[index];

    if (stmt->type == STMT_SEND) {
        return (RioASTNode){
            .type = RIO_AST_SEND,
            .data.send = {
                .target = stmt->as.send.target,
                .selector = stmt->as.send.selector,
                .arg_count = stmt->as.send.arg_count,
                .arguments = (const char**)stmt->as.send.arguments,
                .is_tagged = stmt->as.send.tag.is_tagged,
                .tag_type = stmt->as.send.tag.tag_type
            }
        };
    } else if (stmt->type == STMT_INHERIT) {
        return (RioASTNode){
            .type = RIO_AST_INHERIT,
            .data.inherit = {
                .child = stmt->as.inherit.child,
                .parent = stmt->as.inherit.parent
            }
        };
    }

    return empty_node;
}

// =============================================================================
// INHERITANCE REGISTRY API
// =============================================================================

RioInheritanceMap* rio_result_get_inheritance_map(const RioResult* result) {
    return result ? result->inheritance_map : NULL;
}

bool rio_inheritance_has_parent(const RioInheritanceMap* map,
                               const char* child, const char* parent) {
    if (!map || !child || !parent) return false;

    // Simplified check - in real implementation, this would traverse inheritance graph
    for (size_t i = 0; i < map->count; i++) {
        if (strstr(map->relations[i], child) && strstr(map->relations[i], parent)) {
            return true;
        }
    }

    return false;
}

size_t rio_inheritance_get_parents(const RioInheritanceMap* map,
                                  const char* child, const char** parents, size_t max_parents) {
    // Simplified implementation - returns direct parents only
    size_t found = 0;

    if (!map || !child || !parents || max_parents == 0) return 0;

    for (size_t i = 0; i < map->count && found < max_parents; i++) {
        // Simple parsing of "Child <- Parent" format
        const char* relation = map->relations[i];
        const char* arrow_pos = strstr(relation, " <- ");
        if (arrow_pos) {
            size_t child_len = arrow_pos - relation;
            if (child_len == strlen(child) && strncmp(relation, child, child_len) == 0) {
                const char* parent_start = arrow_pos + 4; // Skip " <- "
                parents[found++] = parent_start;
            }
        }
    }

    return found;
}

// =============================================================================
// CANONICAL REGISTRY API
// =============================================================================

RioCanonicalPath rio_parse_canonical_path(const char* path) {
    RioCanonicalPath result = {NULL, NULL, NULL, path};

    if (!path) return result;

    // Parse Proto.Actor.Func format
    char* copy = strdup(path);
    if (!copy) return result;

    char* dot1 = strchr(copy, '.');
    if (dot1) {
        *dot1 = '\0';
        result.prototype = copy;

        char* dot2 = strchr(dot1 + 1, '.');
        if (dot2) {
            *dot2 = '\0';
            result.actor = dot1 + 1;
            result.function = dot2 + 1;
        } else {
            result.actor = dot1 + 1;
        }
    } else {
        result.prototype = copy;
    }

    return result;
}

bool rio_is_canonical_path(const char* path) {
    if (!path) return false;

    // Check for Proto.Actor.Func format
    int dot_count = 0;
    for (const char* p = path; *p; p++) {
        if (*p == '.') dot_count++;
    }

    return dot_count >= 1 && dot_count <= 2;
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void rio_free_result(RioResult* result) {
    if (!result) return;

    free(result->error_message);
    free(result->canonical_code);
    free(result->reversible_ir);
    free(result->membrane_logs);
    free(result->json_output);

    if (result->ast) {
        rio_free_ast(result->ast);
    }

    if (result->inheritance_map) {
        rio_free_inheritance_map(result->inheritance_map);
    }

    free(result);
}

void rio_free_ast(RioAST* ast) {
    if (!ast) return;

    if (ast->internal_ast) {
        free_surface_ast(ast->internal_ast);
    }

    free(ast);
}

void rio_free_inheritance_map(RioInheritanceMap* map) {
    if (!map) return;

    for (size_t i = 0; i < map->count; i++) {
        free(map->relations[i]);
    }
    free(map->relations);
    free(map);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

const char* rio_version(void) {
    return "Rio+RioVN v1.0.0 (Bootloader M1)";
}

const char* rio_get_last_error(const RioVM* vm) {
    return vm ? vm->last_error : NULL;
}

void rio_set_verbose(RioVM* vm, bool verbose) {
    if (vm) {
        vm->verbose = verbose;
    }
}
