#include "lib.c"


static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 Deg2Rad(f64 Degrees)
{
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = Deg2Rad(lat2 - lat1);
    f64 dLon = Deg2Rad(lon2 - lon1);
    lat1 = Deg2Rad(lat1);
    lat2 = Deg2Rad(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}


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

void PrintToken(Token tok) {
    switch (tok.type) {
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
    printf(":    %.*s\n", tok.len, tok.str);
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
    tok.len = 0;
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
                ++tok.len;
            }
            else if (c == '-') {
                if (minus_char_found) {
                    error = true;
                }
                minus_char_found = true;
                ++tok.len;
            }
            else if (c == '.') {
                if (dot_char_found) {
                    error = true;
                }
                dot_char_found = true;
                if (tok.len == 1) {
                    error = true;
                }
                ++tok.len;
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
            tok.len = 1;
            break;

        case ':':
            tok.type = TOK_COLON;
            tok.len = 1;
            break;        

        case '(':
            tok.type = TOK_LBRACK;
            tok.len = 1;
            break;

        case ')':
            tok.type = TOK_RBRACK;
            tok.len = 1;
            break;

        case '[':
            tok.type = TOK_LSQUAREBRACK;
            tok.len = 1;
            break;

        case ']':
            tok.type = TOK_RSQUAREBRACK;
            tok.len = 1;
            break;

        case '{':
            tok.type = TOK_LCURLBRACK;
            tok.len = 1;
            break;

        case '}':
            tok.type = TOK_RCURLBRACK;
            tok.len = 1;
            break;
        
        case '\0':
            tok.type = TOK_EOF;
            tok.len = 1;
            return tok;
        
        default:
            tok.type = TOK_UNDEFINED;
            tok.len = 0;
            break;
        }
        ++tokenizer->at;
    }
    return tok;
}


f64 ParseDouble(char *str, u8 len) {
    f64 val = 0;
    f64 multiplier = 1;

    // handle sign
    bool sgned = str[0] == '-';
    if (sgned) {
        ++str;
    }

    u8 decs_denom = 0;
    while ((str[decs_denom] != '.') && (decs_denom < len)) {
        ++decs_denom;
    }
    // assert(digs_denom < len - 1 && "ParseDouble invalid string");

    // decimals before dot
    for (int i = 0; i < decs_denom; ++i) {
        char ascii = str[decs_denom - 1 - i];
        u8 decimal = ascii - 48;
        val += decimal * multiplier;
        multiplier *= 10;
    }

    // decimals after dot
    multiplier = 0.1f;
    u8 decs_nom = len - 1 - decs_denom;
    for (int i = decs_denom + 1; i < len; ++i) {
        char ascii = str[i];
        u8 decimal = ascii - 48;
        val += decimal * multiplier;
        multiplier *= 0.1;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
}



void ParseHsPointsJson(char *filename) {
    char* dest_json = (char*) malloc(10 * MEGABYTE);
    Tokenizer tokenizer;
    tokenizer.at = dest_json;
    LoadFilePathBin(filename, (u8*) dest_json);
    printf("loaded file %s:\n%s\n", filename, dest_json);

    // parse floats and put into data storage
    f64* floc = (f64*) malloc(MEGABYTE);
    u32 fidx = 0;
    Token tok;
    do {
        tok = GetToken(&tokenizer);
        if (tok.type == TOK_DOUBLE) {
            floc[fidx] = ParseDouble(tok.str, tok.len);
            ++fidx;
        }
    } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF);

    u32 npairs = (fidx + 1) / 4;
    u32 idx = 0;
    f64 x0, y0, x1, y1;
    printf("pairs and hs result:\n", npairs);
    for (int p = 0; p < npairs; ++p) {
        idx = p * 4;
        x0 = floc[idx];
        y0 = floc[idx + 1];
        x1 = floc[idx + 2];
        y1 = floc[idx + 3];
        printf("(%.16f, %.16f) (%.16f, %.16f)  ->  %.16f\n",
            x0, y0, x1, y1,
            ReferenceHaversine(x0, y0, x1, y1, 6372.8)
            );
    }
}


void Test() {
    printf("Running tests...\n");

    u8* dest_json = (u8*) malloc(10 * MEGABYTE);
    const char *filename = "hspairs_parsetest.json";
    LoadFilePathBin((char*) filename, dest_json);
    printf("loaded file %s:\n%s\n", filename, dest_json);

    // parse tokens and print
    {
        Tokenizer tokenizer;
        tokenizer.at = (char*) dest_json;
        Token tok;
        u16 i = 0;
        printf("parse result of <=100 tokens:\n");
        do {
            tok = GetToken(&tokenizer);
            PrintToken(tok);
            ++i;
        } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF && i < 100);
    }

    // parse floats and put into data storage
    u8* dest_floats = (u8*) malloc(MEGABYTE);
    u8* floc = dest_floats;
    u32 iter = 0;
    {
        Tokenizer tokenizer;
        tokenizer.at = (char*) dest_json;
        Token tok;
        printf("\n\float nparse result of all tokens:\n");
        u32 npairs_parsed = 0;
        do {
            tok = GetToken(&tokenizer);
            if (tok.type == TOK_DOUBLE) {
                f64 val = ParseDouble(tok.str, tok.len);
                printf("str value:    %.*s\n", tok.len, tok.str);
                printf("parsed value: %.15f\n\n", val);
            }
            ++iter;
        } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF && iter < 100);
    }
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
