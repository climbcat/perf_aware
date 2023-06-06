#include "lib.c"


// PLAN:
//
// - first, write a specific json parser
// - then write a generalized one later on
//
// ... how do we write a tokenizer again? I didn't write any for two years..

// NOTES:
// - filter out quotations around strings automatically (not a token), part of parse-string case
// - filter whitespaces automatically


struct Tokenizer {
    char *at;
};

enum TokenType {
    TOK_UNDEFINED,

    TOK_INT,
    TOK_DOUBLE,
    TOK_STRING,

    TOK_COMMA,
    TOK_COLON,
    TOK_LBRACK,
    TOK_RBRACK,
    TOK_LSQUAREBRACK,
    TOK_RSQUAREBRACK,
    TOK_LCURLBRACK,
    TOK_RCURLBRACK,

    TOK_EOF,

    TOK_COUNT,
};

struct Token {
    char *str = 0;
    u8 len = 0;
    TokenType type;
};

void PrintTokenType(TokenType tpe) {
    switch (tpe) {
    case TOK_UNDEFINED: printf("TOK_UNDEFINED"); break;
    case TOK_INT: printf("TOK_INT"); break;
    case TOK_DOUBLE: printf("TOK_DOUBLE"); break;
    case TOK_STRING: printf("TOK_STRING"); break;
    case TOK_COMMA: printf("TOK_COMMA"); break;
    case TOK_COLON: printf("TOK_COLON"); break;
    case TOK_LBRACK: printf("TOK_LBRACK"); break;
    case TOK_RBRACK: printf("TOK_RBRACK"); break;
    case TOK_LSQUAREBRACK: printf("TOK_LSQUAREBRACK"); break;
    case TOK_RSQUAREBRACK: printf("TOK_RSQUAREBRACK"); break;
    case TOK_LCURLBRACK: printf("TOK_LCURLBRACK"); break;
    case TOK_RCURLBRACK: printf("TOK_RCURLBRACK"); break;
    case TOK_EOF: printf("TOK_EOF"); break;
    default: printf("unknown token"); break;
    }
    printf("\n");
    // TODO: print strings and numbers
}

inline
bool IsEndOfFrame(char c) {
    return
        c == '\0';
}
inline
bool IsEndOfLine(char c) {
    return
        c == '\n' ||
        c == '\r';
}
inline
bool IsWhitespace(char c) {
    return
        c == ' ' ||
        c == '\t' ||
        c == '\v' ||
        c == '\f' ||
        IsEndOfLine(c);
}
inline
bool IsNumeric(char c) {
    bool isnum = (c >= '0') && (c <= '9');
    return isnum;
}
inline
bool IsAlphaOrUnderscore(char c) {
    bool result = 
        (c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
    return result;
}

void ParseString(Tokenizer *tokenizer) {
    // TODO: impl, use below. The culprit is "-" minus sign in front of negative numbers...
}

Token GetToken(Tokenizer *tokenizer) {

    // skip white spaces
    while (IsWhitespace(*tokenizer->at)) {
        ++tokenizer->at;
    }

    Token tok;
    tok.len = 1;
    tok.str = tokenizer->at;
    char c = *tokenizer->at;

    // int / float
    if (c == '-' || ((c >= '0') && (c <= '9'))) {
        tok.type = TOK_INT;
        tok.str = tokenizer->at;
        bool minus_char_found = false;
        bool dot_char_found = false;
        bool error = true;
        while (true) {
            c = *tokenizer->at;
            if ((c >= '0') && (c <= '9')) {
                // ...
            }
            else if (c == '-') {
                if (minus_char_found) {
                    error = true;
                }
                minus_char_found = true;
            }
            else if (c == '.') {
                if (dot_char_found) {
                    error = true;
                }
                dot_char_found = true;
                if (tok.len == 1) {
                    error = true;
                }
            }
            else {
                if (error) {
                    tok.type = TOK_UNDEFINED;
                }
                if (dot_char_found == true) {
                    tok.type = TOK_DOUBLE;
                }
                break;
            }
            ++tok.len;
            ++tokenizer->at;
        }
    }
    else {

        // strings
        switch (c) {
        case '"':

            ++tokenizer->at;
            tok.type = TOK_STRING;
            tok.str = tokenizer->at;
            while (true) {
                c = *tokenizer->at;
                if (IsEndOfFrame(c)) {
                    tok.type = TOK_EOF;
                    break;
                }
                else if (c == '"') {
                    break;
                }
                else {
                    ++tok.len;
                }
                ++tokenizer->at;
            }
            break;

        // semantic symbols
        case ',':
            tok.type = TOK_COMMA;
            break;

        case ':':
            tok.type = TOK_COLON;
            break;        

        case '(':
            tok.type = TOK_LBRACK;
            break;

        case ')':
            tok.type = TOK_RBRACK;
            break;

        case '[':
            tok.type = TOK_LSQUAREBRACK;
            break;

        case ']':
            tok.type = TOK_RSQUAREBRACK;
            break;

        case '{':
            tok.type = TOK_LCURLBRACK;
            break;

        case '}':
            tok.type = TOK_RCURLBRACK;
            break;
        
        case '\0':
            tok.type = TOK_EOF;
            return tok;
        
        default:
            tok.len = 0;
            tok.type = TOK_UNDEFINED;
            break;
        }
        ++tokenizer->at;
    }
    return tok;
}

void ParseHsPointsJson(char *filename) {
    u8* dest = (u8*) malloc(1024*1024);
    LoadFilePathBin(filename, dest);
    printf("parsing input:\n %s\n", dest);

    Tokenizer tokenizer;
    tokenizer.at = (char*) dest;
    Token tok;
    u16 i = 0;
    printf("parse result of <=100 tokens:\n");
    do {
        tok = GetToken(&tokenizer);
        PrintTokenType(tok.type);
        ++i;
    } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF && i < 100);
}


void Test() {
    printf("Running tests...\n");
}

int main (int argc, char **argv) {
    if (ContainsArg("--test", argc, argv)) {
        Test();
        exit(0);
    }
    if (argc != 2) {
        printf("hspoints json filename missing\n");
        exit(0);
    }
    char *filename = argv[1];
    printf("read filename: %s\n", filename);
    ParseHsPointsJson(filename);
}
