// rio-riovn-merged/src/consistency_checker.h
// Consistency Checker - Validates Dual-Memory System Integrity

#ifndef CONSISTENCY_CHECKER_H
#define CONSISTENCY_CHECKER_H

#include "hr_ir.h"
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// CONSISTENCY CHECKER TYPES
// =============================================================================

typedef struct ConsistencyResult {
    bool is_consistent;
    const char* error_message;
    size_t operations_checked;
    size_t side_effects_verified;
} ConsistencyResult;

typedef struct ExpectedSideEffect {
    const char* operation;
    const char** args;
    size_t arg_count;
    bool should_succeed;
} ExpectedSideEffect;

// =============================================================================
// CONSISTENCY CHECKING API
// =============================================================================

/**
 * Check consistency between L1 HRIR program and expected L0 side effects
 */
ConsistencyResult check_l1_l0_consistency(HRIR_Program* program,
                                        ExpectedSideEffect* expected_effects,
                                        size_t effect_count);

/**
 * Validate HRIR program internal consistency
 */
ConsistencyResult validate_hrir_consistency(HRIR_Program* program);

/**
 * Run comprehensive consistency suite
 */
int run_consistency_suite(HRIR_Program* program);

#endif // CONSISTENCY_CHECKER_H
