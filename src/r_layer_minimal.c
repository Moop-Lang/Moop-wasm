// r_layer_minimal.c
// Minimal R-Layer: ~100 lines for complete reversible substrate

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Minimal HRIR Cell: 4 bytes
// ============================================================================

typedef struct {
    uint8_t gate;      // 0=CCNOT, 1=CNOT, 2=NOT, 3=SWAP
    uint8_t a, b, c;   // Qubit indices
} Cell;

// ============================================================================
// Minimal Runtime: Single allocation
// ============================================================================

typedef struct {
    uint8_t* qubits;       // Qubit states
    Cell* cells;           // Execution history
    uint32_t qubit_count;
    uint32_t cell_count;
    uint32_t cell_capacity;
} Runtime;

// ============================================================================
// Init/Free: 10 lines
// ============================================================================

Runtime* init(uint32_t qubits) {
    Runtime* r = malloc(sizeof(Runtime));
    r->qubits = calloc(qubits, 1);
    r->cells = malloc(4096 * sizeof(Cell));
    r->qubit_count = qubits;
    r->cell_count = 0;
    r->cell_capacity = 4096;
    return r;
}

void cleanup(Runtime* r) {
    free(r->qubits);
    free(r->cells);
    free(r);
}

// ============================================================================
// Execute gates: 30 lines
// ============================================================================

void exec_CCNOT(Runtime* r, uint8_t a, uint8_t b, uint8_t c) {
    if (r->qubits[a] && r->qubits[b]) r->qubits[c] ^= 1;
    r->cells[r->cell_count++] = (Cell){0, a, b, c};
}

void exec_CNOT(Runtime* r, uint8_t a, uint8_t b) {
    if (r->qubits[a]) r->qubits[b] ^= 1;
    r->cells[r->cell_count++] = (Cell){1, a, b, 0};
}

void exec_NOT(Runtime* r, uint8_t a) {
    r->qubits[a] ^= 1;
    r->cells[r->cell_count++] = (Cell){2, a, 0, 0};
}

void exec_SWAP(Runtime* r, uint8_t a, uint8_t b) {
    uint8_t t = r->qubits[a];
    r->qubits[a] = r->qubits[b];
    r->qubits[b] = t;
    r->cells[r->cell_count++] = (Cell){3, a, b, 0};
}

void exec_cell(Runtime* r, Cell c) {
    switch(c.gate) {
        case 0: exec_CCNOT(r, c.a, c.b, c.c); break;
        case 1: exec_CNOT(r, c.a, c.b); break;
        case 2: exec_NOT(r, c.a); break;
        case 3: exec_SWAP(r, c.a, c.b); break;
    }
}

// ============================================================================
// Reversibility: 20 lines
// ============================================================================

uint32_t checkpoint(Runtime* r) {
    return r->cell_count;  // Just return index
}

void restore(Runtime* r, uint32_t checkpoint) {
    // Rewind by executing inverses
    while (r->cell_count > checkpoint) {
        exec_cell(r, r->cells[--r->cell_count]);  // All gates self-inverse
    }
}

void step_back(Runtime* r) {
    if (r->cell_count > 0) {
        exec_cell(r, r->cells[--r->cell_count]);
    }
}

void step_forward(Runtime* r) {
    if (r->cell_count < r->cell_capacity) {
        exec_cell(r, r->cells[r->cell_count]);
    }
}

// ============================================================================
// Homoiconicity: 15 lines
// ============================================================================

Cell parse_cell(const char* str) {
    // Parse "CCNOT 0 1 2" -> Cell{0, 0, 1, 2}
    Cell c = {0};
    if (strstr(str, "CCNOT")) { sscanf(str, "CCNOT %hhu %hhu %hhu", &c.a, &c.b, &c.c); c.gate = 0; }
    else if (strstr(str, "CNOT")) { sscanf(str, "CNOT %hhu %hhu", &c.a, &c.b); c.gate = 1; }
    else if (strstr(str, "NOT")) { sscanf(str, "NOT %hhu", &c.a); c.gate = 2; }
    else if (strstr(str, "SWAP")) { sscanf(str, "SWAP %hhu %hhu", &c.a, &c.b); c.gate = 3; }
    return c;
}

const char* cell_to_str(Cell c) {
    static char buf[64];
    const char* gates[] = {"CCNOT", "CNOT", "NOT", "SWAP"};
    sprintf(buf, "%s %d %d %d", gates[c.gate], c.a, c.b, c.c);
    return buf;
}

// ============================================================================
// TOTAL: ~100 lines of actual code
// ============================================================================
