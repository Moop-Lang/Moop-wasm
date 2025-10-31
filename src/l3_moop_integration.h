// src/l3_moop_integration.h
// L3 Turchin Integration with Moop Stack (L5→L4→L3→L2→L1)
// This version integrates with the existing Moop compiler architecture

#ifndef L3_MOOP_INTEGRATION_H
#define L3_MOOP_INTEGRATION_H

#include "l3_turchin.h"
#include "l5_moop.h"
#include "hr_ir.h"
#include "architecture.h"

// =============================================================================
// L3 MOOP INTEGRATION - Connect Turchin Actors to Moop Stack
// =============================================================================

// L3 Actor compiled from L5 Moop code
typedef struct L3_MoopActor {
    L3_ActorDefinition* actor_def;
    L5_Statement* originating_statement;
    HRIR_Cell* hrir_cell;
    bool is_reversible;
    bool is_homoiconic;
    char* inheritance_path;  // e.g. "IfTester <- root_proto"
} L3_MoopActor;

// L3 Runtime with Moop Stack Integration
typedef struct L3_MoopRuntime {
    L3_ActorRuntime* base_runtime;
    L5_MoopProgram* moop_program;
    HRIR_Program* hrir_program;

    // Moop stack features
    bool time_travel_enabled;
    bool reversibility_tracking;
    CompilerOptions compiler_options;

    // Actor registry with Moop metadata
    L3_MoopActor** moop_actors;
    size_t moop_actor_count;
    size_t moop_actor_capacity;
} L3_MoopRuntime;

// =============================================================================
// COMPILATION: L5 Moop → L3 Turchin Actors
// =============================================================================

// Compile L5 Moop code into L3 Turchin actors
L3_MoopRuntime* l3_compile_from_moop(const char* moop_code, L5_CompileOptions l5_opts);

// Parse L5 statement into L3 actor definition
L3_MoopActor* l3_parse_moop_statement(L5_Statement* stmt, CompilerOptions opts);

// Generate HRIR cells for L3 actor operations
HRIR_Cell* l3_generate_hrir_for_actor(L3_ActorDefinition* actor_def, const char* inheritance_path);

// =============================================================================
// EXECUTION: Reversible Actor Runtime
// =============================================================================

// Execute with Moop stack features (time-travel, reversibility)
bool l3_execute_moop_runtime(L3_MoopRuntime* runtime);

// Time-travel operations (inherited from L5)
bool l3_undo_moop(L3_MoopRuntime* runtime, int steps);
bool l3_rollback_moop(L3_MoopRuntime* runtime, const char* checkpoint_id);
char* l3_create_moop_checkpoint(L3_MoopRuntime* runtime);

// =============================================================================
// INTEGRATION: Bridge L3 ↔ L5 ↔ L1
// =============================================================================

// Convert L3 actor to L5 homoiconic representation
L5_Statement* l3_to_l5_statement(L3_ActorDefinition* actor_def);

// Convert L3 actor to L1 HRIR cells
HRIR_Program* l3_to_hrir_program(L3_ActorRuntime* runtime);

// Apply L4 inheritance to L3 actors (Proto.Actor.Func)
void l3_apply_inheritance(L3_MoopActor* actor, const char* parent_proto);

// =============================================================================
// MEMBRANE TRACKING: D-term Boundaries
// =============================================================================

// Track D-term operations (irreversible boundaries)
typedef struct L3_MembraneLog {
    int actor_id;
    const char* event;
    const char* operation;
    bool is_reversible;
    double timestamp;
    char* membrane_crossing_type;  // "io", "external", "irreversible"
} L3_MembraneLog;

// Log D-term boundary crossing
void l3_log_membrane_crossing(L3_MoopRuntime* runtime, L3_MembraneLog log);

// Get all membrane logs (for debugging/auditing)
L3_MembraneLog** l3_get_membrane_logs(L3_MoopRuntime* runtime, size_t* count);

// =============================================================================
// MOOP STACK OPTIONS
// =============================================================================

typedef struct L3_MoopOptions {
    // L5 options
    bool enhanced_moop;
    bool auto_inherit;
    bool enable_time_travel;

    // L4 options (Proto.Actor.Func)
    bool auto_hoist;
    bool strict_mode;

    // L3 options (Turchin)
    bool enable_control_flow;
    bool track_membranes;

    // L1 options (HRIR)
    bool generate_hrir;
    bool reversible_default;

    // Debug
    bool debug_mode;
} L3_MoopOptions;

// Default options for Moop stack integration
L3_MoopOptions l3_default_moop_options(void);

// =============================================================================
// COMPILATION RESULT
// =============================================================================

typedef struct L3_MoopCompileResult {
    L3_MoopRuntime* runtime;
    CompilationResult* compiler_result;
    L5_CompileResult* l5_result;

    bool success;
    char* error_message;

    // Statistics
    struct {
        size_t actor_count;
        size_t hrir_cell_count;
        size_t reversible_ops;
        size_t irreversible_ops;
        size_t membrane_crossings;
        double compilation_time_ms;
    } stats;
} L3_MoopCompileResult;

// Main compilation entrypoint
L3_MoopCompileResult* l3_compile_moop_stack(const char* moop_code, L3_MoopOptions opts);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void l3_free_moop_runtime(L3_MoopRuntime* runtime);
void l3_free_moop_actor(L3_MoopActor* actor);
void l3_free_moop_compile_result(L3_MoopCompileResult* result);

#endif // L3_MOOP_INTEGRATION_H
