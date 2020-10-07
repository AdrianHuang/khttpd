#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#include "fib.h"
#include "xs.h"

#define XOR_SWAP(a, b, type) \
    do {                     \
        type *__c = (a);     \
        type *__d = (b);     \
        *__c ^= *__d;        \
        *__d ^= *__c;        \
        *__c ^= *__d;        \
    } while (0)

static void __swap(void *a, void *b, size_t size)
{
    if (a == b)
        return;

    switch (size) {
    case 1:
        XOR_SWAP(a, b, char);
        break;
    case 2:
        XOR_SWAP(a, b, short);
        break;
    case 4:
        XOR_SWAP(a, b, unsigned int);
        break;
    case 8:
        XOR_SWAP(a, b, unsigned long);
        break;
    default:
        /* Do nothing */
        break;
    }
}

static void reverse_str(char *str, size_t n)
{
    int i;
    for (i = 0; i < (n >> 1); i++)
        __swap(&str[i], &str[n - i - 1], sizeof(char));
}

static int string_number_add(xs *a, xs *b, xs *out)
{
    char *data_a, *data_b, *buf;
    size_t size_a, size_b;
    int i, carry = 0;
    int sum;

    /*
     * Make sure the string length of 'a' is always greater than
     * the one of 'b'.
     */
    if (xs_size(a) < xs_size(b))
        __swap((void *) &a, (void *) &b, sizeof(void *));

    data_a = xs_data(a);
    data_b = xs_data(b);

    size_a = xs_size(a);
    size_b = xs_size(b);

    reverse_str(data_a, size_a);
    reverse_str(data_b, size_b);

    buf = kmalloc(size_a + 2, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    for (i = 0; i < size_b; i++) {
        sum = (data_a[i] - '0') + (data_b[i] - '0') + carry;
        buf[i] = '0' + sum % 10;
        carry = sum / 10;
    }

    for (i = size_b; i < size_a; i++) {
        sum = (data_a[i] - '0') + carry;
        buf[i] = '0' + sum % 10;
        carry = sum / 10;
    }

    if (carry)
        buf[i++] = '0' + carry;

    buf[i] = 0;

    reverse_str(buf, i);

    /* Restore the original string */
    reverse_str(data_a, size_a);
    reverse_str(data_b, size_b);

    if (out)
        *out = *xs_tmp(buf);

    kfree(buf);

    return 0;
}

char *fib_sequence(unsigned int k)
{
    char *buf;
    int i, n;
    xs *f;

    f = kmalloc(sizeof(*f) * (k + 2), GFP_KERNEL);
    if (!f) {
        printk("kmalloc for 'xs' object failed!\n");
        return NULL;
    }

    f[0] = *xs_tmp("0");
    f[1] = *xs_tmp("1");

    for (i = 2; i <= k; i++)
        if (string_number_add(&f[i - 1], &f[i - 2], &f[i]))
            goto out;

    n = xs_size(&f[k]);

    buf = kmalloc(n + 1, GFP_KERNEL);
    if (!buf) {
        printk("kmalloc failed!\n");
        goto out;
    }

    strncpy(buf, xs_data(&f[k]), n);
    buf[n] = 0;

out:
    for (i = 0; i <= k; i++)
        xs_free(&f[i]);

    kfree(f);

    return buf;
}
