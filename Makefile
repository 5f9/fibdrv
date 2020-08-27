CONFIG_MODULE_SIG = n
TARGET_MODULE := fibdrv

obj-m := fibkobj.o fibkm.o $(TARGET_MODULE).o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) client
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) -rf client out logs
load: $(TARGET_MODULE).ko
	sudo insmod fibkm.ko
	sudo insmod fibkobj.ko
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null
	sudo rmmod fibkobj.ko || true >/dev/null
	sudo rmmod fibkm.ko || true >/dev/null

client: client.c fib.c
	$(CC) -o $@ $^ -Wall -Werror -O3 -g -lgmp

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

check: all
	@mkdir -p logs
	$(MAKE) unload
	$(MAKE) load
	sudo taskset 0x1 ./client > out
	$(MAKE) unload
	@scripts/verify.py && $(call pass)

performance:
	sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"
	sudo sh scripts/performance.sh

plot: logs
	gnuplot scripts/time.gp
	eog logs/time.png &

format:
	clang-format -i *.[ch]
