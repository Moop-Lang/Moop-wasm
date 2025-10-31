/**
 * HRIR Bridge - System-Wide Homoiconicity Inheritance
 * 
 * This module implements the bridge that propagates L1 HRIR homoiconicity
 * and structural reversibility throughout the entire Moop stack.
 * 
 * Philosophy: L1 HRIR cells are the atomic unit of ALL computation.
 * Every layer inherits homoiconicity and reversibility from L1.
 */

#ifndef HRIR_BRIDGE_H
#define HRIR_BRIDGE_H

#include "hr_ir.h"
#include "surface_parser.h"
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct LayerBridge LayerBridge;
typedef struct HRIRSequence HRIRSequence;
typedef struct ReversibilityContext ReversibilityContext;

/**
 * HRIR Sequence - A sequence of L1 HRIR cells representing higher-layer operations
 */
typedef struct HRIRSequence {
    HRIR_Cell** cells;           // Array of HRIR cell pointers
    size_t count;                // Number of cells
    size_t capacity;             // Allocated capacity
    char* layer_origin;          // Which layer generated this sequence (L2a, L2b, L3, etc.)
    char* canonical_path;        // Canonical path in the layer hierarchy
    bool is_reversible;          // Whether the entire sequence is reversible
    HRIR_Cell* inverse_sequence; // Inverse sequence (if reversible)
} HRIRSequence;

/**
 * Reversibility Context - Tracks reversibility state across layer boundaries
 */
typedef struct ReversibilityContext {
    HRIR_Runtime* l1_runtime;    // L1 runtime for reversible execution
    HRIRSequence** checkpoints;  // Checkpoint sequences for rollback
    size_t checkpoint_count;     // Number of checkpoints
    bool reversibility_enabled;  // Global reversibility flag
    char* current_layer;         // Current layer being processed
} ReversibilityContext;

/**
 * Layer Bridge - Connects higher layers to L1 HRIR foundation
 */
typedef struct LayerBridge {
    ReversibilityContext* context;  // Reversibility context
    HRIR_Program* l1_program;       // Underlying L1 HRIR program
    HRIRSequence** layer_sequences; // Sequences from each layer
    size_t sequence_count;          // Number of sequences
    bool homoiconicity_enabled;     // Global homoiconicity flag
} LayerBridge;

// === HRIR Sequence Management ===

/**
 * Create a new HRIR sequence for a specific layer
 */
HRIRSequence* hrir_sequence_create(const char* layer_origin, const char* canonical_path);

/**
 * Add an HRIR cell to a sequence
 */
bool hrir_sequence_add_cell(HRIRSequence* sequence, HRIR_Cell* cell);

/**
 * Mark a sequence as reversible and generate its inverse
 */
bool hrir_sequence_make_reversible(HRIRSequence* sequence);

/**
 * Free an HRIR sequence and all its cells
 */
void hrir_sequence_free(HRIRSequence* sequence);

// === Layer Bridge Management ===

/**
 * Create a layer bridge with L1 HRIR foundation
 */
LayerBridge* layer_bridge_create(HRIR_Program* l1_program);

/**
 * Add a sequence from a higher layer to the bridge
 */
bool layer_bridge_add_sequence(LayerBridge* bridge, HRIRSequence* sequence);

/**
 * Execute all sequences through L1 HRIR runtime
 */
bool layer_bridge_execute(LayerBridge* bridge);

/**
 * Rollback to a previous checkpoint (system-wide reversibility)
 */
bool layer_bridge_rollback(LayerBridge* bridge, size_t checkpoint_index);

/**
 * Free the layer bridge and all associated resources
 */
void layer_bridge_free(LayerBridge* bridge);

// === L2 Prigogine Integration ===

/**
 * Convert L2a reversible functions to HRIR sequences
 */
HRIRSequence* l2a_to_hrir(const char* l2a_code, const char* canonical_path);

/**
 * Convert L2b coordination operations to HRIR sequences
 */
HRIRSequence* l2b_to_hrir(const char* l2b_code, const char* canonical_path);

// === L3 Turchin Integration ===

/**
 * Convert L3 actor messages to HRIR sequences
 */
HRIRSequence* l3_actor_to_hrir(const char* actor_code, const char* canonical_path);

/**
 * Convert L3 message passing to reversible HRIR operations
 */
HRIRSequence* l3_message_to_hrir(const char* message_code, const char* canonical_path);

// === L4 Rio Integration ===

/**
 * Convert L4 prototype operations to HRIR sequences
 */
HRIRSequence* l4_prototype_to_hrir(const char* proto_code, const char* canonical_path);

/**
 * Convert L4 inheritance declarations to HRIR sequences
 */
HRIRSequence* l4_inheritance_to_hrir(const char* inheritance_code, const char* canonical_path);

// === L5 Moop Integration ===

/**
 * Convert L5 natural language to HRIR sequences
 */
HRIRSequence* l5_natural_to_hrir(const char* natural_code, const char* canonical_path);

// === System-Wide Homoiconicity ===

/**
 * Serialize the entire layer bridge to JSON (homoiconic representation)
 */
char* layer_bridge_to_json(LayerBridge* bridge);

/**
 * Deserialize a layer bridge from JSON
 */
LayerBridge* layer_bridge_from_json(const char* json);

/**
 * Get homoiconic access to any layer's HRIR representation
 */
HRIRSequence* layer_bridge_get_layer_hrir(LayerBridge* bridge, const char* layer_name);

/**
 * Mutate any layer's HRIR representation at runtime (live programming)
 */
bool layer_bridge_mutate_layer_hrir(LayerBridge* bridge, const char* layer_name, HRIRSequence* new_sequence);

// === Reversibility Inheritance ===

/**
 * Create a reversibility context for system-wide reversibility
 */
ReversibilityContext* reversibility_context_create(HRIR_Runtime* l1_runtime);

/**
 * Create a checkpoint across all layers
 */
bool reversibility_context_checkpoint(ReversibilityContext* context, LayerBridge* bridge);

/**
 * Rollback all layers to a previous checkpoint
 */
bool reversibility_context_rollback(ReversibilityContext* context, LayerBridge* bridge, size_t checkpoint_index);

/**
 * Free the reversibility context
 */
void reversibility_context_free(ReversibilityContext* context);

#endif // HRIR_BRIDGE_H
