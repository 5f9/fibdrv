#include <fcntl.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "fib.h"

#define FIB_DEV "/dev/fibonacci"

#define GETTIME(x) (long long) ((x).tv_sec * 1e9 + (x).tv_nsec)

static int type_fd;

void mpz_set_ui256_ptr(mpz_ptr x, u256 *y)
{
    mpz_set_ui(x, y->high >> 64);
    mpz_mul_2exp(x, x, 64);
    mpz_add_ui(x, x, y->high);
    mpz_mul_2exp(x, x, 64);
    mpz_add_ui(x, x, y->low >> 64);
    mpz_mul_2exp(x, x, 64);
    mpz_add_ui(x, x, y->low);
}

static inline void print_proxy(long long k, mpz_ptr mpz)
{
    gmp_printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence %Zd.\n",
               k, mpz);
}

static void fib_time_proxy(fib_256_f fib, long long k, long long *spend)
{
    u256 result = {.low = 0, .high = 0};
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fib(&result, k);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (k > INT128_MAX_LENGTH && result.high == 0) {
        return;
    }
    *spend = GETTIME(end) - GETTIME(start);
    mpz_t mpz;
    mpz_init(mpz);
    mpz_set_ui256_ptr(mpz, &result);
    print_proxy(k, mpz);
    mpz_clear(mpz);
}

static inline void fib_loop_proxy(fib_256_f fib,
                                  long long offset,
                                  FILE *logfile)
{
    long long sz = 0;
    for (int i = 0; i <= offset; i++) {
        fib_time_proxy(fib, i, &sz);
        fprintf(logfile, "%d %lld\n", i, sz);
    }
    for (int i = offset; i >= 0; i--) {
        fib_time_proxy(fib, i, &sz);
        fprintf(logfile, "%d %lld\n", i, sz);
    }
}

static void fib_log_proxy(fib_256_f fib, long long offset, char *filename)
{
    FILE *logfile = fopen(filename, "w");
    if (logfile) {
        fib_loop_proxy(fib, offset, logfile);
        fclose(logfile);
    }
}

static inline void kernel_read_proxy(int fd, int i, mpz_ptr mpz, FILE *logfile)
{
    u256 v;
    lseek(fd, i, SEEK_SET);
    ssize_t sz = read(fd, &v, INT256_SIZE);
    if (i > INT128_MAX_LENGTH && v.high == 0) {
        return;
    }
    mpz_set_ui256_ptr(mpz, &v);
    print_proxy(i, mpz);
    fprintf(logfile, "%d %llu\n", i, (unsigned long long) sz);
}

static void kernel_read_loop_proxy(int fd, long long offset, FILE *logfile)
{
    mpz_t mpz;
    mpz_init(mpz);
    for (int i = 0; i <= offset; i++) {
        kernel_read_proxy(fd, i, mpz, logfile);
    }

    for (int i = offset; i >= 0; i--) {
        kernel_read_proxy(fd, i, mpz, logfile);
    }
    mpz_clear(mpz);
}

static void kernel_read_log_proxy(char type,
                                  int fd,
                                  long long offset,
                                  char *filename)
{
    lseek(type_fd, 0, SEEK_SET);
    if (!write(type_fd, &(type), 1))
        return;

    FILE *logfile = fopen(filename, "w");
    if (logfile) {
        kernel_read_loop_proxy(fd, offset, logfile);
        fclose(logfile);
    }
}

static void kernel_to_user_log_proxy(char type,
                                     int fd,
                                     long long offset,
                                     char *filename)
{
    FILE *logfile = fopen(filename, "w");
    if (logfile) {
        FILE *uk_file = fopen("logs/user_to_kernel_time.dat", "w");
        if (uk_file) {
            struct timespec start, end;
            for (int i = 0; i <= offset; i++) {
                clock_gettime(CLOCK_MONOTONIC, &start);
                ssize_t ktime = write(fd, &type, 1);
                clock_gettime(CLOCK_MONOTONIC, &end);
                fprintf(uk_file, "%d %lld\n", i, ktime - GETTIME(start));
                fprintf(logfile, "%d %lld\n", i, GETTIME(end) - ktime);
            }
            fclose(uk_file);
        }
        fclose(logfile);
    }
}

struct user_fib {
    char *name;
    fib_256_f func;
    char *logfile;
};

static struct user_fib user_fibs[] = {
    {.name = "user doubling clz",
     .func = &fib_doubling_128_clz_proxy,
     .logfile = "logs/user_doubling_clz_time.dat"},

    {.name = "user doubling",
     .func = &fib_doubling_128_proxy,
     .logfile = "logs/user_doubling_time.dat"},

    {.name = "user sequence",
     .func = &fib_sequence_128_proxy,
     .logfile = "logs/user_sequence_time.dat"},

    {.name = "user doubling 256 clz",
     .func = &fib_doubling_256_clz,
     .logfile = "logs/user_doubling_256_clz_time.dat"},

    {.name = "user sequence 256",
     .func = &fib_iterative_256,
     .logfile = "logs/user_sequence_256_time.dat"},
};

typedef void (*kernel_fib_f)(char, int, long long, char *);

struct kernel_fib {
    char *name;
    char type;
    kernel_fib_f func;
    char *logfile;
};

static struct kernel_fib kernel_fibs[] = {
    // kernel to user time spend
    {.name = "kernel to user time spend",
     .type = 't',
     .func = &kernel_to_user_log_proxy,
     .logfile = "logs/kernel_to_user_time.dat"},

    // doubling method clz
    {.name = "kernel doubling clz",
     .type = 'z',
     .func = &kernel_read_log_proxy,
     .logfile = "logs/kernel_doubling_clz_time.dat"},
    // doubling method
    {.name = "kernel doubling",
     .type = 'd',
     .func = &kernel_read_log_proxy,
     .logfile = "logs/kernel_doubling_time.dat"},
    // sequence
    {.name = "kernel sequence",
     .type = 's',
     .func = &kernel_read_log_proxy,
     .logfile = "logs/kernel_sequence_time.dat"},

    // doubling method 256 clz
    {.name = "kernel doubling 256 clz",
     .type = 'Z',
     .func = &kernel_read_log_proxy,
     .logfile = "logs/kernel_doubling_256_clz_time.dat"},
    // sequence 256
    {.name = "kernel sequence 256",
     .type = 'S',
     .func = &kernel_read_log_proxy,
     .logfile = "logs/kernel_sequence_256_time.dat"},
};

int main(int argc, char *argv[])
{
    bool kernel = (argc > 1) ? !strcmp("kernel", argv[1]) : true;
    bool user = (argc > 1) ? !strcmp("user", argv[1]) : true;

    long long offset = MAX_LENGTH;
    if (user) {
        int length = sizeof user_fibs / sizeof user_fibs[0];
        for (size_t i = 0; i < length; i++) {
            struct user_fib *fib = &user_fibs[i];
            fprintf(stderr, "%80s\n", fib->name);
            fib_log_proxy(fib->func, offset, fib->logfile);
        }
    }
    if (kernel) {
        int fd = open(FIB_DEV, O_RDWR);
        if (fd < 0) {
            perror("Failed to open character device");
            exit(1);
        }
        type_fd = open(FIBONACCI_TYPE_NAME, O_RDWR);
        if (type_fd < 0) {
            close(fd);
            perror("Failed to open kobject");
            exit(1);
        }

        int length = sizeof kernel_fibs / sizeof kernel_fibs[0];
        for (size_t i = 0; i < length; i++) {
            struct kernel_fib *fib = &kernel_fibs[i];
            fprintf(stderr, "%80s\n", fib->name);
            (fib->func)(fib->type, fd, offset, fib->logfile);
        }

        close(type_fd);
        close(fd);
    }

    return 0;
}
