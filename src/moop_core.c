// moop_core.c
// Minimal Three-Layer Moop Implementation (~200 lines)

#include "moop_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// R-LAYER IMPLEMENTATION
// ============================================================================

R_Runtime* r_init(uint32_t qubits, uint32_t instance_id) {
    R_Runtime* r = malloc(sizeof(R_Runtime));
    r->qubits = calloc(qubits, 1);
    r->history = malloc(4096 * sizeof(R_Cell));
    r->qubit_count = qubits;
    r->history_count = 0;
    r->history_capacity = 4096;
    r->instance_id = instance_id;
    return r;
}

void r_free(R_Runtime* r) {
    free(r->qubits);
    free(r->history);
    free(r);
}

// Execute gates (homoiconic - automatically record to history)
void r_CCNOT(R_Runtime* r, uint8_t a, uint8_t b, uint8_t c) {
    if (r->qubits[a] && r->qubits[b]) r->qubits[c] ^= 1;
    r->history[r->history_count++] = (R_Cell){0, a, b, c};
}

void r_CNOT(R_Runtime* r, uint8_t a, uint8_t b) {
    if (r->qubits[a]) r->qubits[b] ^= 1;
    r->history[r->history_count++] = (R_Cell){1, a, b, 0};
}

void r_NOT(R_Runtime* r, uint8_t a) {
    r->qubits[a] ^= 1;
    r->history[r->history_count++] = (R_Cell){2, a, 0, 0};
}

void r_SWAP(R_Runtime* r, uint8_t a, uint8_t b) {
    uint8_t t = r->qubits[a];
    r->qubits[a] = r->qubits[b];
    r->qubits[b] = t;
    r->history[r->history_count++] = (R_Cell){3, a, b, 0};
}

// Reversibility (exploit self-inverse property)
uint32_t r_checkpoint(R_Runtime* r) {
    return r->history_count;
}

void r_restore(R_Runtime* r, uint32_t checkpoint) {
    // All R-layer gates are self-inverse: executing again = undo
    while (r->history_count > checkpoint) {
        R_Cell c = r->history[--r->history_count];
        switch(c.gate) {
            case 0: r_CCNOT(r, c.a, c.b, c.c); r->history_count--; break;
            case 1: r_CNOT(r, c.a, c.b); r->history_count--; break;
            case 2: r_NOT(r, c.a); r->history_count--; break;
            case 3: r_SWAP(r, c.a, c.b); r->history_count--; break;
        }
    }
}

void r_step_back(R_Runtime* r) {
    if (r->history_count > 0) {
        R_Cell c = r->history[--r->history_count];
        switch(c.gate) {
            case 0: if (r->qubits[c.a] && r->qubits[c.b]) r->qubits[c.c] ^= 1; break;
            case 1: if (r->qubits[c.a]) r->qubits[c.b] ^= 1; break;
            case 2: r->qubits[c.a] ^= 1; break;
            case 3: { uint8_t t = r->qubits[c.a]; r->qubits[c.a] = r->qubits[c.b]; r->qubits[c.b] = t; } break;
        }
    }
}

// Homoiconicity (gates as data, data as gates)
R_Cell r_parse(const char* str) {
    R_Cell c = {0};
    if (strstr(str, "CCNOT")) { sscanf(str, "CCNOT %hhu %hhu %hhu", &c.a, &c.b, &c.c); c.gate = 0; }
    else if (strstr(str, "CNOT")) { sscanf(str, "CNOT %hhu %hhu", &c.a, &c.b); c.gate = 1; }
    else if (strstr(str, "NOT")) { sscanf(str, "NOT %hhu", &c.a); c.gate = 2; }
    else if (strstr(str, "SWAP")) { sscanf(str, "SWAP %hhu %hhu", &c.a, &c.b); c.gate = 3; }
    return c;
}

const char* r_print(R_Cell c) {
    static char buf[64];
    const char* gates[] = {"CCNOT", "CNOT", "NOT", "SWAP"};
    sprintf(buf, "%s %d %d %d", gates[c.gate], c.a, c.b, c.c);
    return buf;
}

// ============================================================================
// S-LAYER IMPLEMENTATION
// ============================================================================

S_Runtime* s_init(uint32_t qubits, uint32_t instance_id) {
    S_Runtime* s = malloc(sizeof(S_Runtime));

    // Initialize R-layer substrate
    s->r_runtime = r_init(qubits, instance_id);
    s->instance_id = instance_id;

    // Create the dual: root_actor + root_proto emerge simultaneously
    s->root_proto = malloc(sizeof(S_Proto));
    s->root_proto->name = "root_proto";
    s->root_proto->parent = NULL;  // Root has no parent
    s->root_proto->slots = NULL;
    s->root_proto->slot_count = 0;

    s->root_actor = malloc(sizeof(S_Actor));
    s->root_actor->name = "root_actor";
    s->root_actor->role = "Bootstrap coordinator";
    s->root_actor->prototype = s->root_proto;  // Actor references proto
    s->root_actor->state = NULL;
    s->root_actor->state_size = 0;
    s->root_actor->handlers = NULL;
    s->root_actor->handler_count = 0;

    // Initialize collections
    s->actors = malloc(256 * sizeof(S_Actor*));
    s->protos = malloc(256 * sizeof(S_Proto*));
    s->actors[0] = s->root_actor;
    s->protos[0] = s->root_proto;
    s->actor_count = 1;
    s->proto_count = 1;

    return s;
}

void s_free(S_Runtime* s) {
    // Free actors
    for (uint32_t i = 0; i < s->actor_count; i++) {
        free(s->actors[i]->state);
        free(s->actors[i]->handlers);
        free(s->actors[i]);
    }
    free(s->actors);

    // Free protos
    for (uint32_t i = 0; i < s->proto_count; i++) {
        free(s->protos[i]->slots);
        free(s->protos[i]);
    }
    free(s->protos);

    r_free(s->r_runtime);
    free(s);
}

// Actor operations (minimal binding)
S_Actor* s_create_actor(S_Runtime* s, const char* name, const char* role) {
    S_Actor* actor = malloc(sizeof(S_Actor));
    actor->name = name;
    actor->role = role;
    actor->prototype = s->root_proto;  // Inherit from root
    actor->state = NULL;
    actor->state_size = 0;
    actor->handlers = NULL;
    actor->handler_count = 0;

    s->actors[s->actor_count++] = actor;
    return actor;
}

void s_send_message(S_Actor* actor, const char* msg) {
    // Minimal message handling (extend as needed)
    printf("Actor '%s' received message: %s\n", actor->name, msg);
}

// Proto operations (maximum binding)
S_Proto* s_create_proto(S_Runtime* s, const char* name, S_Proto* parent) {
    S_Proto* proto = malloc(sizeof(S_Proto));
    proto->name = name;
    proto->parent = parent ? parent : s->root_proto;  // Default to root
    proto->slots = NULL;
    proto->slot_count = 0;

    s->protos[s->proto_count++] = proto;
    return proto;
}

// ============================================================================
// D-LAYER IMPLEMENTATION (Emergent operations)
// ============================================================================

// AND: controlled by CCNOT (R-layer)
void d_AND(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result) {
    // Clear result first (irreversible semantic)
    if (r->qubits[result]) r_NOT(r, result);
    // Use Toffoli
    r_CCNOT(r, a, b, result);
}

// OR: controlled by CCNOT + NOT (R-layer)
void d_OR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result) {
    // De Morgan: a OR b = NOT(NOT a AND NOT b)
    // Requires ancilla qubits (simplified here)
    r_NOT(r, a);
    r_NOT(r, b);
    d_AND(r, a, b, result);
    r_NOT(r, result);
    r_NOT(r, a);  // Restore
    r_NOT(r, b);  // Restore
}

// XOR: controlled by CNOT (R-layer)
void d_XOR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result) {
    if (r->qubits[result]) r_NOT(r, result);
    r_CNOT(r, a, result);
    r_CNOT(r, b, result);
}

// NAND: controlled by CCNOT + NOT (R-layer)
void d_NAND(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result) {
    d_AND(r, a, b, result);
    r_NOT(r, result);
}

// NOR: controlled by CCNOT + NOT (R-layer)
void d_NOR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result) {
    d_OR(r, a, b, result);
    r_NOT(r, result);
}

// MAYBE: homoiconic data structure (not a gate)
D_Maybe d_maybe() {
    return (D_Maybe){false, false};
}

void d_resolve(D_Maybe* m, bool value) {
    m->resolved = true;
    m->value = value;
}
