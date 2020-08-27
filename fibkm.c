#include <linux/module.h>
#include "fib.c"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Solomon Hsu <solnone@gmail.com>");

EXPORT_SYMBOL_GPL(fib_sequence_128_proxy);
EXPORT_SYMBOL_GPL(fib_doubling_128_proxy);
EXPORT_SYMBOL_GPL(fib_doubling_128_clz_proxy);
EXPORT_SYMBOL_GPL(fib_iterative_256);
EXPORT_SYMBOL_GPL(fib_doubling_256_clz);
