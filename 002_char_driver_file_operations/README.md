# Character Driver with File Operations

## Overview

This project implements a Linux Character Device Driver as a Loadable Kernel Module (LKM).

The driver extends the basic character driver by implementing standard file operations and a pseudo device memory buffer.

## Features

* Dynamic device number allocation
* Character device registration using `cdev`
* Device node creation using `class_create()` and `device_create()`
* Pseudo device memory of 512 bytes
* Read operation support
* Write operation support
* File position handling
* `llseek()` implementation
* Open and release operations

## File Operations Implemented

### open()

Invoked when the device file is opened.

```c
.open = pcd_open
```

### release()

Invoked when the device file is closed.

```c
.release = pcd_release
```

### read()

Copies data from the kernel device buffer to user space using:

```c
copy_to_user()
```

### write()

Copies data from user space to the kernel device buffer using:

```c
copy_from_user()
```

### llseek()

Supports:

* SEEK_SET
* SEEK_CUR
* SEEK_END

and updates the file position accordingly.

## Device Memory

```c
#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
```

The driver uses a 512-byte pseudo device memory to store data written from user space.

## Kernel APIs Used

### Creation

```c
alloc_chrdev_region()
cdev_init()
cdev_add()
class_create()
device_create()
```

### Cleanup

```c
device_destroy()
class_destroy()
cdev_del()
unregister_chrdev_region()
```

## Build

```bash
make
```

## Load Driver

```bash
sudo insmod pcd.ko
```

## Verify

```bash
lsmod | grep pcd
sudo dmesg | tail
```

## Unload Driver

```bash
sudo rmmod pcd
```

## Learning Objectives

This project demonstrates:

* Character device driver development
* File operation implementation
* User space and kernel space data transfer
* File position management
* Device node creation through sysfs

## Author

Sethuraj
