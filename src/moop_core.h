// moop_core.h
// Minimal Three-Layer Moop Implementation
// Properties: Turing-complete, Homoiconic, Reversible

#ifndef MOOP_CORE_H
#define MOOP_CORE_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// R-LAYER: Reversible Substrate (4 primitives)
// ============================================================================

// Minimal HRIR Cell: 4 bytes
typedef struct {
    uint8_t gate;      // 0=CCNOT, 1=CNOT, 2=NOT, 3=SWAP
    uint8_t a, b, c;   // Qubit indices (0-255)
} R_Cell;

// R-Layer Runtime
typedef struct {
    uint8_t* qubits;           // Qubit states
    R_Cell* history;           // Execution history (homoiconic)
    uint32_t qubit_count;
    uint32_t history_count;
    uint32_t history_capacity;
    uint32_t instance_id;      // Program instance isolation
} R_Runtime;

// R-Layer API (Minimal)
R_Runtime* r_init(uint32_t qubits, uint32_t instance_id);
void r_free(R_Runtime* r);

// Execute gates (records to history automatically)
void r_CCNOT(R_Runtime* r, uint8_t a, uint8_t b, uint8_t c);
void r_CNOT(R_Runtime* r, uint8_t a, uint8_t b);
void r_NOT(R_Runtime* r, uint8_t a);
void r_SWAP(R_Runtime* r, uint8_t a, uint8_t b);

// Reversibility
uint32_t r_checkpoint(R_Runtime* r);
void r_restore(R_Runtime* r, uint32_t checkpoint);
void r_step_back(R_Runtime* r);

// Homoiconicity
R_Cell r_parse(const char* str);
const char* r_print(R_Cell cell);

// ============================================================================
// S-LAYER: Coordination Primitive (actor-proto duality)
// ============================================================================

// Forward declarations
typedef struct S_Actor S_Actor;
typedef struct S_Proto S_Proto;

// The coordination primitive has two dual aspects:
// 1. Actor (minimal binding)
// 2. Proto (maximum binding)

struct S_Proto {
    const char* name;
    S_Proto* parent;           // Inheritance (maximum binding)
    void* slots;               // Data slots
    uint32_t slot_count;
};

struct S_Actor {
    const char* name;
    const char* role;
    S_Proto* prototype;        // Structure from proto
    void* state;               // Local state (minimal binding)
    uint32_t state_size;
    void** handlers;           // Message handlers
    uint32_t handler_count;
};

// S-Layer Runtime (per program instance)
typedef struct {
    R_Runtime* r_runtime;      // R-layer substrate
    S_Actor* root_actor;       // Temporal coordinator (minimal binding)
    S_Proto* root_proto;       // Structural coordinator (maximum binding)
    S_Actor** actors;
    S_Proto** protos;
    uint32_t actor_count;
    uint32_t proto_count;
    uint32_t instance_id;
} S_Runtime;

// S-Layer API
S_Runtime* s_init(uint32_t qubits, uint32_t instance_id);
void s_free(S_Runtime* s);

// Actor operations (minimal binding)
S_Actor* s_create_actor(S_Runtime* s, const char* name, const char* role);
void s_send_message(S_Actor* actor, const char* msg);

// Proto operations (maximum binding)
S_Proto* s_create_proto(S_Runtime* s, const char* name, S_Proto* parent);

// ============================================================================
// D-LAYER: Emergent Operations (controlled by R + S primitives)
// ============================================================================

// D-layer has NO primitives - these operations emerge from R-layer gates
// controlled by S-layer organization (actor/proto)

// Irreversible gates (implemented via R-layer)
void d_AND(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result);
void d_OR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result);
void d_XOR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result);
void d_NAND(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result);
void d_NOR(R_Runtime* r, uint8_t a, uint8_t b, uint8_t result);

// MAYBE primitive (homoiconic data structure)
typedef struct {
    bool resolved;
    bool value;
} D_Maybe;

D_Maybe d_maybe();
void d_resolve(D_Maybe* m, bool value);

#endif // MOOP_CORE_H
