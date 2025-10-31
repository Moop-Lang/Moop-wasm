// Simple test for surface parser
#include "src/surface_parser.h"
#include <stdio.h>

// Forward declaration for simplified parser
SurfaceAST* parse_surface_simple(const char* source);

int main() {
    const char* test_code = "MathProto <- ObjectProto\nmath -> add 5 3\n";

    printf("Testing simplified surface parser...\n");
    SurfaceAST* ast = parse_surface_simple(test_code);

    if (ast) {
        printf("Parsed %zu statements\n", ast->statement_count);
        printf("Inheritance relations: %zu\n", ast->inheritance_count);
        free_surface_ast(ast);
        printf("Memory freed successfully\n");
    } else {
        printf("Parse failed\n");
    }

    return 0;
}
