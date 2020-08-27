#include "fib.h"

u128 fib_sequence(int k)
{
    u128 a = 0, b = 1, f = k;

    for (size_t i = 2; i <= (size_t) k; i++) {
        f = a + b;
        a = b;
        b = f;
    }
    return f;
}

static inline void _fib_doubling(int k, u128 *a, u128 *b)
{
    // *a = 1; // F(1)=1
    // *b = 1; // F(2)=1
    if (k < 2) {
        *a = 1;
        *b = 1;
        return;
    }
    u128 c, d;
    size_t m = k >> 1;
    _fib_doubling(m, a, b);
    // c = a * ((b << 1) - a)
    c = *a * ((*b << 1) - *a);
    // d = a^2 + b^2
    d = *a * *a + *b * *b;

    if (k & 1) {
        *a = d;
        *b = c + d;
    } else {
        *a = c;
        *b = d;
    }
}

u128 fib_doubling(int k)
{
    if (k < 2)
        return k;

    // start from a=F(1) b=F(2), calculate loop count
    u128 a, b;
    _fib_doubling(k - 1, &a, &b);
    return b;
}

static inline u128 _fib_doubling_clz(int k)
{
    /* use clz/ctz and fast algorithms to speed up */
    int count = fls(k) - 1;
    unsigned int mask = 1 << (count);

    // *a = 1; // F(1)=1
    // *b = 1; // F(2)=1
    u128 a = 1, b = 1, c, d;
    for (size_t i = 0; i < count; i++) {
        mask >>= 1;

        // c = a * ((b << 1) - a)
        c = a * ((b << 1) - a);
        // d = a^2 + b^2
        d = a * a + b * b;

        if (k & mask) {
            a = d;
            b = c + d;
        } else {
            a = c;
            b = d;
        }
    }
    return b;
}

u128 fib_doubling_clz(int k)
{
    if (k < 2)
        return k;

    // start from a=F(1) b=F(2), calculate loop count
    return _fib_doubling_clz(k - 1);
}

void fib_iterative_256(u256 *f, int n)
{
    if (n < 2) {
        ui256_set_si(f, n);
        return;
    }

    u256 a = {.low = 0, .high = 0}, b = {.low = 1, .high = 0};

    for (size_t i = 2; i <= (size_t) n; i++) {
        ui256_add(f, &a, &b);
        ui256_set(&a, &b);
        ui256_set(&b, f);
    }
}

static inline void _fib_doubling_256_clz(unsigned int n, u256 *a, u256 *b)
{
    // start from a=F(1) b=F(2), calculate loop count
    unsigned int count = fls(n) - 1;
    unsigned int mask = 1 << (count);

    // *a = 1; // F(1)=1
    // *b = 1; // F(2)=1
    u256 c, d;
    for (size_t i = 0; i < count; i++) {
        mask >>= 1;
        // c = a * ((b << 1) - a)
        ui256_add(&c, b, b);
        ui256_sub(&c, &c, a);
        ui256_mul(&c, c.low, a->low);

        // d = a^2 + b^2
        ui256_mul(a, a->low, a->low);
        ui256_mul(b, b->low, b->low);
        ui256_add(&d, a, b);

        if (n & mask) {
            ui256_set(a, &d);
            ui256_add(b, &c, &d);
        } else {
            ui256_set(a, &c);
            ui256_set(b, &d);
        }
    }
}

void fib_doubling_256_clz(u256 *f, int n)
{
    if (n < 2) {
        f->low = n;
        f->high = 0;
        return;
    }

    u256 a = {.low = 1, .high = 0};  // F(1)=1
    f->low = 1;                      // F(2)=1
    f->high = 0;

    _fib_doubling_256_clz(n - 1, &a, f);
}

void fib_doubling_128_clz_proxy(u256 *v, int n)
{
    v->low = fib_doubling_clz(n);
}

void fib_doubling_128_proxy(u256 *v, int n)
{
    v->low = fib_doubling(n);
}

void fib_sequence_128_proxy(u256 *v, int n)
{
    v->low = fib_sequence(n);
}
