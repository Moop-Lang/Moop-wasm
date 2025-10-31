// rio-riovn-merged/src/surface_parser.c
// Surface Parser Implementation - Two Arrows with Optional Tags

#include "surface_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// =============================================================================
// LEXER IMPLEMENTATION
// =============================================================================

// Create lexer for source code
Lexer* create_lexer(const char* source) {
    Lexer* lexer = calloc(1, sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = strlen(source);
    lexer->current_pos = 0;
    lexer->current_line = 1;
    lexer->current_column = 1;
    return lexer;
}

// Free lexer resources
void free_lexer(Lexer* lexer) {
    free(lexer);
}

// Check if character is valid identifier start
static bool is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

// Check if character is valid identifier continuation
static bool is_identifier_continue(char c) {
    return isalnum(c) || c == '_';
}

// Advance lexer position
static void advance(Lexer* lexer) {
    lexer->current_pos++;
    lexer->current_column++;
}

// Peek at current character
static char peek(Lexer* lexer) {
    if (lexer->current_pos >= lexer->source_length) return '\0';
    return lexer->source[lexer->current_pos];
}

// Get next token from source
Token* next_token(Lexer* lexer) {
    // Skip whitespace and comments
    while (lexer->current_pos < lexer->source_length) {
        char c = peek(lexer);

        if (c == ' ' || c == '\t') {
            advance(lexer);
            continue;
        }

        if (c == '\n') {
            lexer->current_line++;
            lexer->current_column = 1;
            advance(lexer);
            continue;
        }

        if (c == '/' && lexer->current_pos + 1 < lexer->source_length &&
            lexer->source[lexer->current_pos + 1] == '/') {
            // Skip comment line
            while (lexer->current_pos < lexer->source_length && peek(lexer) != '\n') {
                advance(lexer);
            }
            continue;
        }

        break;
    }

    if (lexer->current_pos >= lexer->source_length) {
        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_EOF;
        token->lexeme = strdup("");
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        return token;
    }

    char c = peek(lexer);

    // Check for arrows
    if (c == '-' && lexer->current_pos + 1 < lexer->source_length &&
        lexer->source[lexer->current_pos + 1] == '>') {
        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_MESSAGE_ARROW;
        token->lexeme = strdup("->");
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token;
    }

    if (c == '<' && lexer->current_pos + 1 < lexer->source_length &&
        lexer->source[lexer->current_pos + 1] == '-') {
        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_INHERIT_ARROW;
        token->lexeme = strdup("<-");
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token;
    }

    // Check for tags
    if (c == '@') {
        size_t start = lexer->current_pos;
        advance(lexer); // consume '@'

        while (lexer->current_pos < lexer->source_length &&
               is_identifier_continue(peek(lexer))) {
            advance(lexer);
        }

        size_t length = lexer->current_pos - start;
        char* lexeme = malloc(length + 1);
        memcpy(lexeme, lexer->source + start, length);
        lexeme[length] = '\0';

        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_TAG;
        token->lexeme = lexeme;
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        return token;
    }

    // Check for string literals
    if (c == '"') {
        size_t start = lexer->current_pos;
        advance(lexer); // consume opening quote

        while (lexer->current_pos < lexer->source_length && peek(lexer) != '"') {
            if (peek(lexer) == '\\' && lexer->current_pos + 1 < lexer->source_length) {
                advance(lexer); // skip escape
            }
            advance(lexer);
        }

        if (lexer->current_pos < lexer->source_length) {
            advance(lexer); // consume closing quote
        }

        size_t length = lexer->current_pos - start;
        char* lexeme = malloc(length + 1);
        memcpy(lexeme, lexer->source + start, length);
        lexeme[length] = '\0';

        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_LITERAL;
        token->lexeme = lexeme;
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        return token;
    }

    // Check for identifiers/numbers
    if (is_identifier_start(c)) {
        size_t start = lexer->current_pos;
        advance(lexer);

        while (lexer->current_pos < lexer->source_length &&
               is_identifier_continue(peek(lexer))) {
            advance(lexer);
        }

        size_t length = lexer->current_pos - start;
        char* lexeme = malloc(length + 1);
        memcpy(lexeme, lexer->source + start, length);
        lexeme[length] = '\0';

        Token* token = calloc(1, sizeof(Token));
        token->type = TOKEN_IDENTIFIER;
        token->lexeme = lexeme;
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        return token;
    }

    // Unknown character
    Token* token = calloc(1, sizeof(Token));
    token->type = TOKEN_ERROR;
    token->lexeme = malloc(2);
    token->lexeme[0] = c;
    token->lexeme[1] = '\0';
    token->line = lexer->current_line;
    token->column = lexer->current_column;
    advance(lexer);
    return token;
}

// =============================================================================
// PARSER IMPLEMENTATION
// =============================================================================

// Parse source into surface AST (using simplified parser)
SurfaceAST* parse_surface(const char* source) {
    // Forward declaration for simplified parser
    SurfaceAST* parse_surface_simple(const char* source);
    return parse_surface_simple(source);
}

// Free surface AST
void free_surface_ast(SurfaceAST* ast) {
    if (!ast) return;

    for (size_t i = 0; i < ast->statement_count; i++) {
        Statement* stmt = &ast->statements[i];
        if (stmt->type == STMT_SEND) {
            if (stmt->as.send.target) free(stmt->as.send.target);
            if (stmt->as.send.selector) free(stmt->as.send.selector);
            if (stmt->as.send.arguments) {
                for (size_t j = 0; j < stmt->as.send.arg_count; j++) {
                    if (stmt->as.send.arguments[j]) free(stmt->as.send.arguments[j]);
                }
                free(stmt->as.send.arguments);
            }
            if (stmt->as.send.tag.tag_type) free(stmt->as.send.tag.tag_type);
        } else if (stmt->type == STMT_INHERIT) {
            if (stmt->as.inherit.child) free(stmt->as.inherit.child);
            if (stmt->as.inherit.parent) free(stmt->as.inherit.parent);
        }
    }

    if (ast->statements) free(ast->statements);

    if (ast->inheritance_relations) {
        for (size_t i = 0; i < ast->inheritance_count; i++) {
            if (ast->inheritance_relations[i]) free(ast->inheritance_relations[i]);
        }
        free(ast->inheritance_relations);
    }

    free(ast);
}

// =============================================================================
// OPERATION CLASSIFICATION
// =============================================================================

// Classify operation type (R/D/S)
OperationType classify_operation(const char* target, const char* selector,
                               OperationTag tag) {
    // If explicitly tagged, it's D-term
    if (tag.is_tagged) {
        return OP_D_TERM;
    }

    // Check if it's a known D-term operation
    if (is_known_d_term_operation(target, selector)) {
        return OP_D_TERM;
    }

    // Default to R-term (reversible)
    return OP_R_TERM;
}

// Check if identifier is known D-term operation
bool is_known_d_term_operation(const char* target, const char* selector) {
    // Known I/O operations
    if (strcmp(target, "io") == 0 || strcmp(target, "file") == 0 ||
        strcmp(target, "network") == 0 || strcmp(target, "system") == 0) {
        return true;
    }

    // Known coordination operations that might be irreversible
    if (strcmp(selector, "fork") == 0 || strcmp(selector, "spawn") == 0 ||
        strcmp(selector, "kill") == 0 || strcmp(selector, "exit") == 0) {
        return true;
    }

    return false;
}

// Extract tag from token stream (simplified)
OperationTag parse_operation_tag(Lexer* lexer) {
    (void)lexer; // Suppress unused parameter warning
    OperationTag tag = {false, NULL};
    // In this simplified version, tags are handled by source code ordering
    return tag;
}

// Free token resources
void free_token(Token* token) {
    if (!token) return;
    if (token->lexeme) free(token->lexeme);
                    free_token(token);
}

// PascalCase utility
char* to_pascal_case(const char* input) {
    if (!input || strlen(input) == 0) {
        char* empty = strdup("");
        return empty ? empty : NULL;
    }

    char* result = strdup(input);
    if (!result) return NULL;

    if (result[0] >= 'a' && result[0] <= 'z') {
        result[0] = result[0] - 'a' + 'A';
    }

    return result;
}
