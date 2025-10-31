// rio-riovn-merged/examples/api_example.c
// Example of using the embeddable Rio+RioVN API

#include "../src/rio_api.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("🌀 Rio+RioVN Embeddable API Example\n");
    printf("===================================\n\n");

    // Create VM
    RioVM* vm = rio_create_vm();
    if (!vm) {
        fprintf(stderr, "❌ Failed to create VM\n");
        return 1;
    }

    rio_set_verbose(vm, true);
    printf("✅ VM created successfully\n");

    // Example source code
    const char* source =
        "// Example Rio+RioVN program\n"
        "MathProto <- ObjectProto\n"
        "CalculatorProto <- MathProto\n"
        "IoProto <- SystemProto\n"
        "\n"
        "math -> add 5 3\n"
        "calc -> multiply result 2\n"
        "@io io -> output \"Hello from embedded Rio!\"\n";

    // Compilation options
    RioCompileOptions options = rio_default_options();
    options.json_output = true;
    options.debug_mode = true;

    printf("\n📝 Compiling source code...\n");

    // Compile source
    RioResult* result = rio_compile_string(vm, source, options);
    if (!result) {
        fprintf(stderr, "❌ Compilation failed: %s\n", rio_get_last_error(vm));
        rio_destroy_vm(vm);
        return 1;
    }

    // Check result
    if (rio_result_success(result)) {
        printf("✅ Compilation successful!\n\n");

        // Access results
        printf("📊 Compilation Stats:\n");
        RioStats stats = rio_result_stats(result);
        printf("  Statements: %zu\n", rio_result_statement_count(result));
        printf("  Inheritance relations: %zu\n", rio_result_inheritance_count(result));
        printf("  Compilation time: %.2f ms\n", stats.compilation_time_ms);
        printf("\n");

        // Show inheritance relations
        printf("📋 Inheritance Relations:\n");
        size_t inheritance_count = rio_result_inheritance_count(result);
        for (size_t i = 0; i < inheritance_count; i++) {
            const char* relation = rio_result_inheritance_relation(result, i);
            if (relation) {
                printf("  - %s\n", relation);
            }
        }
        printf("\n");

        // Show JSON output
        const char* json = rio_result_json_output(result);
        if (json) {
            printf("📄 JSON Output:\n%s\n", json);
        }

        // Demonstrate AST access (homoiconicity)
        printf("🔍 AST Inspection (Homoiconicity Demo):\n");
        RioAST* ast = rio_result_get_ast(result);
        if (ast) {
            size_t node_count = rio_ast_node_count(ast);
            printf("  AST nodes: %zu\n", node_count);

            for (size_t i = 0; i < node_count && i < 3; i++) { // Show first 3 nodes
                RioASTNode node = rio_ast_get_node(ast, i);
                if (node.type == RIO_AST_SEND) {
                    printf("  Node %zu: SEND %s -> %s\n", i,
                           node.data.send.target, node.data.send.selector);
                } else if (node.type == RIO_AST_INHERIT) {
                    printf("  Node %zu: INHERIT %s <- %s\n", i,
                           node.data.inherit.child, node.data.inherit.parent);
                }
            }
        }
        printf("\n");

        // Demonstrate inheritance registry
        printf("🏛️ Inheritance Registry Demo:\n");
        RioInheritanceMap* inheritance_map = rio_result_get_inheritance_map(result);
        if (inheritance_map) {
            printf("  Checking inheritance relationships...\n");
            printf("  CalculatorProto inherits from MathProto: %s\n",
                   rio_inheritance_has_parent(inheritance_map, "CalculatorProto", "MathProto") ? "YES" : "NO");
            printf("  MathProto inherits from ObjectProto: %s\n",
                   rio_inheritance_has_parent(inheritance_map, "MathProto", "ObjectProto") ? "YES" : "NO");
        }
        printf("\n");

        // Demonstrate canonical path parsing
        printf("🎯 Canonical Path Demo:\n");
        const char* test_path = "MathProto.MathActor.add";
        RioCanonicalPath parsed = rio_parse_canonical_path(test_path);
        printf("  Path: %s\n", test_path);
        printf("  Prototype: %s\n", parsed.prototype ? parsed.prototype : "N/A");
        printf("  Actor: %s\n", parsed.actor ? parsed.actor : "N/A");
        printf("  Function: %s\n", parsed.function ? parsed.function : "N/A");
        printf("  Is canonical: %s\n", rio_is_canonical_path(test_path) ? "YES" : "NO");

    } else {
        fprintf(stderr, "❌ Compilation failed: %s\n", rio_result_error_message(result));
        fprintf(stderr, "Error code: %d\n", rio_result_error_code(result));
    }

    // Cleanup
    rio_free_result(result);
    rio_destroy_vm(vm);

    printf("✅ API example completed successfully!\n");
    printf("   Demonstrated: VM lifecycle, compilation, AST access, inheritance registry\n");

    return 0;
}
