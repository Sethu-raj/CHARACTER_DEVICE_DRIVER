# Linux Character Device Driver with Multiple Device Nodes

## Overview

This project implements a **Linux Character Device Driver** that supports multiple pseudo character devices using a single driver framework.

The driver creates four device nodes:

```bash
/dev/pcdev-1
/dev/pcdev-2
/dev/pcdev-3
/dev/pcdev-4
```

Each device has:

* Independent memory buffer
* Separate access permissions
* Unique minor number
* Common file operation implementation

The project demonstrates Linux Character Driver development, VFS integration, major/minor number handling, permission checking, and user-space to kernel-space communication.

---

## Features

* Dynamic device number allocation
* Character device registration with VFS
* Multiple device node support
* Independent memory regions for each device
* Permission-based access control
* open()
* read()
* write()
* llseek()
* release()
* Automatic device node creation
* Proper driver cleanup

---

## Device Permissions

| Device       | Permission   |
| ------------ | ------------ |
| /dev/pcdev-1 | Read Only    |
| /dev/pcdev-2 | Write Only   |
| /dev/pcdev-3 | Read / Write |
| /dev/pcdev-4 | Read / Write |

---

## Driver Architecture

```text
User Application
        |
        v
   /dev/pcdev-x
        |
        v
        VFS
        |
        v
 Character Driver
        |
        v
 Minor Number Check
        |
 -----------------------
 |     |     |      |
 v     v     v      v
PC1   PC2   PC3    PC4
```

The VFS forwards user requests to the driver. The driver identifies the target device using the minor number and routes the request to the appropriate device memory buffer.

---

## File Operations

### open()

* Identifies the device using minor number
* Verifies access permissions
* Stores device private data

### read()

Transfers data from kernel space to user space using:

```c
copy_to_user()
```

### write()

Transfers data from user space to kernel space using:

```c
copy_from_user()
```

### llseek()

Updates the file offset using:

```c
SEEK_SET
SEEK_CUR
SEEK_END
```

### release()

Called when the device is closed.

---

## Device Registration Flow

```text
alloc_chrdev_region()
          |
          v
      cdev_init()
          |
          v
       cdev_add()
          |
          v
     class_create()
          |
          v
     device_create()
          |
          v
      /dev/pcdev-x
```

---

## APIs Used

### Registration APIs

```c
alloc_chrdev_region()
cdev_init()
cdev_add()
class_create()
device_create()
```

### Cleanup APIs

```c
device_destroy()
class_destroy()
cdev_del()
unregister_chrdev_region()
```

---

## Build the Driver

Compile the module:

```bash
make
```

Generated output:

```bash
pcdrv.ko
```

---

## Load the Driver

```bash
sudo insmod pcdrv.ko
```

Verify:

```bash
dmesg
ls /dev/pcdev*
```

---

## Test Write Operation

```bash
echo "Hello Driver" > /dev/pcdev-3
```

---

## Test Read Operation

```bash
cat /dev/pcdev-3
```

---

## Unload the Driver

```bash
sudo rmmod pcdrv
```

Verify cleanup:

```bash
dmesg
```

---

## Learning Objectives

This project demonstrates:

* Linux Character Device Driver Development
* Virtual File System (VFS)
* Major and Minor Numbers
* Device Registration
* Device File Creation
* User Space ↔ Kernel Space Communication
* Multiple Device Node Management
* Permission Handling
* Driver Cleanup

---

## Summary

A single Linux character driver manages four independent pseudo devices. Each device maintains its own memory buffer and access permissions while sharing a common driver implementation. The design demonstrates how Linux device drivers use major/minor numbers, file operations, and private device data to efficiently manage multiple device instances.

## AUTHOR 
Sethu Raj
