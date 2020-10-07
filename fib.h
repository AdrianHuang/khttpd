#ifndef FIB_H
#define FIB_H

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

char *fib_sequence(unsigned int k);

#endif
