// r_layer.c
// R-Layer: Reversible Substrate Runtime Implementation

#include "r_layer.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Runtime Lifecycle
// ============================================================================

R_Runtime* r_runtime_init(uint32_t qubit_count, uint32_t instance_id) {
    R_Runtime* runtime = (R_Runtime*)malloc(sizeof(R_Runtime));
    if (!runtime) return NULL;

    runtime->instance_id = instance_id;
    runtime->next_cell_id = 0;

    // Initialize memory
    runtime->memory = (R_Memory*)malloc(sizeof(R_Memory));
    if (!runtime->memory) {
        free(runtime);
        return NULL;
    }

    // Allocate qubits
    runtime->memory->qubits = (uint8_t*)calloc(qubit_count, sizeof(uint8_t));
    runtime->memory->qubit_count = qubit_count;

    // Allocate HRIR cell storage
    runtime->memory->cell_capacity = 1024;
    runtime->memory->cells = (HRIR_Cell**)calloc(
        runtime->memory->cell_capacity,
        sizeof(HRIR_Cell*)
    );
    runtime->memory->cell_count = 0;

    // Allocate checkpoint storage
    runtime->memory->checkpoint_capacity = 64;
    runtime->memory->checkpoints = (R_Checkpoint**)calloc(
        runtime->memory->checkpoint_capacity,
        sizeof(R_Checkpoint*)
    );
    runtime->memory->checkpoint_count = 0;

    // Allocate execution history
    runtime->memory->execution_capacity = 4096;
    runtime->memory->execution_history = (HRIR_Cell**)calloc(
        runtime->memory->execution_capacity,
        sizeof(HRIR_Cell*)
    );
    runtime->memory->execution_count = 0;

    return runtime;
}

void r_runtime_free(R_Runtime* runtime) {
    if (!runtime) return;

    if (runtime->memory) {
        // Free qubits
        free(runtime->memory->qubits);

        // Free cells
        for (uint32_t i = 0; i < runtime->memory->cell_count; i++) {
            r_free_cell(runtime->memory->cells[i]);
        }
        free(runtime->memory->cells);

        // Free checkpoints
        for (uint32_t i = 0; i < runtime->memory->checkpoint_count; i++) {
            r_free_checkpoint(runtime->memory->checkpoints[i]);
        }
        free(runtime->memory->checkpoints);

        // Free execution history
        free(runtime->memory->execution_history);

        free(runtime->memory);
    }

    free(runtime);
}

// ============================================================================
// HRIR Cell Creation (Homoiconic)
// ============================================================================

HRIR_Cell* r_create_cell(
    R_Runtime* runtime,
    const char* opcode,
    const char** args,
    uint32_t arg_count,
    const char* canonical_path
) {
    HRIR_Cell* cell = (HRIR_Cell*)malloc(sizeof(HRIR_Cell));
    if (!cell) return NULL;

    cell->id = runtime->next_cell_id++;
    cell->opcode = strdup(opcode);
    cell->arg_count = arg_count;

    // Copy arguments
    cell->args = (const char**)malloc(arg_count * sizeof(char*));
    for (uint32_t i = 0; i < arg_count; i++) {
        cell->args[i] = strdup(args[i]);
    }

    cell->canonical_path = canonical_path ? strdup(canonical_path) : NULL;
    cell->source_location = NULL;

    // Reversibility
    cell->is_reversible = true;  // All R-layer gates are reversible
    cell->inverse = NULL;        // Set later by caller

    // Execution state
    cell->executed = false;
    cell->result = NULL;

    // DAG structure
    cell->dependencies = NULL;
    cell->dep_count = 0;
    cell->dependents = NULL;
    cell->dependent_count = 0;

    // Add to runtime memory
    if (runtime->memory->cell_count >= runtime->memory->cell_capacity) {
        // Grow array
        runtime->memory->cell_capacity *= 2;
        runtime->memory->cells = (HRIR_Cell**)realloc(
            runtime->memory->cells,
            runtime->memory->cell_capacity * sizeof(HRIR_Cell*)
        );
    }

    runtime->memory->cells[runtime->memory->cell_count++] = cell;

    return cell;
}

void r_free_cell(HRIR_Cell* cell) {
    if (!cell) return;

    free((void*)cell->opcode);
    for (uint32_t i = 0; i < cell->arg_count; i++) {
        free((void*)cell->args[i]);
    }
    free(cell->args);
    free((void*)cell->canonical_path);
    free((void*)cell->source_location);
    free(cell->dependencies);
    free(cell->dependents);
    free(cell);
}

// ============================================================================
// Gate Execution (The Four Reversible Primitives)
// ============================================================================

bool r_execute_CCNOT(R_Runtime* runtime, uint32_t a, uint32_t b, uint32_t c) {
    if (a >= runtime->memory->qubit_count ||
        b >= runtime->memory->qubit_count ||
        c >= runtime->memory->qubit_count) {
        return false;
    }

    // Toffoli gate: if (a AND b) then flip c
    if (runtime->memory->qubits[a] && runtime->memory->qubits[b]) {
        runtime->memory->qubits[c] ^= 1;
    }

    return true;
}

bool r_execute_CNOT(R_Runtime* runtime, uint32_t a, uint32_t b) {
    if (a >= runtime->memory->qubit_count ||
        b >= runtime->memory->qubit_count) {
        return false;
    }

    // Controlled NOT: if a then flip b
    if (runtime->memory->qubits[a]) {
        runtime->memory->qubits[b] ^= 1;
    }

    return true;
}

bool r_execute_NOT(R_Runtime* runtime, uint32_t a) {
    if (a >= runtime->memory->qubit_count) {
        return false;
    }

    // NOT: flip a
    runtime->memory->qubits[a] ^= 1;

    return true;
}

bool r_execute_SWAP(R_Runtime* runtime, uint32_t a, uint32_t b) {
    if (a >= runtime->memory->qubit_count ||
        b >= runtime->memory->qubit_count) {
        return false;
    }

    // SWAP: exchange a and b
    uint8_t temp = runtime->memory->qubits[a];
    runtime->memory->qubits[a] = runtime->memory->qubits[b];
    runtime->memory->qubits[b] = temp;

    return true;
}

// ============================================================================
// Execute HRIR Cell (Homoiconic Execution)
// ============================================================================

bool r_execute_cell(R_Runtime* runtime, HRIR_Cell* cell) {
    if (!cell || cell->executed) return false;

    bool success = false;

    // Parse arguments as qubit indices
    uint32_t args[3] = {0};
    for (uint32_t i = 0; i < cell->arg_count && i < 3; i++) {
        args[i] = (uint32_t)atoi(cell->args[i]);
    }

    // Execute based on opcode
    if (strcmp(cell->opcode, "CCNOT") == 0 && cell->arg_count == 3) {
        success = r_execute_CCNOT(runtime, args[0], args[1], args[2]);
    } else if (strcmp(cell->opcode, "CNOT") == 0 && cell->arg_count == 2) {
        success = r_execute_CNOT(runtime, args[0], args[1]);
    } else if (strcmp(cell->opcode, "NOT") == 0 && cell->arg_count == 1) {
        success = r_execute_NOT(runtime, args[0]);
    } else if (strcmp(cell->opcode, "SWAP") == 0 && cell->arg_count == 2) {
        success = r_execute_SWAP(runtime, args[0], args[1]);
    }

    if (success) {
        cell->executed = true;

        // Add to execution history
        if (runtime->memory->execution_count >= runtime->memory->execution_capacity) {
            runtime->memory->execution_capacity *= 2;
            runtime->memory->execution_history = (HRIR_Cell**)realloc(
                runtime->memory->execution_history,
                runtime->memory->execution_capacity * sizeof(HRIR_Cell*)
            );
        }
        runtime->memory->execution_history[runtime->memory->execution_count++] = cell;
    }

    return success;
}

// ============================================================================
// Reversibility Operations
// ============================================================================

R_Checkpoint* r_save_checkpoint(R_Runtime* runtime, const char* label) {
    R_Checkpoint* checkpoint = (R_Checkpoint*)malloc(sizeof(R_Checkpoint));
    if (!checkpoint) return NULL;

    checkpoint->id = runtime->memory->checkpoint_count;
    checkpoint->label = label ? strdup(label) : NULL;
    checkpoint->timestamp = (uint64_t)time(NULL);

    // Copy qubit state
    checkpoint->qubit_count = runtime->memory->qubit_count;
    checkpoint->qubit_state = (uint8_t*)malloc(checkpoint->qubit_count);
    memcpy(
        checkpoint->qubit_state,
        runtime->memory->qubits,
        checkpoint->qubit_count
    );

    // Save execution index
    checkpoint->execution_index = runtime->memory->execution_count;

    // Add to runtime
    if (runtime->memory->checkpoint_count >= runtime->memory->checkpoint_capacity) {
        runtime->memory->checkpoint_capacity *= 2;
        runtime->memory->checkpoints = (R_Checkpoint**)realloc(
            runtime->memory->checkpoints,
            runtime->memory->checkpoint_capacity * sizeof(R_Checkpoint*)
        );
    }
    runtime->memory->checkpoints[runtime->memory->checkpoint_count++] = checkpoint;

    return checkpoint;
}

bool r_restore_checkpoint(R_Runtime* runtime, R_Checkpoint* checkpoint) {
    if (!checkpoint) return false;

    // Restore qubit state
    if (checkpoint->qubit_count != runtime->memory->qubit_count) {
        return false;  // Qubit count mismatch
    }

    memcpy(
        runtime->memory->qubits,
        checkpoint->qubit_state,
        checkpoint->qubit_count
    );

    // Restore execution index
    runtime->memory->execution_count = checkpoint->execution_index;

    // Mark cells as not executed
    for (uint32_t i = checkpoint->execution_index;
         i < runtime->memory->cell_count;
         i++) {
        if (runtime->memory->cells[i]) {
            runtime->memory->cells[i]->executed = false;
        }
    }

    return true;
}

void r_free_checkpoint(R_Checkpoint* checkpoint) {
    if (!checkpoint) return;
    free((void*)checkpoint->label);
    free(checkpoint->qubit_state);
    free(checkpoint);
}

// ============================================================================
// Inverse Operations
// ============================================================================

HRIR_Cell* r_get_inverse(HRIR_Cell* cell) {
    if (!cell) return NULL;

    // All R-layer gates are self-inverse
    return cell;
}

bool r_execute_inverse(R_Runtime* runtime, HRIR_Cell* cell) {
    // Since all R-layer gates are self-inverse, just execute again
    return r_execute_cell(runtime, cell);
}

// ============================================================================
// Time-Travel Debugging
// ============================================================================

bool r_rewind_to_index(R_Runtime* runtime, uint32_t execution_index) {
    if (execution_index > runtime->memory->execution_count) {
        return false;
    }

    // Execute inverses for all operations after the index
    for (uint32_t i = runtime->memory->execution_count; i > execution_index; i--) {
        HRIR_Cell* cell = runtime->memory->execution_history[i - 1];
        r_execute_inverse(runtime, cell);
    }

    runtime->memory->execution_count = execution_index;
    return true;
}

bool r_step_forward(R_Runtime* runtime) {
    if (runtime->memory->execution_count >= runtime->memory->cell_count) {
        return false;
    }

    HRIR_Cell* cell = runtime->memory->cells[runtime->memory->execution_count];
    return r_execute_cell(runtime, cell);
}

bool r_step_backward(R_Runtime* runtime) {
    if (runtime->memory->execution_count == 0) {
        return false;
    }

    return r_rewind_to_index(runtime, runtime->memory->execution_count - 1);
}

// ============================================================================
// Homoiconicity Operations
// ============================================================================

const char* r_cell_to_string(HRIR_Cell* cell) {
    if (!cell) return NULL;

    static char buffer[512];
    snprintf(buffer, sizeof(buffer), "Cell#%u: %s(",
             cell->id, cell->opcode);

    for (uint32_t i = 0; i < cell->arg_count; i++) {
        if (i > 0) strcat(buffer, ", ");
        strcat(buffer, cell->args[i]);
    }
    strcat(buffer, ")");

    if (cell->canonical_path) {
        strcat(buffer, " @ ");
        strcat(buffer, cell->canonical_path);
    }

    return buffer;
}

// ============================================================================
// Qubit Access
// ============================================================================

uint8_t r_read_qubit(R_Runtime* runtime, uint32_t index) {
    if (index >= runtime->memory->qubit_count) return 0;
    return runtime->memory->qubits[index];
}

bool r_write_qubit(R_Runtime* runtime, uint32_t index, uint8_t value) {
    if (index >= runtime->memory->qubit_count) return false;
    runtime->memory->qubits[index] = value & 1;
    return true;
}

// ============================================================================
// Debugging and Introspection
// ============================================================================

void r_print_cell(HRIR_Cell* cell) {
    printf("%s\n", r_cell_to_string(cell));
}

void r_print_memory_state(R_Runtime* runtime) {
    printf("R-Layer Memory State (Instance %u):\n", runtime->instance_id);
    printf("  Qubits (%u): ", runtime->memory->qubit_count);
    for (uint32_t i = 0; i < runtime->memory->qubit_count && i < 32; i++) {
        printf("%u", runtime->memory->qubits[i]);
    }
    if (runtime->memory->qubit_count > 32) printf("...");
    printf("\n");

    printf("  Cells: %u/%u\n",
           runtime->memory->cell_count,
           runtime->memory->cell_capacity);

    printf("  Checkpoints: %u\n", runtime->memory->checkpoint_count);
    printf("  Execution history: %u ops\n", runtime->memory->execution_count);
}

void r_print_execution_history(R_Runtime* runtime) {
    printf("Execution History (%u operations):\n",
           runtime->memory->execution_count);

    for (uint32_t i = 0; i < runtime->memory->execution_count; i++) {
        printf("  [%u] ", i);
        r_print_cell(runtime->memory->execution_history[i]);
    }
}
