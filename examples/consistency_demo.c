// rio-riovn-merged/examples/consistency_demo.c
// Consistency Checker Demo - Validates Dual-Memory System

#include "../src/hr_ir.h"
#include "../src/consistency_checker.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("🔍 Rio+RioVN Consistency Checker Demo\n");
    printf("=====================================\n\n");

    // Create test HRIR program
    HRIR_Program* program = hr_ir_create_program("consistency_test");
    if (!program) {
        fprintf(stderr, "❌ Failed to create HRIR program\n");
        return 1;
    }

    printf("✅ Created test HRIR program\n");

    // Add R-term operations
    const char* add_args[] = {"10", "5"};
    HRIR_Cell* add_cell = hr_ir_create_cell("add", add_args, 2);
    if (add_cell) {
        hr_ir_set_cell_meta(add_cell, "demo.c", 1, "MathProto.MathActor.add");
        hr_ir_add_cell(program, add_cell);
        printf("✅ Added R-term: add(10, 5)\n");
    }

    const char* mul_args[] = {"result", "2"};
    HRIR_Cell* mul_cell = hr_ir_create_cell("multiply", mul_args, 2);
    if (mul_cell) {
        hr_ir_set_cell_meta(mul_cell, "demo.c", 2, "MathProto.MathActor.multiply");
        hr_ir_add_cell(program, mul_cell);
        printf("✅ Added R-term: multiply(result, 2)\n");
    }

    // Add D-term operation
    const char* print_args[] = {"Calculation complete!"};
    HRIR_Cell* print_cell = hr_ir_create_cell("print", print_args, 1);
    if (print_cell) {
        print_cell->is_reversible = false; // D-term
        hr_ir_set_cell_meta(print_cell, "demo.c", 3, "IoProto.IoActor.output");
        hr_ir_add_cell(program, print_cell);
        printf("✅ Added D-term: print(\"Calculation complete!\")\n");
    }

    printf("\n📊 Program Statistics:\n");
    HRIR_Stats stats = hr_ir_get_stats(program);
    printf("  Total cells: %zu\n", stats.total_cells);
    printf("  R-term cells: %zu\n", stats.r_term_cells);
    printf("  D-term cells: %zu\n", stats.d_term_cells);

    // Expected side effects for D-term validation
    ExpectedSideEffect expected_effects[] = {
        {
            .operation = "print",
            .args = (const char*[]){"Calculation complete!"},
            .arg_count = 1,
            .should_succeed = true
        }
    };

    printf("\n🔍 Running L1→L0 Consistency Check:\n");
    ConsistencyResult consistency = check_l1_l0_consistency(
        program, expected_effects, 1
    );

    printf("\n🔍 Running HRIR Internal Consistency Validation:\n");
    ConsistencyResult internal = validate_hrir_consistency(program);

    printf("\n🧪 Running Comprehensive Consistency Suite:\n");
    int suite_result = run_consistency_suite(program);

    // Cleanup
    hr_ir_free_program(program);

    printf("\n📋 Summary:\n");
    printf("  L1→L0 Consistency: %s\n", consistency.is_consistent ? "✅ PASSED" : "❌ FAILED");
    printf("  HRIR Internal: %s\n", internal.is_consistent ? "✅ PASSED" : "❌ FAILED");
    printf("  Test Suite: %s\n", suite_result == 0 ? "✅ PASSED" : "❌ FAILED");

    if (consistency.is_consistent && internal.is_consistent && suite_result == 0) {
        printf("\n🎉 All consistency checks PASSED! Dual-memory system is sound.\n");
        printf("   • L1 reversible operations maintain information integrity\n");
        printf("   • D-term operations produce expected side effects\n");
        printf("   • HRIR program structure is internally consistent\n");
        printf("   • Cross-linking between reversible and irreversible domains works\n");
        return 0;
    } else {
        printf("\n⚠️  Some consistency checks FAILED - review implementation\n");
        return 1;
    }
}
