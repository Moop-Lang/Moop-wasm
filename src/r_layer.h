// r_layer.h
// R-Layer: Reversible Substrate Runtime
// Properties: Turing-complete, Homoiconic, Reversible, System-facing

#ifndef R_LAYER_H
#define R_LAYER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// ============================================================================
// HRIR Cell: Homoiconic Reversible Intermediate Representation
// ============================================================================

typedef struct HRIR_Cell {
    uint32_t id;                    // Unique cell identifier

    // Homoiconicity: Gates as data
    const char* opcode;             // "CCNOT", "CNOT", "NOT", "SWAP"
    const char** args;              // Gate arguments (qubit/bit ids)
    uint32_t arg_count;             // Number of arguments

    // Reversibility: Inverse operations
    struct HRIR_Cell* inverse;      // Pointer to inverse operation
    bool is_reversible;             // Always true for R-layer gates

    // Homoiconicity: Self-describing paths
    const char* canonical_path;     // "Proto.Actor.Func"
    const char* source_location;    // "file.moop:line:col"

    // Execution state
    bool executed;                  // Has this cell been executed?
    void* result;                   // Result data (for checkpointing)

    // DAG structure
    struct HRIR_Cell** dependencies; // Cells this depends on
    uint32_t dep_count;

    struct HRIR_Cell** dependents;   // Cells that depend on this
    uint32_t dependent_count;
} HRIR_Cell;

// ============================================================================
// R-Layer Memory: Reversible Gate Memory
// ============================================================================

typedef struct R_Memory {
    // Qubit/bit storage
    uint8_t* qubits;                // Qubit states (classical bits for now)
    uint32_t qubit_count;

    // HRIR cell storage (homoiconic representation)
    HRIR_Cell** cells;              // All computation cells
    uint32_t cell_count;
    uint32_t cell_capacity;

    // Checkpoint storage (for reversibility)
    struct R_Checkpoint** checkpoints;
    uint32_t checkpoint_count;
    uint32_t checkpoint_capacity;

    // Execution history (enables time-travel)
    HRIR_Cell** execution_history;
    uint32_t execution_count;
    uint32_t execution_capacity;
} R_Memory;

// ============================================================================
// R-Layer Checkpoint: State snapshots for reversibility
// ============================================================================

typedef struct R_Checkpoint {
    uint32_t id;

    // Full memory state
    uint8_t* qubit_state;           // Copy of qubit states
    uint32_t qubit_count;

    // Execution point
    uint32_t execution_index;       // Where in history we were

    // Metadata
    const char* label;              // Optional checkpoint label
    uint64_t timestamp;             // When checkpoint was created
} R_Checkpoint;

// ============================================================================
// R-Layer Runtime: Executes reversible gates
// ============================================================================

typedef struct R_Runtime {
    R_Memory* memory;               // Reversible gate memory

    // Gate execution
    uint32_t next_cell_id;          // For generating unique IDs

    // Program instance isolation
    uint32_t instance_id;           // Each program instance gets unique ID
} R_Runtime;

// ============================================================================
// R-Layer API: Core functions
// ============================================================================

// Runtime lifecycle
R_Runtime* r_runtime_init(uint32_t qubit_count, uint32_t instance_id);
void r_runtime_free(R_Runtime* runtime);

// HRIR Cell creation (homoiconic gate representation)
HRIR_Cell* r_create_cell(
    R_Runtime* runtime,
    const char* opcode,
    const char** args,
    uint32_t arg_count,
    const char* canonical_path
);

void r_free_cell(HRIR_Cell* cell);

// Gate execution (the four reversible primitives)
bool r_execute_CCNOT(R_Runtime* runtime, uint32_t a, uint32_t b, uint32_t c);
bool r_execute_CNOT(R_Runtime* runtime, uint32_t a, uint32_t b);
bool r_execute_NOT(R_Runtime* runtime, uint32_t a);
bool r_execute_SWAP(R_Runtime* runtime, uint32_t a, uint32_t b);

// Execute HRIR cell (homoiconic execution)
bool r_execute_cell(R_Runtime* runtime, HRIR_Cell* cell);

// Reversibility operations
R_Checkpoint* r_save_checkpoint(R_Runtime* runtime, const char* label);
bool r_restore_checkpoint(R_Runtime* runtime, R_Checkpoint* checkpoint);
void r_free_checkpoint(R_Checkpoint* checkpoint);

// Inverse operations (for reversibility)
HRIR_Cell* r_get_inverse(HRIR_Cell* cell);
bool r_execute_inverse(R_Runtime* runtime, HRIR_Cell* cell);

// Time-travel debugging
bool r_rewind_to_index(R_Runtime* runtime, uint32_t execution_index);
bool r_step_forward(R_Runtime* runtime);
bool r_step_backward(R_Runtime* runtime);

// Homoiconicity operations
const char* r_cell_to_string(HRIR_Cell* cell);  // Serialize cell as data
HRIR_Cell* r_string_to_cell(const char* str);   // Parse data as cell

// Qubit access
uint8_t r_read_qubit(R_Runtime* runtime, uint32_t index);
bool r_write_qubit(R_Runtime* runtime, uint32_t index, uint8_t value);

// Debugging and introspection
void r_print_cell(HRIR_Cell* cell);
void r_print_memory_state(R_Runtime* runtime);
void r_print_execution_history(R_Runtime* runtime);

#endif // R_LAYER_H
