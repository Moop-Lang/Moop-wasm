// rio-riovn-merged/src/consistency_checker.c
// Consistency Checker - Validates Dual-Memory System Integrity
//
// This module verifies that L1 tape replay reproduces the expected L0 side effects,
// ensuring the dual-memory system maintains consistency between reversible and
// irreversible operations.

#include "hr_ir.h"
#include "architecture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// CONSISTENCY CHECKER
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
                                        size_t effect_count) {
    ConsistencyResult result = {
        .is_consistent = true,
        .error_message = NULL,
        .operations_checked = 0,
        .side_effects_verified = 0
    };

    if (!program || !expected_effects) {
        result.is_consistent = false;
        result.error_message = "Invalid parameters";
        return result;
    }

    // Create runtime for simulation
    HRIR_Runtime* runtime = hr_ir_create_runtime(program);
    if (!runtime) {
        result.is_consistent = false;
        result.error_message = "Failed to create runtime";
        return result;
    }

    printf("🔍 Checking L1→L0 consistency for %zu operations\n", program->cell_count);

    size_t effect_index = 0;

    // Execute program and check side effects
    for (size_t i = 0; i < program->cell_count; i++) {
        result.operations_checked++;

        HRIR_Cell* cell = hr_ir_get_cell(program, i);
        if (!cell) continue;

        // Simulate execution
        bool step_result = hr_ir_step(runtime);

        if (cell->is_reversible) {
            // R-term: Should always succeed and be undoable
            if (!step_result) {
                result.is_consistent = false;
                result.error_message = "R-term operation failed";
                break;
            }

            // Test undo capability
            if (!hr_ir_undo(runtime)) {
                result.is_consistent = false;
                result.error_message = "R-term undo failed";
                break;
            }

            // Redo to continue
            if (!hr_ir_step(runtime)) {
                result.is_consistent = false;
                result.error_message = "R-term redo failed";
                break;
            }

            printf("  ✅ R-term: %s - reversible\n", cell->opcode);

        } else {
            // D-term: Check against expected side effects
            if (effect_index < effect_count) {
                ExpectedSideEffect* expected = &expected_effects[effect_index];

                // Compare operation details
                bool operation_matches = strcmp(cell->opcode, expected->operation) == 0;

                if (operation_matches) {
                    result.side_effects_verified++;
                    printf("  ✅ D-term: %s - side effect verified\n", cell->opcode);

                    // Check if success matches expectation
                    if (step_result != expected->should_succeed) {
                        result.is_consistent = false;
                        result.error_message = "D-term side effect mismatch";
                        break;
                    }
                } else {
                    printf("  ⚠️  D-term: %s - no matching expected effect\n", cell->opcode);
                }

                effect_index++;
            } else {
                printf("  ⚠️  D-term: %s - unexpected side effect\n", cell->opcode);
            }
        }
    }

    // Verify final state
    if (result.is_consistent) {
        if (!hr_ir_is_complete(runtime)) {
            result.is_consistent = false;
            result.error_message = "Program did not complete";
        }
    }

    hr_ir_free_runtime(runtime);

    printf("📊 Consistency check: %s\n", result.is_consistent ? "PASSED" : "FAILED");
    printf("   Operations checked: %zu\n", result.operations_checked);
    printf("   Side effects verified: %zu\n", result.side_effects_verified);

    if (!result.is_consistent && result.error_message) {
        printf("   Error: %s\n", result.error_message);
    }

    return result;
}

/**
 * Validate HRIR program internal consistency
 */
ConsistencyResult validate_hrir_consistency(HRIR_Program* program) {
    ConsistencyResult result = {
        .is_consistent = true,
        .error_message = NULL,
        .operations_checked = 0,
        .side_effects_verified = 0
    };

    if (!program) {
        result.is_consistent = false;
        result.error_message = "Invalid program";
        return result;
    }

    printf("🔍 Validating HRIR internal consistency\n");

    // Check cell integrity
    for (size_t i = 0; i < program->cell_count; i++) {
        result.operations_checked++;

        HRIR_Cell* cell = hr_ir_get_cell(program, i);
        if (!cell) {
            result.is_consistent = false;
            result.error_message = "Invalid cell";
            break;
        }

        // Check required fields
        if (!cell->opcode || !cell->args) {
            result.is_consistent = false;
            result.error_message = "Missing cell data";
            break;
        }

        // Check inverse consistency
        if (cell->is_reversible && !cell->inverse) {
            result.is_consistent = false;
            result.error_message = "Reversible cell missing inverse";
            break;
        }

        // Check ID uniqueness
        for (size_t j = 0; j < program->cell_count; j++) {
            if (i != j) {
                HRIR_Cell* other = hr_ir_get_cell(program, j);
                if (other && cell->id == other->id) {
                    result.is_consistent = false;
                    result.error_message = "Duplicate cell ID";
                    break;
                }
            }
        }

        if (!result.is_consistent) break;
    }

    // Validate program statistics
    if (result.is_consistent) {
        HRIR_Stats stats = hr_ir_get_stats(program);

        if (stats.total_cells != program->cell_count) {
            result.is_consistent = false;
            result.error_message = "Statistics mismatch";
        }
    }

    printf("📊 HRIR validation: %s\n", result.is_consistent ? "PASSED" : "FAILED");
    return result;
}

/**
 * Run comprehensive consistency suite
 */
int run_consistency_suite(HRIR_Program* program) {
    printf("🧪 Running Comprehensive Consistency Suite\n");
    printf("==========================================\n");

    int passed = 0;
    int total = 0;

    // Test 1: HRIR internal consistency
    total++;
    ConsistencyResult internal = validate_hrir_consistency(program);
    if (internal.is_consistent) {
        passed++;
        printf("✅ Test 1 PASSED: HRIR internal consistency\n");
    } else {
        printf("❌ Test 1 FAILED: %s\n", internal.error_message);
    }

    // Test 2: Empty program (edge case)
    total++;
    HRIR_Program* empty_program = hr_ir_create_program("empty_test");
    ConsistencyResult empty_result = validate_hrir_consistency(empty_program);
    if (empty_result.is_consistent) {
        passed++;
        printf("✅ Test 2 PASSED: Empty program consistency\n");
    } else {
        printf("❌ Test 2 FAILED: %s\n", empty_result.error_message);
    }
    hr_ir_free_program(empty_program);

    // Test 3: R-term operations only
    total++;
    HRIR_Program* r_term_program = hr_ir_create_program("r_term_test");
    HRIR_Cell* add_cell = hr_ir_create_cell("add", (const char*[]){"5", "3"}, 2);
    HRIR_Cell* mul_cell = hr_ir_create_cell("multiply", (const char*[]){"result", "2"}, 2);
    hr_ir_add_cell(r_term_program, add_cell);
    hr_ir_add_cell(r_term_program, mul_cell);

    ConsistencyResult r_term_result = validate_hrir_consistency(r_term_program);
    if (r_term_result.is_consistent) {
        passed++;
        printf("✅ Test 3 PASSED: R-term program consistency\n");
    } else {
        printf("❌ Test 3 FAILED: %s\n", r_term_result.error_message);
    }
    hr_ir_free_program(r_term_program);

    printf("\n📊 Suite Results: %d/%d tests passed\n", passed, total);

    if (passed == total) {
        printf("🎉 All consistency tests PASSED!\n");
        return 0;
    } else {
        printf("⚠️  Some tests FAILED - check implementation\n");
        return 1;
    }
}
