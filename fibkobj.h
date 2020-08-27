#pragma once

typedef void (*func_fibkobj_callback)(char);

char get_fibonacci_type(void);
void notify_fibonacci_type_callback(func_fibkobj_callback);