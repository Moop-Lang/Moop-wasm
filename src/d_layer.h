// d_layer.h
// D-Layer: Dissipative Operations (User-Facing)
// Hosted on R-Layer, provides irreversible gates

#ifndef D_LAYER_H
#define D_LAYER_H

#include "r_layer.h"

// ============================================================================
// D-Layer: Irreversible gates implemented via R-layer substrate
// ============================================================================

typedef struct D_Runtime {
    R_Runtime* r_runtime;           // R-layer substrate (inherited)

    // Ancilla qubits for irreversible operations
    uint32_t ancilla_start;         // First ancilla qubit index
    uint32_t ancilla_count;         // Number of ancilla qubits
} D_Runtime;

// ============================================================================
// D-Layer API
// ============================================================================

// Runtime lifecycle
D_Runtime* d_runtime_init(uint32_t qubit_count, uint32_t ancilla_count, uint32_t instance_id);
void d_runtime_free(D_Runtime* runtime);

// The 6 irreversible gates (user-facing)
bool d_execute_AND(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result);
bool d_execute_OR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result);
bool d_execute_NAND(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result);
bool d_execute_NOR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result);
bool d_execute_XOR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result);

// MAYBE primitive (superposition/ambiguity)
typedef struct D_Maybe {
    bool is_resolved;
    bool value;                     // Resolved value
    void* superposition_data;       // Unresolved state (for LLM resolution)
} D_Maybe;

D_Maybe* d_create_maybe(void* superposition_data);
bool d_resolve_maybe(D_Maybe* maybe, bool value);
bool d_is_resolved(D_Maybe* maybe);
void d_free_maybe(D_Maybe* maybe);

// Actor coordination (D-layer)
typedef struct D_Actor {
    uint32_t id;
    const char* name;
    const char* role;

    // State (hosted on R-layer memory)
    void* state;                    // Actor state data
    uint32_t state_size;

    // Message queue
    struct D_Message** queue;
    uint32_t queue_size;
    uint32_t queue_capacity;

    // Handlers
    struct D_Handler** handlers;
    uint32_t handler_count;
} D_Actor;

typedef struct D_Message {
    const char* name;
    void* payload;
    uint32_t payload_size;
} D_Message;

typedef struct D_Handler {
    const char* message_name;
    void (*handler_func)(D_Actor* actor, D_Message* msg);
} D_Handler;

// Actor operations
D_Actor* d_create_actor(D_Runtime* runtime, const char* name, const char* role);
void d_free_actor(D_Actor* actor);
bool d_send_message(D_Actor* actor, const char* message_name, void* payload, uint32_t size);
bool d_process_messages(D_Actor* actor);

// Inherit R-layer operations
static inline R_Checkpoint* d_save_checkpoint(D_Runtime* d_runtime, const char* label) {
    return r_save_checkpoint(d_runtime->r_runtime, label);
}

static inline bool d_restore_checkpoint(D_Runtime* d_runtime, R_Checkpoint* checkpoint) {
    return r_restore_checkpoint(d_runtime->r_runtime, checkpoint);
}

static inline bool d_step_backward(D_Runtime* d_runtime) {
    return r_step_backward(d_runtime->r_runtime);
}

static inline bool d_step_forward(D_Runtime* d_runtime) {
    return r_step_forward(d_runtime->r_runtime);
}

// Debugging
void d_print_runtime_state(D_Runtime* runtime);

#endif // D_LAYER_H
