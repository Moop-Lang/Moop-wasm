// src/l5_moop.h
// L5 Moop: Natural Language Layer - Inherits Homoiconicity & Reversibility
// Homoiconic: Natural language as executable data
// Reversible: Message passing operations can be undone
// Inherits from L1 HRIR → L2a Functions → L3 Actors → L4 root_proto

#ifndef L5_MOOP_H
#define L5_MOOP_H

#include <stdbool.h>
#include <stdlib.h>
#include "hr_ir.h"
#include "surface_parser.h"

// =============================================================================
// L5 MOOP - NATURAL LANGUAGE HOMOICONIC PROGRAM
// =============================================================================

// L5 Statement types
typedef enum {
    L5_STMT_INHERITANCE,
    L5_STMT_MESSAGE_SEND,
    L5_STMT_OUTPUT,
    L5_STMT_UNKNOWN
} L5_StatementType;

// L5 Statement structure
typedef struct L5_Statement {
    int id;
    char* text;
    L5_StatementType type;
    bool is_homoiconic;
    bool is_reversible;
    bool executed;

    // Parsed data
    union {
        struct {
            char* child;
            char* parent;
        } inheritance;

        struct {
            char* target;
            char* selector;
            char** args;
            int arg_count;
        } message_send;

        struct {
            char* content;
        } output;
    } data;

    // HRIR cell for this statement
    HRIR_Cell* hrir_cell;
} L5_Statement;

// L5 Homoiconic Program
typedef struct L5_MoopProgram {
    char* source_name;
    L5_Statement** statements;
    int statement_count;
    int statement_capacity;

    // HRIR integration
    HRIR_Program* hrir_program;

    // Execution state
    bool is_homoiconic;
    bool is_reversible;
    char* inheritance_chain;

    // Execution history for time-travel
    int* execution_history;
    int history_count;
    int history_capacity;

    // Checkpoints for rollback
    char** checkpoints;
    int checkpoint_count;
    int checkpoint_capacity;
} L5_MoopProgram;

// =============================================================================
// L5 COMPILATION OPTIONS
// =============================================================================

typedef struct {
    bool enhanced;              // Enable homoiconic features
    bool auto_inherit;          // Auto-inherit from L4
    bool enable_time_travel;    // Enable execution history
    bool generate_hrir;         // Generate HRIR cells
} L5_CompileOptions;

// =============================================================================
// L5 COMPILATION RESULT
// =============================================================================

typedef struct L5_CompileResult {
    // Legacy L4 output for backward compatibility
    char* l4_output;

    // Enhanced homoiconic features (when options.enhanced = true)
    L5_MoopProgram* homoiconic_program;
    char* program_id;
    HRIR_Program* hrir_program;
    bool is_homoiconic;
    bool is_reversible;
    char* inheritance_chain;

    // Features
    struct {
        int homoiconic_statements;
        int hrir_cells;
        int reversible_operations;
        bool time_travel_capable;
    } features;

    // Success/error
    bool success;
    char* error_message;
} L5_CompileResult;

// =============================================================================
// PUBLIC API FUNCTIONS
// =============================================================================

// Main compilation function with orthogonal enhancement flag
L5_CompileResult* l5_compile_moop(const char* moop_code, L5_CompileOptions options);

// Legacy function for backward compatibility (always minimal)
char* l5_compile_moop_legacy(const char* moop_code);

// Enhanced functions (available when options.enhanced = true)
L5_MoopProgram* l5_create_homoiconic_program(const char* source_name);
bool l5_add_statement(L5_MoopProgram* program, const char* statement_text);
bool l5_generate_hrir_cells(L5_MoopProgram* program);
bool l5_execute_program(L5_MoopProgram* program);
bool l5_undo_program(L5_MoopProgram* program, int steps);
bool l5_rollback_program(L5_MoopProgram* program, const char* checkpoint_id);
char* l5_create_checkpoint(L5_MoopProgram* program);
char* l5_get_program_data(L5_MoopProgram* program);

// Memory management
void l5_free_compile_result(L5_CompileResult* result);
void l5_free_moop_program(L5_MoopProgram* program);

// Utility functions
L5_CompileOptions l5_default_options(void);
L5_CompileOptions l5_enhanced_options(void);

// Environment detection for orthogonal flag
bool l5_should_use_enhanced_mode(void);

// =============================================================================
// L5 TIME-TRAVEL API
// =============================================================================

// Execute program with time-travel capabilities
typedef struct {
    bool (*execute)(L5_MoopProgram* program);
    bool (*undo)(L5_MoopProgram* program, int steps);
    bool (*rollback)(L5_MoopProgram* program, const char* checkpoint_id);
    char* (*checkpoint)(L5_MoopProgram* program);
    char* (*get_program_data)(L5_MoopProgram* program);
} L5_TimeTravelAPI;

L5_TimeTravelAPI l5_get_time_travel_api(L5_MoopProgram* program);

#endif // L5_MOOP_H
