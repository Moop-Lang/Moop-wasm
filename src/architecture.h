// rio-riovn-merged/src/architecture.h
// Unified Rio+RioVN Architecture Definition

#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stddef.h>  // For size_t

// =============================================================================
// PHILOSOPHY EMBODIMENT
// =============================================================================

// This merged system embodies:
// - Minimalism: Two arrows at surface, single entrypoint
// - Conceptual Unification: Everything resolves to canonical Proto.Actor.Func
// - Orthogonality: Options vary independently (strict/autoHoist/debug)
// - Synergy: Rio reversibility + RioVN canonicalization = auditable system

// =============================================================================
// LAYER DEFINITIONS (Rio's Layers + RioVN Integration)
// =============================================================================

/*
L7: User Interface (Future - orthogonal extension)
L6: Application Logic (Future - orthogonal extension)
L5: Moop (Natural Language) - Unified surface with two arrows
L4: Rio (Prototypes) + RioVN (Canonicalization) - Forced hierarchy internally
L3: Turchin (Actors) - D-term coordination membranes
L2a: Prigogine (Reversible Functions) - R-term compute
L2b: Prigogine (D-term Gates) - Explicit irreversible coordination
L1: McCarthy (Reversible Operations) - R-term operational core
L0: Assembly - Target with reversible execution support
*/

// =============================================================================
// SURFACE SYNTAX (Minimal - Two Arrows Only)
// =============================================================================

// Core operators (preserved from both Rio and RioVN)
#define MESSAGE_ARROW "->"    // Send/message (R-term by default)
#define INHERIT_ARROW "<-"    // Inheritance (S-term)

// Optional tags for D-term boundaries (orthogonal extensions)
#define IRREVERSIBLE_TAG "@irreversible"  // Marks D-term operations
#define IO_TAG "@io"                      // Marks I/O operations

// =============================================================================
// COMPILER OPTIONS (Orthogonal Concerns)
// =============================================================================

typedef struct {
    bool strict_mode;     // Policy: Enforce explicit D-term tagging
    bool auto_hoist;      // Policy: Auto-generate synthetic hierarchy
    bool debug_mode;      // I/O: Verbose logging + stats
    bool reversible_default; // Always true - reversibility by default
} CompilerOptions;

// =============================================================================
// COMPILATION RESULT (Unified Output)
// =============================================================================

typedef struct {
    bool success;
    char* canonical_code;     // Resolved Proto.Actor.Func paths
    char* reversible_ir;      // L2a/L1 reversible intermediate representation
    char* membrane_logs;      // D-term boundary logging
    char* hrir_json;          // L1 HRIR JSON representation
    char** inheritance_graph; // Proto <- Parent relationships
    size_t path_count;
    size_t inheritance_count;

    // Debug-only stats
    struct {
        size_t canonical_paths_count;
        size_t inheritance_edges_count;
        size_t r_term_ops_count;
        size_t d_term_ops_count;
        size_t membrane_crossings_count;
        double compilation_time_ms;
        double validation_time_ms;
    } stats;

    // Diagnostics
    size_t error_count;
    char* first_error_message;
    size_t warning_count;
} CompilationResult;

// =============================================================================
// UNIFIED ENTRYPOINT (Conceptual Unification)
// =============================================================================

// Single compilation function for all layers
CompilationResult* compile(const char* code, CompilerOptions options);

// Memory management (unified cleanup)
void free_compilation_result(CompilationResult* result);

// =============================================================================
// LAYER INTERFACES (Orthogonal Contracts)
// =============================================================================

// Note: SurfaceAST is defined in surface_parser.h

// Note: Layer-specific structs would be defined here for full implementation
// For this unified demo, we focus on the surface parser and unified compiler

// =============================================================================
// COMPILER PIPELINE PHASES (Synergy)
// =============================================================================

/*
Phase 1: Parse Surface (L5/L4)
  - Tokenize arrows and optional tags
  - Build initial AST with inheritance relations
  - Detect syntax errors and ambiguities

Phase 2: Canonicalize (RioVN integration)
  - Resolve all sends to Proto.Actor.Func paths
  - Apply forced hierarchy internally
  - Auto-hoist sugar syntax
  - Validate inheritance chains

Phase 3: Classify R/D/S (Rio integration)
  - Classify operations by UME terms
  - Tag D-term boundaries (explicit or inferred)
  - Build reversible vs irreversible operation sets

Phase 4: Lower to Layers (Rio pipeline)
  - R-term: Generate reversible IR (L2a → L1)
  - D-term: Create membranes with journaling (L2b → L3)
  - S-term: Maintain prototype structure (L4)

Phase 5: Emit & Validate
  - Combine all layer outputs
  - Run validation (strict mode checks)
  - Generate debug stats if requested
  - Return unified CompilationResult
*/

#endif // ARCHITECTURE_H
