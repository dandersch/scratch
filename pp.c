#!./ext/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

/* helper macros */
#define IS_END_OF_LINE(character)               ((character == '\n') || (character == '\r'))
#define IS_WHITESPACE(character)                ((character == ' ')  || (character == '\t') || IS_END_OF_LINE(character))
#define IS_ALPHA(character)                     (((character >= 'a') && (character <= 'z')) || ((character >= 'A') && (character <= 'Z')))
#define IS_NUMERIC(character)                   ((character >= '0') &&  (character <= '9'))
#define IS_VALID_IDENTIFIER(character)          (IS_ALPHA(character)   || IS_NUMERIC(character) || (character == '_'))
#define IS_VALID_START_OF_IDENTIFIER(character) (IS_ALPHA(character) || (character == '_'))

const char* markers[] = { "INTROSPECT" };

/* TOKENIZER */
typedef enum {
    TOKEN_TYPE_INVALID,
    // NOTE: missing &&, &, |, <<, >>, ...
    TOKEN_TYPE_OPEN_PAREN,
    TOKEN_TYPE_CLOSE_PAREN,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_SEMICOLON,
    TOKEN_TYPE_ASTERISK,
    TOKEN_TYPE_OPEN_BRACKET,
    TOKEN_TYPE_CLOSE_BRACKET,
    TOKEN_TYPE_OPEN_BRACE,
    TOKEN_TYPE_CLOSE_BRACE,
    TOKEN_TYPE_EQUALS,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_OR,
    TOKEN_TYPE_POUND,

    TOKEN_TYPE_STRING,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_NUMBER,

    TOKEN_TYPE_WHITESPACE,
    TOKEN_TYPE_NEWLINE,
    TOKEN_TYPE_COMMENT,

    TOKEN_TYPE_EOF,

    TOKEN_TYPE_COUNT,
} token_type_e;
typedef struct {
    token_type_e type;
    size_t       text_len;
    char*        text;
} token_t;
typedef struct {
    char* at;
} tokenizer_t;
int token_equals(token_t token, const char* string) {
    for (int idx = 0; idx < token.text_len; idx++)
    {
        if ((string[idx] != token.text[idx]) || string[idx] == '\0') { return 0; }
    }

    int ended_at_same_time = (string[token.text_len] == '\0');
    return ended_at_same_time;
}
token_t get_token(tokenizer_t* tokenizer) {
    /* skip whitespace and comments */
    while (1)
    {
        if (IS_WHITESPACE(tokenizer->at[0])) { ++tokenizer->at; }

        else if (tokenizer->at[0] == '/' && tokenizer->at[1] == '*')
        {
            /* c style comment */
            ++tokenizer->at;
            ++tokenizer->at;
            while (!((tokenizer->at[0] == '*') && (tokenizer->at[1] == '/')))
            {
                ++tokenizer->at;
            }
            ++tokenizer->at;
            ++tokenizer->at;
        }

        else if (tokenizer->at[0] == '/' && tokenizer->at[1] == '/')
        {
            // cpp style comment
            ++tokenizer->at;
            ++tokenizer->at;
            while (!IS_END_OF_LINE(tokenizer->at[0])) { ++tokenizer->at; }
            ++tokenizer->at;
        }

        else { break; };
    }

    token_t token = { TOKEN_TYPE_INVALID, 1, tokenizer->at };

    switch (tokenizer->at[0])
    {
        case '\0': { token.type = TOKEN_TYPE_EOF;           } break;
        case '(':  { token.type = TOKEN_TYPE_OPEN_PAREN;    } break;
        case ')':  { token.type = TOKEN_TYPE_CLOSE_PAREN;   } break;
        case ':':  { token.type = TOKEN_TYPE_COLON;         } break;
        case ';':  { token.type = TOKEN_TYPE_SEMICOLON;     } break;
        case '*':  { token.type = TOKEN_TYPE_ASTERISK;      } break;
        case '[':  { token.type = TOKEN_TYPE_OPEN_BRACKET;  } break;
        case ']':  { token.type = TOKEN_TYPE_CLOSE_BRACKET; } break;
        case '{':  { token.type = TOKEN_TYPE_OPEN_BRACE;    } break;
        case '}':  { token.type = TOKEN_TYPE_CLOSE_BRACE;   } break;
        case '#':  { token.type = TOKEN_TYPE_POUND;         } break;

        // TODO
        case '=':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '?':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '<':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '>':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '!':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '\\': { token.type = TOKEN_TYPE_INVALID;       } break;
        case ',':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '.':  { token.type = TOKEN_TYPE_INVALID;       } break;
        case '/':  { token.type = TOKEN_TYPE_INVALID;       } break;

        default:
        {
            /* tokenize string */
            if (tokenizer->at[0] == '"' && tokenizer->at[1])
            {
                if (tokenizer->at[1] == '"') // empty string
                {
                    token.type     = TOKEN_TYPE_STRING;
                    token.text_len = 0;
                    token.text     = 0;
                    ++tokenizer->at;
                    break;
                }

                char* string_start = &tokenizer->at[1];
                token.type     = TOKEN_TYPE_STRING;
                token.text     = string_start;
                while (tokenizer->at[1] != '"' && tokenizer->at[1])
                {
                    ++tokenizer->at;
                    if (tokenizer->at[0] == '\\') { ++tokenizer->at; } // ignore escaped quotes
                }
                char* string_end = ++tokenizer->at;
                token.text_len   = string_end - string_start;
            }
            else
            {
                //assert(IS_VALID_START_OF_IDENTIFIER(tokenizer->at[0]) || IS_NUMERIC(tokenizer->at[0])); // TODO fails...

                /* tokenize identifier */
                if (IS_VALID_START_OF_IDENTIFIER(tokenizer->at[0]))
                {
                    token.type        = TOKEN_TYPE_IDENTIFIER;
                    char* ident_start = tokenizer->at;
                    token.text        = ident_start;

                    while (IS_VALID_IDENTIFIER(tokenizer->at[1]) && tokenizer->at[1])
                    {
                        ++tokenizer->at;
                    }
                    char* ident_end  = &tokenizer->at[1];
                    token.text_len   = ident_end - ident_start;
                }
                /* tokenize number */
                else if (IS_NUMERIC(tokenizer->at[0]))
                {
                    token.type      = TOKEN_TYPE_NUMBER;
                    char* num_start = tokenizer->at;
                    token.text      = num_start;
                    // TODO parse number
                }
            }
        } break;
    }

    tokenizer->at = &tokenizer->at[1];

    return token;
}
token_t peek_token(tokenizer_t* tokenizer) {
    char* pos_before = tokenizer->at;
    token_t peeked   = get_token(tokenizer);
    tokenizer->at    = pos_before;
    return peeked;
}
int require_token(tokenizer_t* tokenizer, token_type_e required_type) {
    token_t token = get_token(tokenizer);
    return token.type == required_type;
}

/* INTROSPECTION */
#define INTROSPECT_MARKERS_MAX_COUNT 4096
char* introspect_markers[INTROSPECT_MARKERS_MAX_COUNT];
int introspect_markers_count = 0;
void introspect(tokenizer_t* tokenizer)
{
    /* TODO parse introspection parameters */
    if (require_token(tokenizer, TOKEN_TYPE_OPEN_PAREN))
    {
        int params = 0;

        printf("#define ");
        const char* struct_name = NULL;
        size_t struct_name_len  = 0;
        while (1)
        {
            token_t token = get_token(tokenizer);
            if (token.type == TOKEN_TYPE_CLOSE_PAREN) { break; }

            if (token.type == TOKEN_TYPE_OPEN_BRACE)
            {
                while (token.type != TOKEN_TYPE_CLOSE_BRACE)
                {
                    /* printf("  X("); */
                    /* token = get_token(tokenizer); */
                    /* if (require_token(tokenizer, TOKEN_TYPE_IDENTIFIER)) */
                    /* { */
                    /*     printf("%.*s, ", token.text_len, token.text); */
                    /* } */
                    /* else { fprintf(stderr, "Expected identifier\n"); } */

                    /* if (peek_token(tokenizer).type == TOKEN_TYPE_ASTERISK) */
                    /* { */
                    /*     token = get_token(tokenizer); */
                    /*     printf("*, "); */
                    /* } */
                    /* else { printf(" , "); } */

                    /* if (require_token(tokenizer, TOKEN_TYPE_IDENTIFIER)) */
                    /* { */
                    /*     printf("%.*s, ", token.text_len, token.text); */
                    /* } */
                    /* else { fprintf(stderr, "Expected identifier\n"); } */

                    /* if (peek_token(tokenizer).type == TOKEN_TYPE_OPEN_BRACKET) */
                    /* { */
                    /*     token = get_token(tokenizer); */
                    /*     if (require_token(tokenizer, TOKEN_TYPE_NUMBER)) */
                    /*     { */
                    /*         printf("[%.*s], ", token.text_len, token.text); */
                    /*     } */
                    /*     if (!require_token(tokenizer, TOKEN_TYPE_OPEN_BRACKET)) { fprintf(stderr, "Expected closing bracket\n"); } */
                    /* } */
                    /* else { printf(" , "); } */
                    /* printf("__VA_ARGS__) "); */
                    /* if (peek_token(tokenizer).type != TOKEN_TYPE_CLOSE_BRACE) { printf("\\\n"); } */
                    /* else { printf("\n"); } */
                    /* token = get_token(tokenizer); */

                    printf("  X(");
                    while (token.type != TOKEN_TYPE_SEMICOLON)
                    {
                        if (token.type == TOKEN_TYPE_IDENTIFIER) {
                            printf("%.*s, ", token.text_len, token.text);
                        }
                        token = get_token(tokenizer);
                    }
                    printf("__VA_ARGS__) ");
                    if (peek_token(tokenizer).type != TOKEN_TYPE_CLOSE_BRACE) { printf("\\\n"); }
                    else { printf("\n"); }
                    token = get_token(tokenizer);
                }
                continue;
            }

            if (token.type == TOKEN_TYPE_IDENTIFIER)
            {
                if (token_equals(token, "typedef")) { continue; } // TODO handle "typedef struct {} name;"
                if (token_equals(token, "struct"))  { continue; }
                else
                {
                    if (peek_token(tokenizer).type == TOKEN_TYPE_SEMICOLON) { printf(""); }
                    else
                    {
                        struct_name     = token.text;
                        struct_name_len = token.text_len;
                        printf("%.*s(X, ...) \\\n", struct_name_len, struct_name);
                    }
                }
            }
        }

        printf("STRUCT(%.*s);\n", struct_name_len, struct_name);
        //printf("META(%.*s)\n\n", struct_name_len, struct_name);
    }
    else
    {
        fprintf(stderr, "ERROR: Missing open parenthesis for INTROSPECT marker.\n"); return;
    }
}

int main(int argc, char** argv)
{
    char* buffer = NULL;

    for (int i = 1; i < argc; i++)
    {
        size_t file_size;

        /* read in entire file null terminated */
        {
            FILE* fd = fopen(argv[i], "r");
            if (!fd) { return 1; }

            fseek(fd, 0, SEEK_END);
            file_size = ftell(fd);
            fseek(fd, 0, SEEK_SET);

            buffer = (char*) malloc(file_size + 1);
            fread(buffer, sizeof(*buffer), file_size, fd);
            buffer[file_size] = '\0';

            fclose(fd);
        }

        /* tokenize */
        tokenizer_t tokenizer = {buffer};
        int parsing = 1;
        while (parsing)
        {
            token_t token = get_token(&tokenizer);

            switch (token.type)
            {
                case TOKEN_TYPE_EOF:     { parsing = 0; } break;
                case TOKEN_TYPE_INVALID: { } break;

                case TOKEN_TYPE_POUND:
                {
                    token_t token = get_token(&tokenizer);
                    if (token_equals(token, "define"))
                    {
                        if (token_equals(peek_token(&tokenizer), "INTROSPECT"))
                        {
                            get_token(&tokenizer); // ignore INTROSPECT macro definition
                        }
                    }
                } break;

                case TOKEN_TYPE_IDENTIFIER:
                {
                    if (token_equals(token, "typedef"))
                    {
                        token_t token = get_token(&tokenizer);
                        if (token_equals(token, "struct"))
                        {
                            token_t token = get_token(&tokenizer);
                        }
                    }

                    if (token_equals(token, "struct"))
                    {
                        token_t token = get_token(&tokenizer);
                    }

                    if (token_equals(token, "INTROSPECT"))
                    {
                        /* record location of introspect marker for later */
                        introspect_markers[introspect_markers_count++] = tokenizer.at;
                        //introspect(&tokenizer);
                    }
                } break;

                default: { } break;
            }
        }

        /* perform introspection at markers */
        for (int i = 0; i < introspect_markers_count; i++)
        {
            char* curr_marker = introspect_markers[i];
            tokenizer_t tokenizer = {curr_marker};
            introspect(&tokenizer);
        }
        introspect_markers_count = 0;

        free(buffer);
    }

    return 0;
}
