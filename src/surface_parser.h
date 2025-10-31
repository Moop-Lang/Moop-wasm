// rio-riovn-merged/src/surface_parser.h
// Unified Surface Parser - Two Arrows with Optional Tags

#ifndef SURFACE_PARSER_H
#define SURFACE_PARSER_H

#include "architecture.h"

// =============================================================================
// SURFACE SYNTAX DEFINITION (Minimal)
// =============================================================================

// Token types for the minimal surface language
typedef enum {
    TOKEN_MESSAGE_ARROW,     // ->
    TOKEN_INHERIT_ARROW,     // <-
    TOKEN_IDENTIFIER,        // names like MathProto, add, etc.
    TOKEN_TAG,              // @irreversible, @io
    TOKEN_LITERAL,          // "strings", numbers
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

// Lexer state
typedef struct {
    const char* source;
    size_t source_length;
    size_t current_pos;
    int current_line;
    int current_column;
} Lexer;

// =============================================================================
// PARSER AST (Unified for L5/L4)
// =============================================================================

// Operation types (UME classification)
typedef enum {
    OP_R_TERM,     // Reversible compute (default)
    OP_D_TERM,     // Irreversible coordination/IO (explicit)
    OP_S_TERM      // Structure/Inheritance
} OperationType;

// Tagged operation (for D-term boundaries)
typedef struct {
    bool is_tagged;
    char* tag_type;  // "irreversible" or "io"
} OperationTag;

// Send operation (message passing)
typedef struct {
    char* target;           // receiver (e.g., "math")
    char* selector;         // method name (e.g., "add")
    char** arguments;       // argument list
    size_t arg_count;
    OperationTag tag;       // optional D-term tag
    OperationType type;     // R/D classification
} SendOperation;

// Inheritance declaration
typedef struct {
    char* child;            // child prototype
    char* parent;           // parent prototype
} InheritanceDecl;

// Surface statement types
typedef enum {
    STMT_SEND,              // message send
    STMT_INHERIT,           // inheritance declaration
    STMT_COMMENT            // comment (ignored)
} StatementType;

// Unified surface statement
typedef struct {
    StatementType type;
    union {
        SendOperation send;
        InheritanceDecl inherit;
    } as;
} Statement;

// Surface AST (L5/L4 interface)
struct SurfaceAST {
    Statement* statements;
    size_t statement_count;
    char** inheritance_relations;
    size_t inheritance_count;
};
typedef struct SurfaceAST SurfaceAST;

// =============================================================================
// PARSER API (Minimal Interface)
// =============================================================================

// Create lexer for source code
Lexer* create_lexer(const char* source);

// Free lexer resources
void free_lexer(Lexer* lexer);

// Get next token from source
Token* next_token(Lexer* lexer);

// Parse source into surface AST
SurfaceAST* parse_surface(const char* source);

// Free surface AST
void free_surface_ast(SurfaceAST* ast);

// Classify operation type (R/D/S)
OperationType classify_operation(const char* target, const char* selector,
                               OperationTag tag);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Check if identifier is known D-term operation
bool is_known_d_term_operation(const char* target, const char* selector);

// Extract tag from token stream
OperationTag parse_operation_tag(Lexer* lexer);

// Free token resources
void free_token(Token* token);

// PascalCase utility (from RioVN)
char* to_pascal_case(const char* input);

#endif // SURFACE_PARSER_H
