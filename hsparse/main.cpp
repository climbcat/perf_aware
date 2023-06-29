#include "../lib.c"

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
bool IsWhitespace(char c) {
    return
        c == ' ' ||
        c == '\t' ||
        c == '\v' ||
        c == '\f' ||
        c == '\n' ||
        c == '\r';
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
                if (c == '\0') {
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




void ParseHsPointsJson(char *filename) {
    
    // sys / process timing [secs]
    u64 time_0 = ReadSystemTimerMySec();
    u64 tick_0 = ReadCPUTimer();

    // read
    u64 size_bytes;
    char* dest_json = LoadFileMMAP(filename, &size_bytes);

    u64 tick_1_read = ReadCPUTimer();

    // parse floats and put into data storage
    Tokenizer tokenizer;
    tokenizer.at = dest_json;

    f64* data = (f64*) malloc(size_bytes/2);
    u32 fidx = 0;
    Token tok;
    do {
        tok = GetToken(&tokenizer);
        if (tok.type == TOK_DOUBLE) {
            data[fidx] = ParseDouble(tok.str, tok.len);
            ++fidx;
        }
    } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF);

    u64 tick_2_parse = ReadCPUTimer();

    // do the sum
    f64 sum = 0;
    f64 mean = 0;
    u32 idx = 0;
    f64 x0, y0, x1, y1, d;
    u32 npairs = (fidx + 1) / 4;

    for (int p = 0; p < npairs; ++p) {
        idx = p * 4;

        x0 = data[idx];
        y0 = data[idx + 1];
        x1 = data[idx + 2];
        y1 = data[idx + 3];
        d = ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
        sum += d;

        //printf("(%.16f, %.16f) (%.16f, %.16f)  ->  %.16f\n", x0, y0, x1, y1, d);
    }
    mean = sum / npairs;
    printf("Haversine dist mean over %d pairs: %.16f\n", npairs, mean);

    u64 tick_3_sum = ReadCPUTimer();
    
    // print timing results
    printf("\n");
    CalibrateRdtsc(10000);
    u64 time_1 = ReadSystemTimerMySec();

    u64 total_ticks = tick_3_sum - tick_0;

    u64 tdiff_read = tick_1_read - tick_0;
    u64 tdiff_parse = tick_2_parse - tick_1_read;
    u64 tdiff_sum = tick_3_sum - tick_2_parse;

    f64 tpercent_read = tdiff_read / (float) total_ticks * 100;
    f64 tpercent_parse = tdiff_parse / (float) total_ticks * 100;
    f64 tpercent_sum = tdiff_sum / (float) total_ticks * 100;

    printf("Total time:  %f secs\n", (time_1 - time_0) / (double) 1000000.0);
    printf("Read ticks:  %lu (%.2f %%)\n", tdiff_read, tpercent_read);
    printf("Parse ticks: %lu (%.2f %%)\n", tdiff_parse, tpercent_parse);
    printf("Sum ticks:   %lu (%.2f %%)\n", tdiff_sum, tpercent_sum);
}


void Test() {
    printf("Running tests...\n");

    const char *filename = "parsetest.json";
    u8* dest_json = (u8*) LoadFileMMAP((char*) filename);
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
                //printf("str value:    %.*s\n", tok.len, tok.str);
                //printf("parsed value: %.15f\n\n", val);
            }
            ++iter;
        } while (tok.type != TOK_UNDEFINED && tok.type != TOK_EOF && iter < 100);
    }
}

int main (int argc, char **argv) {
    if (CLAContainsArg("--help", argc, argv) || argc != 2) {
        printf("Usage:\n        hsparse <pairs_json_file>\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv)) {
        Test();
        exit(0);
    }

    char *filename = argv[1];

    ParseHsPointsJson(filename);
}
