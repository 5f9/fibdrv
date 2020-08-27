// SPDX-License-Identifier: GPL-2.0
/*
 * Sample kobject implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 */
#include "fibkobj.h"
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>

static char fibonacci_type;

char get_fibonacci_type(void)
{
    return fibonacci_type;
}

static func_fibkobj_callback fib_func = NULL;

void notify_fibonacci_type_callback(func_fibkobj_callback func)
{
    fib_func = func;
}

/*
 * The "fibonacci_type" file where a static variable is read from and written
 * to.
 */
static ssize_t fibonacci_type_show(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   char *buf)
{
    *buf = fibonacci_type;
    return 1;
}

static ssize_t fibonacci_type_store(struct kobject *kobj,
                                    struct kobj_attribute *attr,
                                    const char *buf,
                                    size_t count)
{
    fibonacci_type = *buf;
    if (fib_func) {
        fib_func(fibonacci_type);
    }
    return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute fibonacci_type_attribute =
    __ATTR(fibonacci_type, 0664, fibonacci_type_show, fibonacci_type_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
    &fibonacci_type_attribute.attr,
    NULL, /* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *fibkobj_kobj;

static int __init fibkobj_init(void)
{
    int retval;

    /*
     * Create a simple kobject with the name of "fibonacci",
     * located under /sys/kernel/
     *
     * As this is a simple directory, no uevent will be sent to
     * userspace.  That is why this function should not be used for
     * any type of dynamic kobjects, where the name and number are
     * not known ahead of time.
     */
    fibkobj_kobj = kobject_create_and_add("fibonacci", kernel_kobj);
    if (!fibkobj_kobj)
        return -ENOMEM;

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(fibkobj_kobj, &attr_group);
    if (retval)
        kobject_put(fibkobj_kobj);

    return retval;
}

static void __exit fibkobj_exit(void)
{
    kobject_put(fibkobj_kobj);
}

module_init(fibkobj_init);
module_exit(fibkobj_exit);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Solomon Hsu <solnone@gmail.com>");

EXPORT_SYMBOL_GPL(get_fibonacci_type);
EXPORT_SYMBOL_GPL(notify_fibonacci_type_callback);