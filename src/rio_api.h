// rio-riovn-merged/src/rio_api.h
// Embeddable C API for Rio+RioVN Unified Compiler

#ifndef RIO_API_H
#define RIO_API_H

#include <stddef.h>
#include <stdbool.h>

// =============================================================================
// RIO API - EMBEDDABLE C INTERFACE
// =============================================================================

// Opaque types for embeddable API
typedef struct RioVM RioVM;
typedef struct RioResult RioResult;
typedef struct RioAST RioAST;
typedef struct RioInheritanceMap RioInheritanceMap;

// =============================================================================
// VM LIFECYCLE
// =============================================================================

// Create a new Rio+RioVN virtual machine
RioVM* rio_create_vm(void);

// Destroy a Rio+RioVN virtual machine
void rio_destroy_vm(RioVM* vm);

// =============================================================================
// COMPILATION API
// =============================================================================

// Compilation options (orthogonal concerns)
typedef struct {
    bool strict_mode;        // Enforce explicit D-term tagging
    bool auto_hoist;         // Auto-generate synthetic hierarchy
    bool debug_mode;         // Verbose logging + stats
    bool reversible_default; // Default to reversible operations
    bool json_output;        // Enable JSON output
} RioCompileOptions;

// Default compilation options
RioCompileOptions rio_default_options(void);

// Compile source code string
RioResult* rio_compile_string(RioVM* vm, const char* source, RioCompileOptions options);

// Compile source file
RioResult* rio_compile_file(RioVM* vm, const char* filename, RioCompileOptions options);

// =============================================================================
// RESULT ACCESS API
// =============================================================================

// Check if compilation succeeded
bool rio_result_success(const RioResult* result);

// Get error message (NULL if success)
const char* rio_result_error_message(const RioResult* result);

// Get canonical code output
const char* rio_result_canonical_code(const RioResult* result);

// Get reversible IR output
const char* rio_result_reversible_ir(const RioResult* result);

// Get membrane logs
const char* rio_result_membrane_logs(const RioResult* result);

// Get JSON output (if enabled)
const char* rio_result_json_output(const RioResult* result);

// Get HRIR JSON output (L1 homoiconic reversible IR)
const char* rio_result_hrir_json(const RioResult* result);

// Get inheritance relations count
size_t rio_result_inheritance_count(const RioResult* result);

// Get inheritance relation by index
const char* rio_result_inheritance_relation(const RioResult* result, size_t index);

// Get statement count
size_t rio_result_statement_count(const RioResult* result);

// Get compilation stats
typedef struct {
    size_t canonical_paths_count;
    size_t inheritance_edges_count;
    size_t r_term_ops_count;
    size_t d_term_ops_count;
    size_t membrane_crossings_count;
    double compilation_time_ms;
    double validation_time_ms;
} RioStats;

RioStats rio_result_stats(const RioResult* result);

// =============================================================================
// HOMOICONICITY API (Runtime AST Access)
// =============================================================================

// Get AST from compilation result
RioAST* rio_result_get_ast(const RioResult* result);

// AST inspection functions
typedef enum {
    RIO_AST_SEND,      // Message send operation
    RIO_AST_INHERIT,   // Inheritance declaration
    RIO_AST_COMMENT    // Comment (ignored)
} RioASTNodeType;

// AST node information
typedef struct {
    RioASTNodeType type;
    union {
        struct {
            const char* target;
            const char* selector;
            size_t arg_count;
            const char** arguments;
            bool is_tagged;
            const char* tag_type;
        } send;
        struct {
            const char* child;
            const char* parent;
        } inherit;
    } data;
} RioASTNode;

// Get AST node count
size_t rio_ast_node_count(const RioAST* ast);

// Get AST node by index
RioASTNode rio_ast_get_node(const RioAST* ast, size_t index);

// =============================================================================
// INHERITANCE REGISTRY API
// =============================================================================

// Get inheritance map from result
RioInheritanceMap* rio_result_get_inheritance_map(const RioResult* result);

// Check if class inherits from another
bool rio_inheritance_has_parent(const RioInheritanceMap* map,
                               const char* child, const char* parent);

// Get all parents of a class
size_t rio_inheritance_get_parents(const RioInheritanceMap* map,
                                  const char* child, const char** parents, size_t max_parents);

// =============================================================================
// CANONICAL REGISTRY API
// =============================================================================

// Canonical path utilities
typedef struct {
    const char* prototype;
    const char* actor;
    const char* function;
    const char* full_path;
} RioCanonicalPath;

// Parse canonical path
RioCanonicalPath rio_parse_canonical_path(const char* path);

// Check if path is canonical
bool rio_is_canonical_path(const char* path);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

// Free compilation result
void rio_free_result(RioResult* result);

// Free AST
void rio_free_ast(RioAST* ast);

// Free inheritance map
void rio_free_inheritance_map(RioInheritanceMap* map);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Version information
const char* rio_version(void);

// Get last error from VM
const char* rio_get_last_error(const RioVM* vm);

// Enable/disable verbose logging
void rio_set_verbose(RioVM* vm, bool verbose);

// =============================================================================
// ERROR CODES
// =============================================================================

typedef enum {
    RIO_SUCCESS = 0,
    RIO_ERROR_FILE_NOT_FOUND,
    RIO_ERROR_PARSE_FAILED,
    RIO_ERROR_COMPILATION_FAILED,
    RIO_ERROR_MEMORY_ALLOCATION,
    RIO_ERROR_INVALID_OPTIONS,
    RIO_ERROR_INVALID_PATH,
    RIO_ERROR_INHERITANCE_CYCLE,
    RIO_ERROR_STRICT_MODE_VIOLATION
} RioErrorCode;

// Get error code from result
RioErrorCode rio_result_error_code(const RioResult* result);

#endif // RIO_API_H
