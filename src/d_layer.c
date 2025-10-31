// d_layer.c
// D-Layer: Dissipative Operations Implementation

#include "d_layer.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Runtime Lifecycle
// ============================================================================

D_Runtime* d_runtime_init(uint32_t qubit_count, uint32_t ancilla_count, uint32_t instance_id) {
    D_Runtime* runtime = (D_Runtime*)malloc(sizeof(D_Runtime));
    if (!runtime) return NULL;

    // Initialize R-layer substrate
    runtime->r_runtime = r_runtime_init(qubit_count + ancilla_count, instance_id);
    if (!runtime->r_runtime) {
        free(runtime);
        return NULL;
    }

    runtime->ancilla_start = qubit_count;
    runtime->ancilla_count = ancilla_count;

    return runtime;
}

void d_runtime_free(D_Runtime* runtime) {
    if (!runtime) return;
    r_runtime_free(runtime->r_runtime);
    free(runtime);
}

// ============================================================================
// Irreversible Gates (Implemented via R-layer)
// ============================================================================

// AND gate: result = a AND b
// Implemented using Toffoli (CCNOT) gate
bool d_execute_AND(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result) {
    // Use Toffoli: CCNOT a b result
    // This is irreversible because we lose the original value of 'result'

    // First, clear result qubit
    uint8_t result_val = r_read_qubit(runtime->r_runtime, result);
    if (result_val) {
        r_execute_NOT(runtime->r_runtime, result);
    }

    // Execute Toffoli
    return r_execute_CCNOT(runtime->r_runtime, a, b, result);
}

// OR gate: result = a OR b
// Implemented using De Morgan's law: a OR b = NOT(NOT a AND NOT b)
bool d_execute_OR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result) {
    if (runtime->ancilla_count < 2) return false;

    uint32_t ancilla_a = runtime->ancilla_start;
    uint32_t ancilla_b = runtime->ancilla_start + 1;

    // Copy and invert a and b
    r_write_qubit(runtime->r_runtime, ancilla_a, r_read_qubit(runtime->r_runtime, a));
    r_execute_NOT(runtime->r_runtime, ancilla_a);

    r_write_qubit(runtime->r_runtime, ancilla_b, r_read_qubit(runtime->r_runtime, b));
    r_execute_NOT(runtime->r_runtime, ancilla_b);

    // AND the inverted values
    d_execute_AND(runtime, ancilla_a, ancilla_b, result);

    // Invert result
    r_execute_NOT(runtime->r_runtime, result);

    return true;
}

// NAND gate: result = NOT(a AND b)
bool d_execute_NAND(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result) {
    d_execute_AND(runtime, a, b, result);
    r_execute_NOT(runtime->r_runtime, result);
    return true;
}

// NOR gate: result = NOT(a OR b)
bool d_execute_NOR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result) {
    d_execute_OR(runtime, a, b, result);
    r_execute_NOT(runtime->r_runtime, result);
    return true;
}

// XOR gate: result = a XOR b
// Implemented using CNOTs
bool d_execute_XOR(D_Runtime* runtime, uint32_t a, uint32_t b, uint32_t result) {
    // Clear result
    uint8_t result_val = r_read_qubit(runtime->r_runtime, result);
    if (result_val) {
        r_execute_NOT(runtime->r_runtime, result);
    }

    // XOR using CNOT chain
    r_execute_CNOT(runtime->r_runtime, a, result);
    r_execute_CNOT(runtime->r_runtime, b, result);

    return true;
}

// ============================================================================
// MAYBE Primitive (Superposition/Ambiguity)
// ============================================================================

D_Maybe* d_create_maybe(void* superposition_data) {
    D_Maybe* maybe = (D_Maybe*)malloc(sizeof(D_Maybe));
    if (!maybe) return NULL;

    maybe->is_resolved = false;
    maybe->value = false;
    maybe->superposition_data = superposition_data;

    return maybe;
}

bool d_resolve_maybe(D_Maybe* maybe, bool value) {
    if (!maybe || maybe->is_resolved) return false;

    maybe->is_resolved = true;
    maybe->value = value;

    return true;
}

bool d_is_resolved(D_Maybe* maybe) {
    return maybe && maybe->is_resolved;
}

void d_free_maybe(D_Maybe* maybe) {
    if (!maybe) return;
    // Note: superposition_data is not freed here - caller manages it
    free(maybe);
}

// ============================================================================
// Actor Coordination (D-layer)
// ============================================================================

D_Actor* d_create_actor(D_Runtime* runtime, const char* name, const char* role) {
    (void)runtime;  // Will use for memory allocation later

    D_Actor* actor = (D_Actor*)malloc(sizeof(D_Actor));
    if (!actor) return NULL;

    static uint32_t next_id = 0;
    actor->id = next_id++;
    actor->name = name ? strdup(name) : NULL;
    actor->role = role ? strdup(role) : NULL;

    actor->state = NULL;
    actor->state_size = 0;

    // Initialize message queue
    actor->queue_capacity = 16;
    actor->queue = (D_Message**)calloc(actor->queue_capacity, sizeof(D_Message*));
    actor->queue_size = 0;

    // Initialize handlers
    actor->handlers = NULL;
    actor->handler_count = 0;

    return actor;
}

void d_free_actor(D_Actor* actor) {
    if (!actor) return;

    free((void*)actor->name);
    free((void*)actor->role);
    free(actor->state);

    // Free message queue
    for (uint32_t i = 0; i < actor->queue_size; i++) {
        D_Message* msg = actor->queue[i];
        free((void*)msg->name);
        free(msg->payload);
        free(msg);
    }
    free(actor->queue);

    // Free handlers
    for (uint32_t i = 0; i < actor->handler_count; i++) {
        free((void*)actor->handlers[i]->message_name);
        free(actor->handlers[i]);
    }
    free(actor->handlers);

    free(actor);
}

bool d_send_message(D_Actor* actor, const char* message_name, void* payload, uint32_t size) {
    if (!actor || !message_name) return false;

    // Create message
    D_Message* msg = (D_Message*)malloc(sizeof(D_Message));
    if (!msg) return false;

    msg->name = strdup(message_name);
    msg->payload = NULL;
    msg->payload_size = size;

    if (payload && size > 0) {
        msg->payload = malloc(size);
        memcpy(msg->payload, payload, size);
    }

    // Add to queue
    if (actor->queue_size >= actor->queue_capacity) {
        actor->queue_capacity *= 2;
        actor->queue = (D_Message**)realloc(
            actor->queue,
            actor->queue_capacity * sizeof(D_Message*)
        );
    }

    actor->queue[actor->queue_size++] = msg;

    return true;
}

bool d_process_messages(D_Actor* actor) {
    if (!actor || actor->queue_size == 0) return false;

    // Process first message
    D_Message* msg = actor->queue[0];

    // Find handler
    for (uint32_t i = 0; i < actor->handler_count; i++) {
        if (strcmp(actor->handlers[i]->message_name, msg->name) == 0) {
            actor->handlers[i]->handler_func(actor, msg);
            break;
        }
    }

    // Remove from queue
    free((void*)msg->name);
    free(msg->payload);
    free(msg);

    // Shift queue
    for (uint32_t i = 0; i < actor->queue_size - 1; i++) {
        actor->queue[i] = actor->queue[i + 1];
    }
    actor->queue_size--;

    return true;
}

// ============================================================================
// Debugging
// ============================================================================

void d_print_runtime_state(D_Runtime* runtime) {
    printf("D-Layer Runtime State:\n");
    printf("  Ancilla qubits: %u (starting at %u)\n",
           runtime->ancilla_count,
           runtime->ancilla_start);

    printf("\n");
    r_print_memory_state(runtime->r_runtime);
}
