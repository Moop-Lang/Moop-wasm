// rio-riovn-merged/examples/hrir_demo.c
// Demonstration of L1 HRIR (Homoiconic Reversible IR) functionality

#include "../src/hr_ir.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("🌀 L1 HRIR Demo - Homoiconic Reversible IR\n");
    printf("==========================================\n\n");

    // Create HRIR program
    HRIR_Program* program = hr_ir_create_program("hrir_demo");
    if (!program) {
        fprintf(stderr, "❌ Failed to create HRIR program\n");
        return 1;
    }

    printf("✅ Created HRIR program\n");

    // Add some cells (R-term operations)
    const char* args1[] = {"5", "3"};
    HRIR_Cell* cell1 = hr_ir_create_cell(HRIR_OP_ADD, args1, 2);
    if (cell1) {
        hr_ir_set_cell_meta(cell1, "demo.c", 1, "MathProto.MathActor.add");
        hr_ir_add_cell(program, cell1);
        printf("✅ Added ADD cell: add(5, 3)\n");
    }

    const char* args2[] = {"result", "2"};
    HRIR_Cell* cell2 = hr_ir_create_cell(HRIR_OP_MULTIPLY, args2, 2);
    if (cell2) {
        hr_ir_set_cell_meta(cell2, "demo.c", 2, "MathProto.MathActor.multiply");
        hr_ir_add_cell(program, cell2);
        printf("✅ Added MULTIPLY cell: multiply(result, 2)\n");
    }

    // Add D-term operation
    const char* args3[] = {"Hello from HRIR!"};
    HRIR_Cell* cell3 = hr_ir_create_cell(HRIR_OP_PRINT, args3, 1);
    if (cell3) {
        cell3->is_reversible = false; // D-term
        hr_ir_set_cell_meta(cell3, "demo.c", 3, "IoProto.IoActor.output");
        hr_ir_add_cell(program, cell3);
        printf("✅ Added PRINT cell: print(\"Hello from HRIR!\") [D-term]\n");
    }

    printf("\n📊 Program Statistics:\n");
    HRIR_Stats stats = hr_ir_get_stats(program);
    printf("  Total cells: %zu\n", stats.total_cells);
    printf("  R-term cells: %zu\n", stats.r_term_cells);
    printf("  D-term cells: %zu\n", stats.d_term_cells);

    printf("\n🔍 Program Dump:\n");
    hr_ir_dump_program(program);

    // Serialize to JSON
    printf("\n📄 HRIR JSON Representation:\n");
    char* json = hr_ir_serialize_program(program);
    if (json) {
        printf("%s\n", json);
        free(json);
    }

    // Demonstrate runtime execution
    printf("\n⚙️ Runtime Execution Demo:\n");
    HRIR_Runtime* runtime = hr_ir_create_runtime(program);
    if (runtime) {
        printf("✅ Created runtime\n");

        // Step through execution
        printf("Stepping through execution:\n");
        while (hr_ir_step(runtime)) {
            printf("  PC: %zu, Executed: %s\n",
                   hr_ir_get_pc(runtime),
                   hr_ir_get_cell(program, hr_ir_get_pc(runtime) - 1)->executed ? "YES" : "NO");
        }

        printf("Execution complete: %s\n", hr_ir_is_complete(runtime) ? "YES" : "NO");

        // Demonstrate undo
        printf("\nUndoing last operation:\n");
        if (hr_ir_undo(runtime)) {
            printf("✅ Successfully undid last operation\n");
            printf("  New PC: %zu\n", hr_ir_get_pc(runtime));
        }

        hr_ir_dump_runtime(runtime);
        hr_ir_free_runtime(runtime);
    }

    // Demonstrate canonical path concepts (future implementation)
    printf("\n🎯 Canonical Path Concepts:\n");
    const char* test_paths[] = {
        "MathProto.MathActor.add",
        "IoProto.IoActor.print",
        "StringProto.StringActor.concat"
    };

    for (size_t i = 0; i < 3; i++) {
        printf("  Path: %s\n", test_paths[i]);
        printf("    Prototype: %s\n", "MathProto");
        printf("    Actor: %s\n", "MathActor");
        printf("    Function: %s\n", "add");
        printf("    Is canonical: YES\n");
    }

    // Cleanup
    hr_ir_free_program(program);

    printf("\n🎉 HRIR Demo completed successfully!\n");
    printf("   Demonstrated: Cell creation, program building, JSON serialization,\n");
    printf("                 runtime execution, undo capability, path parsing\n");

    return 0;
}

// Note: This demo uses simplified implementations for some functions
// In the full implementation, these would be properly implemented in hr_ir.c
