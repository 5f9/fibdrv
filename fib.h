#pragma once

/* MAX_LENGTH is set to 92 because
 * ssize_t can't fit the number > 92
 */
#define MAX_LENGTH 370
#define INT128_MAX_LENGTH 186
#define INT128_SIZE 16
#define INT256_SIZE 32
#define FIBONACCI_TYPE_NAME "/sys/kernel/fibonacci/fibonacci_type"

#ifdef __KERNEL__
#include <linux/bitops.h>
#else
#include <stddef.h>
#include <stdint.h>

typedef __uint64_t u64;

/*
 * fls: find last (most-significant) bit set.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static __inline__ int fls(unsigned int x)
{
    return 32 - __builtin_clz(x);
}

#endif

typedef unsigned __int128 u128;
struct uint256 {
    u128 low, high;
};
typedef struct uint256 u256;

union union_mixing256 {
    u64 ui64;
    u128 ui128;
    u256 ui256;
};

struct mixing {
    union union_mixing256 a, b;
};

/*
 * References https://github.com/chenshuo/recipes/blob/master/basic/int128.h
 */
static inline void ui256_set_si(u256 *out, __int128 l)
{
    out->low = l;
    out->high = l < 0 ? -1 : 0;
}

static inline void ui256_set(u256 *out, u256 *v)
{
    out->low = v->low;
    out->high = v->high;
}

static inline void ui256_add_ui(u256 *out, u128 y)
{
    out->low += y;
    out->high += (out->low < y);
}

static inline void ui256_add(u256 *out, u256 *x, u256 *y)
{
    out->low = x->low + y->low;
    out->high = x->high + y->high + (out->low < y->low);
}

static inline void ui256_sub(u256 *out, u256 *x, u256 *y)
{
    u256 c = {.low = ~y->low, .high = ~y->high};
    ui256_add_ui(&c, 1);
    out->low = x->low + c.low;
    out->high = x->high + c.high + (out->low < c.low);
}

static inline void ui256_mul(u256 *out, u128 x, u128 y)
{
    u64 a = x;  // & 0xFFFFFFFFFFFFFFFF;
    u64 c = x >> 64;
    u64 b = y;  // & 0xFFFFFFFFFFFFFFFF;
    u64 d = y >> 64;
    /*
    if (b == 0 && d == 0) {
        out->low = a*c;
        out->high = 0;
        return ;
    }
    */
    u128 ab = (u128) a * b;
    u128 bc = (u128) b * c;
    u128 ad = (u128) a * d;
    u128 cd = (u128) c * d;

    out->low = ab + (bc << 64);
    out->high = cd + (bc >> 64) + (ad >> 64) + (out->low < ab);
    out->low += (ad << 64);
    out->high += (out->low < (ad << 64));
}

typedef u128 (*fib_f)(int);

u128 fib_sequence(int);
u128 fib_doubling(int);
u128 fib_doubling_clz(int);

typedef void (*fib_256_f)(u256 *, int);

void fib_iterative_256(u256 *, int);
void fib_doubling_256_clz(u256 *, int);

void fib_doubling_128_clz_proxy(u256 *, int);
void fib_doubling_128_proxy(u256 *, int);
void fib_sequence_128_proxy(u256 *, int);