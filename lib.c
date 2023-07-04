#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "math.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define GIGABYTE 1024*1024*1024
#define MEGABYTE 1024 * 1024
#define KILOBYTE 1024

#define SIXTEEN_MB 16 * MEGABYTE
#define SIXTEEN_KB 16 * KILOBYTE
#define THIRTYTWO_KB 32 * KILOBYTE
#define SIXTYFOUR_KB 64 * KILOBYTE


//
// min / max


inline u8 MinU8(u8 a, u8 b) { return (a <= b) ? a : b; }
inline u16 MinU16(u16 a, u16 b) { return (a <= b) ? a : b; }
inline u32 MinU32(u32 a, u32 b) { return (a <= b) ? a : b; }
inline u64 MinU64(u64 a, u64 b) { return (a <= b) ? a : b; }

inline s8 MinS8(s8 a, s8 b) { return (a <= b) ? a : b; }
inline s16 MinS16(s16 a, s16 b) { return (a <= b) ? a : b; }
inline s32 MinS32(s32 a, s32 b) { return (a <= b) ? a : b; }
inline s64 MinS64(s64 a, s64 b) { return (a <= b) ? a : b; }

inline f32 MinF32(f32 a, f32 b) { return (a <= b) ? a : b; }
inline f64 MinF64(f64 a, f64 b) { return (a <= b) ? a : b; }

inline u8 MaxU8(u8 a, u8 b) { return (a > b) ? a : b; }
inline u16 MaxU16(u16 a, u16 b) { return (a > b) ? a : b; }
inline u32 MaxU32(u32 a, u32 b) { return (a > b) ? a : b; }
inline u64 MaxU64(u64 a, u64 b) { return (a > b) ? a : b; }

inline s8 MaxS8(s8 a, s8 b) { return (a > b) ? a : b; }
inline s16 MaxS16(s16 a, s16 b) { return (a > b) ? a : b; }
inline s32 MaxS32(s32 a, s32 b) { return (a > b) ? a : b; }
inline s64 MaxS64(s64 a, s64 b) { return (a > b) ? a : b; }

inline f32 MaxF32(f32 a, f32 b) { return (a > b) ? a : b; }
inline f64 MaxF64(f64 a, f64 b) { return (a > b) ? a : b; }


//
// linked list


struct LList1 {
    LList1 *next;
};

struct LList2 {
    LList2 *next;
    LList2 *prev;
};

struct LList3 {
    LList3 *next;
    LList3 *prev;
    LList3 *descend;
};

void *InsertBefore1(void *newlink, void *before) {
    LList1 *newlnk = (LList1*) newlink;

    newlnk->next = (LList1*) before;
    return newlink;
}
void *InsertAfter1(void *after, void *newlink) {
    ((LList1*) after)->next = (LList1*) newlink;
    return newlink;
}
void InsertBefore2(void *newlink, void *before) {
    LList2 *newlnk = (LList2*) newlink;
    LList2 *befre = (LList2*) before;

    newlnk->prev = befre->prev;
    newlnk->next = befre;
    if (befre->prev != NULL) {
        befre->prev->next = newlnk;
    }
    befre->prev = newlnk;
}
void InsertBelow3(void *newlink, void *below) {
    LList3 *newlnk = (LList3*) newlink;
    LList3 *belw = (LList3*) below;

    newlnk->descend = belw->descend;
    belw->descend = newlnk;
}


//
// memory allocation


// NOTE: The pool de-alloc function does not check whether the element was ever allocated. I am not
// sure how to do this - perhaps with an occupation list. However tis adds complexity tremendously.
// Maybe with a hash list, seems overkill. Having a no-use header for every element either messes
// up alignment or compactness.
// TODO: arena de-allocation that shrinks commited memory (useful when e.g. closing large files)
// TODO: scratch arenas


struct MArena {
    u8 *mem;
    u64 mapped;
    u64 committed;
    u64 used;
    bool locked = false;
};

#define ARENA_RESERVE_SIZE GIGABYTE
#define ARENA_COMMIT_CHUNK SIXTEEN_KB

struct MPool {
    u8 *mem;
    u32 block_size;
    u32 nblocks;
    LList1 *free_list;
};

#define MPOOL_CACHE_LINE_SIZE 64

u64 MemoryProtect(void *from, u64 amount) {
    mprotect(from, amount, PROT_READ | PROT_WRITE);
    return amount;
}
void *MemoryReserve(u64 reserve_size) {
    void *result = mmap(NULL, ARENA_RESERVE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    return result;
}
MArena ArenaCreate() {
    MArena a;

    a.mem = (u8*) mmap(NULL, ARENA_RESERVE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    a.mapped = ARENA_RESERVE_SIZE;
    mprotect(a.mem, ARENA_COMMIT_CHUNK, PROT_READ | PROT_WRITE);
    a.committed = ARENA_COMMIT_CHUNK;
    a.used = 0;

    return a;
}
inline
void *ArenaAlloc(MArena *a, u64 len) {
    assert(!a->locked && "ArenaAlloc: memory arena is open, use MArenaClose to allocate");

    if (a->committed < a->used + len) {
        MemoryProtect(a->mem + a->committed, (len / ARENA_COMMIT_CHUNK + 1) * SIXTEEN_KB);
    }
    void *result = a->mem + a->used;
    a->used += len;

    return result;
}
void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    memcpy(dest, data, len);
    return dest;
}
void *ArenaOpen(MArena *a) {
    assert(!a->locked && "ArenaOpen: memory arena is alredy open");

    a->locked = true;
    return a->mem + a->used;
}
void ArenaClose(MArena *a, u64 len) {
    assert(a->locked && "ArenaClose: memory arena not open");

    a->locked = false;
    ArenaAlloc(a, len);
}

MPool PoolCreate(u32 block_size_min, u32 nblocks) {
    assert(nblocks > 1);

    MPool p;
    p.block_size = MPOOL_CACHE_LINE_SIZE * (block_size_min / MPOOL_CACHE_LINE_SIZE + 1);
    p.mem = (u8*) mmap(NULL, p.block_size * nblocks, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);
    p.free_list = (LList1*) p.mem;

    LList1 *current = p.free_list;
    for (u32 i = 0; i < nblocks - 1; ++i) {
        current->next = (LList1*) (p.mem + i * p.block_size);
        current = current->next;
    }
    current->next = NULL;

    return p;
}
void *PoolAlloc(MPool *p) {
    if (p->free_list == NULL) {
        return NULL;
    }
    void *retval = p->free_list;
    p->free_list = p->free_list->next;
    memset(retval, 0, MPOOL_CACHE_LINE_SIZE);

    return retval;
}
void PoolFree(MPool *p, void *element) {
    assert(element >= (void*) p->mem); // check lower bound
    u64 offset = (u8*) element -  p->mem;
    assert(offset % p->block_size == 0); // check alignment
    assert(offset < p->block_size * p->nblocks); // check upper bound

    LList1 *element_as_list = (LList1*) element;
    element_as_list->next = p->free_list;
    p->free_list = element_as_list;
}


//
// strings

// NOTE: currently no "string list header" struct, which means that strings and str lst are
// treated a bit differently: Strings are passed as a struct, but str lists as a pointer.
// NOTE: the string list is in fact an LList1

struct String {
    char *str = NULL;
    u32 len = 0;
};

struct StringList {
    StringList *next = NULL;
    String value;
};

String StrLiteral(MArena *a, const char *lit) {
    String s;
    s.len = 0;
    while (*(lit + s.len) != '\0') {
        ++s.len;
    }
    s.str = (char*) ArenaAlloc(a, s.len);
    memcpy(s.str, lit, s.len);

    return s;
}
void StrPrint(const char *format, String s) {
    u8 format_str_max_len = 255;
    char buff[s.len + format_str_max_len];
    sprintf(buff, "%.*s", s.len, s.str);
    printf(format, buff);
}
void StrPrint(String s) {
    printf("%.*s", s.len, s.str);
}
bool StrEqual(String a, String b) {
    u32 i = 0;
    u32 len = MinU32(a.len, b.len);
    while (i < len) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
        ++i;
    }

    return a.len == b.len;
}
String StrCat(MArena *arena, String a, String b) {
    String cat;
    cat.len = a.len + b.len;
    cat.str = (char*) ArenaAlloc(arena, cat.len);
    memcpy(cat.str, a.str, a.len);
    memcpy(cat.str + a.len, b.str, b.len);

    return cat;
}
void StrLstPrint(StringList *lst) {
    while (lst != NULL) {
        StrPrint(lst->value);
        printf(", ");
        lst = lst->next;
    }
}
StringList *StrSplit(MArena *arena, String base, char split_at_and_remove) {
    StringList *first;
    StringList *node;
    StringList *prev = NULL;
    u32 i = 0;

    while (true) {
        while (base.str[i] == split_at_and_remove) {
            ++i;
        }
        if (i >= base.len) {
            return first;
        }

        node = (StringList *) ArenaAlloc(arena, sizeof(StringList));
        node->value.str = (char*) ArenaOpen(arena);
        node->value.len = 0;

        if (prev != NULL) {
            prev->next = node;
        }
        else {
            first = node;
        }

        int j = 0;
        while (base.str[i] != split_at_and_remove && i < base.len) {
            ++node->value.len;
            node->value.str[j] = base.str[i];
            ++i;
            ++j;
        }

        ArenaClose(arena, node->value.len);
        prev = node;
        if (i < base.len) {
            ++i; // skip the split char
        }
        else {
            return first;
        }
    }
}
// TODO: impl. "arena push version" e.g. a version that uses ArenaPush(src, len) rather than
//     ArenaAlloc() to see which one is most readable, I expect it to be better
// StringList *StrSplit(MArena *arena, String base, char split) {}

String StrJoin(MArena *a, StringList *strs) {
    String join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        memcpy(join.str + join.len, strs->value.str, strs->value.len);
        join.len += strs->value.len;
        strs = strs->next;
    }

    ArenaClose(a, join.len);
    return join;
}
String StrJoinInsertChar(MArena *a, StringList *strs, char insert) {
    String join;
    join.str = (char*) ArenaOpen(a);
    join.len = 0;

    while (strs != NULL) {
        memcpy(join.str + join.len, strs->value.str, strs->value.len);
        join.len += strs->value.len;
        strs = strs->next;

        if (strs != NULL) {
            join.str[join.len] = insert;
            ++join.len;
        }
    }

    ArenaClose(a, join.len);
    return join;
}


//
// string to number


u32 ParseInt(char *text) {
    u32 val = 0;
    u32 multiplier = 1;

    // signed?
    bool sgned = text[0] == '-';
    if (sgned) {
        ++text;
    }

    u32 len = strlen(text);

    // decimals before dot
    for (int i = 0; i < len; ++i) {
        val += (text[len - 1 - i] - 48) * multiplier;
        multiplier *= 10;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
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


//
// random


#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long)0xffffffffffffffffUL)
#endif


void Kiss_SRandom(unsigned long state[7], unsigned long seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
unsigned long Kiss_Random(unsigned long state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}
unsigned long _hash(unsigned long x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
unsigned long g_state[7];
bool g_didinit = false;
#define Random() Kiss_Random(g_state)
u32 RandInit(u32 seed = 0) {
    if (g_didinit == true)
        return 0;

    if (seed == 0) {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        seed = _hash((unsigned long) tm.tv_sec*1000000 + tm.tv_usec);
    }
    Kiss_SRandom(g_state, seed);

    g_didinit = true;
    return seed;
}

double Rand01() {
    double randnum;
    randnum = (double) Random();
    randnum /= (double) ULONG_MAX + 1;
    return randnum;
}
double RandPM1() {
    double randnum;
    randnum = (double) Random();
    randnum /= ((double) ULONG_MAX + 1) / 2;
    randnum -= 1;
    return randnum;
}

int RandMinMaxI(int min, int max) {
    // TODO: assert macro
    //assert(max > min);
    return Random() % (max - min + 1) + min;
}


//
// cmd-line args


bool CLAContainsArg(const char *search, int argc, char **argv, int *idx = NULL) {
    for (int i = 0; i < argc; ++i) {
        char *arg = argv[i];
        if (!strcmp(argv[i], search)) {
            if (idx != NULL) {
                *idx = i;
            }
            return true;
        }
    }
    return false;
}

bool CLAContainsArgs(const char *search_a, const char *search_b, int argc, char **argv) {
    bool found_a = false;
    bool found_b = false;
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], search_a)) {
            found_a = true;
        }
        if (!strcmp(argv[i], search_b)) {
            found_b = true;
        }
    }
    return found_a && found_b;
}

char *CLAGetArgValue(const char *key, int argc, char **argv) {
    int i;
    bool error = !CLAContainsArg(key, argc, argv, &i) || i == argc - 1;;
    if (error == false) {
        char *val = argv[i+1];
        error = strlen(val) > 1 && val[0] == '-' && val[1] == '-';
    }
    if (error == true) {
        printf("KW arg %s must be followed by a value arg\n", key);
        exit(0);
    }
    return argv[i+1];
}


//
// file I/O


char *LoadFileMMAP(char *filepath, u64 *size_bytes = NULL) {
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("Could not open file: %s\n", filepath);
        exit(1);
    }

    s32 fd = fileno(f);
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        printf("Could not get file size: %s\n", filepath);
        exit(1);
    }

    char *str = (char*) mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0);
    if (size_bytes != NULL) {
        *size_bytes = sb.st_size;
    }

    fclose(f);
    return str;
}


//
// timing


u64 ReadSystemTimerMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}

u64 ReadCPUTimer() {
    // gcc:
    u64 ticks = __builtin_ia32_rdtsc();
    return ticks;
}

struct ProfilerBlock {
    ProfilerBlock *next = NULL;
    String name;
    u64 tsc_diff = 0;
};

struct Profiler {
    MArena arena;
    ProfilerBlock *first = NULL;
    ProfilerBlock *current = NULL;

    u64 cputime_diff = 0;
    u64 systime_diff = 0;
    float cpu_freq = 0;
};

Profiler ProfilerInit() {
    ProfilerBlock block;
    Profiler prof;
    prof.arena = ArenaCreate();
    prof.first = (ProfilerBlock*) ArenaPush(&prof.arena, &block, sizeof(block));
    prof.current = prof.first;
    prof.systime_diff = ReadSystemTimerMySec();
    prof.cputime_diff = ReadCPUTimer();

    return prof;
}

ProfilerBlock *ProfilerReadBlockStart(Profiler *p, const char *func_name) {
    ProfilerBlock *block = (ProfilerBlock*) ArenaPush(&p->arena, &block, sizeof(ProfilerBlock));
    p->current = (ProfilerBlock*) InsertAfter1(p->current, block);
    p->current->next = NULL;
    p->current->name = StrLiteral(&p->arena, func_name);
    p->current->tsc_diff = ReadCPUTimer();
    return p->current;
}
void ProfilerReadBlockEnd(Profiler *p, ProfilerBlock *block) {
    block->tsc_diff = ReadCPUTimer() - block->tsc_diff;
}

void ProfilerStop(Profiler *p) {
    p->systime_diff = ReadSystemTimerMySec() - p->systime_diff;
    p->cputime_diff = ReadCPUTimer() - p->cputime_diff;
    p->cpu_freq = (float) p->cputime_diff / p->systime_diff;
    p->current = NULL;
}
void ProfilerPrint(Profiler *p) {
    ProfilerBlock *current = p->first->next;
    printf("\n");
    printf("Total time: %lu mysec", p->systime_diff);
    printf(" (tsc: %lu,", p->cputime_diff);
    printf(" freq [tsc/mys]: %f)\n", p->cpu_freq);
    while (current != NULL) {
        StrPrint("  %s: ", current->name);
        printf("%lu (%f %%)\n", current->tsc_diff, (double) current->tsc_diff / p->cputime_diff * 100);
        current = current->next;
    }
}

Profiler g_prof = ProfilerInit();
class ProfileScopeMechanism {
    ProfilerBlock *block;
public:
    ProfileScopeMechanism(const char * func_name) {
        this->block = ProfilerReadBlockStart(&g_prof, func_name);
    }
    ~ProfileScopeMechanism() {
        ProfilerReadBlockEnd(&g_prof, this->block);
    }
};

#define TimeFunction ProfileScopeMechanism __prof_mechanism__(__FUNCTION__);
#define TimeBlock(name) ProfileScopeMechanism __prof_mechanism__(name);
#define TimePrint ProfilerStop(&g_prof); ProfilerPrint(&g_prof);


//
// performance aware:


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


#define EARTH_RADIUS 6372.8
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
