# Basic Character Device Driver

A simple Linux Character Device Driver implemented as a Loadable Kernel Module (LKM).

## Implemented APIs

### Driver Initialization

- alloc_chrdev_region()
- cdev_init()
- cdev_add()
- class_create()
- device_create()

### Driver Cleanup

- device_destroy()
- class_destroy()
- cdev_del()
- unregister_chrdev_region()

## Learning Outcomes

- Dynamic device number allocation
- Character device registration with VFS
- Device class creation under sysfs
- Automatic device node creation under /dev
- Driver cleanup and resource management

## Build

```bash
make
```

## Load Module

```bash
sudo insmod pcd.ko
```

## Unload Module

```bash
sudo rmmod pcd
```

## Check Kernel Logs

```bash
sudo dmesg
```

Kernel: Ubuntu 22.04 / Linux 6.x
