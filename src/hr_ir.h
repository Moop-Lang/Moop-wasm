// rio-riovn-merged/src/hr_ir.h
// L1 HRIR - Homoiconic Reversible Intermediate Representation

#ifndef HR_IR_H
#define HR_IR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// =============================================================================
// L1 HRIR - HOMOICONIC REVERSIBLE IR
// =============================================================================

// HRIR Cell - Self-describing, reversible operation
typedef struct HRIR_Cell {
    uint32_t id;              // Unique stable identifier
    const char* opcode;       // Operation name ("add", "send", etc.)
    const char** args;        // Argument array (NULL-terminated)
    size_t arg_count;         // Number of arguments

    // Reversibility
    struct HRIR_Cell* inverse; // Inverse operation cell
    bool is_reversible;       // Can this operation be undone?

    // Meta information
    const char* source_location; // Original source location
    uint32_t line_number;     // Line in source
    const char* canonical_path; // Proto.Actor.Func path

    // Execution state
    bool executed;            // Has this cell been executed?
    void* result;            // Execution result (if any)
} HRIR_Cell;

// HRIR Program - Array of cells representing the program
typedef struct HRIR_Program {
    HRIR_Cell** cells;       // Array of cells
    size_t cell_count;       // Number of cells
    size_t capacity;         // Allocated capacity

    // Execution state
    size_t pc;              // Program counter
    void** tape;            // Reversible execution tape
    size_t tape_size;       // Tape size

    // Metadata
    const char* source_name; // Original source filename
    uint32_t next_id;       // Next cell ID to assign
} HRIR_Program;

// HRIR Runtime - Execution environment
typedef struct HRIR_Runtime {
    HRIR_Program* program;   // Current program
    size_t checkpoint;       // Last checkpoint position

    // Statistics
    size_t steps_executed;   // Total execution steps
    size_t rollbacks;        // Number of rollbacks performed

    // Error handling
    const char* last_error;  // Last error message
} HRIR_Runtime;

// =============================================================================
// CELL CREATION API
// =============================================================================

// Create a new HRIR cell
HRIR_Cell* hr_ir_create_cell(const char* opcode, const char** args, size_t arg_count);

// Create inverse cell for a given cell
HRIR_Cell* hr_ir_create_inverse(HRIR_Cell* cell);

// Set cell metadata
void hr_ir_set_cell_meta(HRIR_Cell* cell, const char* source_location,
                        uint32_t line_number, const char* canonical_path);

// Free cell resources
void hr_ir_free_cell(HRIR_Cell* cell);

// =============================================================================
// PROGRAM MANAGEMENT API
// =============================================================================

// Create new HRIR program
HRIR_Program* hr_ir_create_program(const char* source_name);

// Add cell to program
bool hr_ir_add_cell(HRIR_Program* program, HRIR_Cell* cell);

// Get cell by ID
HRIR_Cell* hr_ir_get_cell_by_id(HRIR_Program* program, uint32_t id);

// Get cell by index
HRIR_Cell* hr_ir_get_cell(HRIR_Program* program, size_t index);

// Serialize program to JSON
char* hr_ir_serialize_program(HRIR_Program* program);

// Deserialize program from JSON
HRIR_Program* hr_ir_deserialize_program(const char* json);

// Free program resources
void hr_ir_free_program(HRIR_Program* program);

// =============================================================================
// RUNTIME EXECUTION API
// =============================================================================

// Create runtime for program
HRIR_Runtime* hr_ir_create_runtime(HRIR_Program* program);

// Execute one step
bool hr_ir_step(HRIR_Runtime* runtime);

// Execute until completion or error
bool hr_ir_run(HRIR_Runtime* runtime);

// Undo last step
bool hr_ir_undo(HRIR_Runtime* runtime);

// Create checkpoint
bool hr_ir_checkpoint(HRIR_Runtime* runtime);

// Rollback to last checkpoint
bool hr_ir_rollback(HRIR_Runtime* runtime);

// Get current execution state
size_t hr_ir_get_pc(HRIR_Runtime* runtime);
bool hr_ir_is_complete(HRIR_Runtime* runtime);

// Free runtime resources
void hr_ir_free_runtime(HRIR_Runtime* runtime);

// =============================================================================
// COMPILER INTEGRATION API
// =============================================================================

// Convert R-term send to HRIR cell
HRIR_Cell* hr_ir_from_send_operation(const char* target, const char* selector,
                                    const char** args, size_t arg_count);

// Convert D-term operation to membrane cell
HRIR_Cell* hr_ir_from_d_term_operation(const char* operation,
                                      const char** args, size_t arg_count);

// Validate HRIR program invariants
bool hr_ir_validate_program(HRIR_Program* program, char** error_message);

// Optimize HRIR program
bool hr_ir_optimize_program(HRIR_Program* program);

// =============================================================================
// BUILT-IN OPERATIONS (L1 Core)
// =============================================================================

// Arithmetic operations (R-term)
extern const char* HRIR_OP_ADD;
extern const char* HRIR_OP_SUBTRACT;
extern const char* HRIR_OP_MULTIPLY;
extern const char* HRIR_OP_DIVIDE;

// Comparison operations (R-term)
extern const char* HRIR_OP_EQUAL;
extern const char* HRIR_OP_LESS;
extern const char* HRIR_OP_GREATER;

// Control flow (R-term)
extern const char* HRIR_OP_JUMP;
extern const char* HRIR_OP_JUMP_IF;

// I/O operations (D-term - require membrane)
extern const char* HRIR_OP_PRINT;
extern const char* HRIR_OP_READ;

// Memory operations (R-term)
extern const char* HRIR_OP_STORE;
extern const char* HRIR_OP_LOAD;

// =============================================================================
// DEBUGGING & INSPECTION API
// =============================================================================

// Get program statistics
typedef struct {
    size_t total_cells;
    size_t r_term_cells;
    size_t d_term_cells;
    size_t executed_cells;
    size_t checkpoint_count;
} HRIR_Stats;

HRIR_Stats hr_ir_get_stats(HRIR_Program* program);

// Dump program to stdout (debug)
void hr_ir_dump_program(HRIR_Program* program);

// Dump runtime state (debug)
void hr_ir_dump_runtime(HRIR_Runtime* runtime);

// =============================================================================
// ERROR HANDLING
// =============================================================================

typedef enum {
    HRIR_SUCCESS = 0,
    HRIR_ERROR_INVALID_CELL,
    HRIR_ERROR_INVALID_PROGRAM,
    HRIR_ERROR_EXECUTION_FAILED,
    HRIR_ERROR_MEMORY_ALLOCATION,
    HRIR_ERROR_INVALID_OPERATION,
    HRIR_ERROR_IRREVERSIBLE_OPERATION,
    HRIR_ERROR_CHECKPOINT_NOT_FOUND
} HRIR_Error;

// Get last error from runtime
HRIR_Error hr_ir_get_last_error(HRIR_Runtime* runtime);
const char* hr_ir_get_error_message(HRIR_Error error);

#endif // HR_IR_H
