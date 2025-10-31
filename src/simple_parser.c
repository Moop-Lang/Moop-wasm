// rio-riovn-merged/src/simple_parser.c
// Simplified surface parser to avoid memory issues

#include "surface_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Simplified token structure
typedef struct {
    TokenType type;
    char lexeme[256];
} SimpleToken;

// Simple lexer
SimpleToken* simple_next_token(const char** source) {
    while (**source) {
        char c = **source;

        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\n') {
            (*source)++;
            continue;
        }

        // Skip comments
        if (c == '/' && *(*source + 1) == '/') {
            while (**source && **source != '\n') (*source)++;
            continue;
        }

        // Message arrow
        if (c == '-' && *(*source + 1) == '>') {
            SimpleToken* token = calloc(1, sizeof(SimpleToken));
            token->type = TOKEN_MESSAGE_ARROW;
            strcpy(token->lexeme, "->");
            *source += 2;
            return token;
        }

        // Inherit arrow
        if (c == '<' && *(*source + 1) == '-') {
            SimpleToken* token = calloc(1, sizeof(SimpleToken));
            token->type = TOKEN_INHERIT_ARROW;
            strcpy(token->lexeme, "<-");
            *source += 2;
            return token;
        }

        // Identifier
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            SimpleToken* token = calloc(1, sizeof(SimpleToken));
            token->type = TOKEN_IDENTIFIER;
            int i = 0;
            while ((**source >= 'a' && **source <= 'z') ||
                   (**source >= 'A' && **source <= 'Z') ||
                   (**source >= '0' && **source <= '9') ||
                   **source == '_') {
                token->lexeme[i++] = **source;
                (*source)++;
            }
            token->lexeme[i] = '\0';
            return token;
        }

        // Unknown - skip
        (*source)++;
    }

    // EOF
    SimpleToken* token = calloc(1, sizeof(SimpleToken));
    token->type = TOKEN_EOF;
    strcpy(token->lexeme, "");
    return token;
}

// Simplified surface parser
SurfaceAST* parse_surface_simple(const char* source) {
    SurfaceAST* ast = calloc(1, sizeof(SurfaceAST));
    const char* current = source;

    while (1) {
        SimpleToken* token = simple_next_token(&current);

        if (token->type == TOKEN_EOF) {
            free(token);
            break;
        }

        if (token->type == TOKEN_IDENTIFIER) {
            char* identifier = strdup(token->lexeme);
            free(token);

            token = simple_next_token(&current);

            if (token->type == TOKEN_MESSAGE_ARROW) {
                free(token);
                token = simple_next_token(&current);

                if (token->type == TOKEN_IDENTIFIER) {
                    // Add send statement
                    Statement stmt;
                    stmt.type = STMT_SEND;
                    stmt.as.send.target = strdup(identifier);
                    stmt.as.send.selector = strdup(token->lexeme);
                    stmt.as.send.arguments = NULL;
                    stmt.as.send.arg_count = 0;
                    stmt.as.send.tag.is_tagged = false;
                    stmt.as.send.tag.tag_type = NULL;
                    stmt.as.send.type = OP_R_TERM; // Default

                    ast->statements = realloc(ast->statements,
                        sizeof(Statement) * (ast->statement_count + 1));
                    ast->statements[ast->statement_count] = stmt;
                    ast->statement_count++;
                }
                free(token);

            } else if (token->type == TOKEN_INHERIT_ARROW) {
                free(token);
                token = simple_next_token(&current);

                if (token->type == TOKEN_IDENTIFIER) {
                    // Add inheritance relation
                    ast->inheritance_relations = realloc(ast->inheritance_relations,
                        sizeof(char*) * (ast->inheritance_count + 1));
                    char* relation = malloc(strlen(identifier) + strlen(token->lexeme) + 4);
                    sprintf(relation, "%s <- %s", identifier, token->lexeme);
                    ast->inheritance_relations[ast->inheritance_count] = relation;
                    ast->inheritance_count++;

                    // Also add the inheritance statement
                    Statement stmt;
                    stmt.type = STMT_INHERIT;
                    stmt.as.inherit.child = strdup(identifier);
                    stmt.as.inherit.parent = strdup(token->lexeme);

                    ast->statements = realloc(ast->statements,
                        sizeof(Statement) * (ast->statement_count + 1));
                    ast->statements[ast->statement_count] = stmt;
                    ast->statement_count++;
                }
                free(token);
            }

            free(identifier);
        } else {
            free(token);
        }
    }

    return ast;
}
