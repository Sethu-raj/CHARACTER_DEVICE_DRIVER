#include <linux/module.h>
#include  <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

#define NO_OF_DEVICES 4
#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 1024
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 1024

//READ AND WRITE OPERATION CARRIED OUT THIS MEMORY POOL
/*PSEUDO DEVICE MEMORY*/
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1]; //MEMORY
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];


/*Device private data structure */
struct pcdev_private_data
{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev cdev;
};

/*Driver private data structure */
struct pcdrv_private_data
{
    int total_devices;
    /*this holds the device number */
    dev_t device_number;
    struct  class *class_pcd;
    struct device *device_pcd;

    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data =
{
    .total_devices = NO_OF_DEVICES,

    .pcdev_data = {

        [0] = {
            .buffer = device_buffer_pcdev1,
            .size = MEM_SIZE_MAX_PCDEV1,
            .serial_number = "PCDEV1XYZ123",
            .perm = 0x1
        },

        [1] = {
            .buffer = device_buffer_pcdev2,
            .size = MEM_SIZE_MAX_PCDEV2,
            .serial_number = "PCDEV2XYZ123",
            .perm = 0x10
        },

        [2] = {
            .buffer = device_buffer_pcdev3,
            .size = MEM_SIZE_MAX_PCDEV3,
            .serial_number = "PCDEV3XYZ123",
            .perm = 0x11
        },

        [3] = {
            .buffer = device_buffer_pcdev4,
            .size = MEM_SIZE_MAX_PCDEV4,
            .serial_number = "PCDEV4XYZ123",
            .perm = 0x11
        }
    }
};




loff_t pcd_lseek(struct file *filp,
                 loff_t offset,
                 int whence)
{
    loff_t temp;
    struct pcdev_private_data *pcdev_data;

    pcdev_data = (struct pcdev_private_data *)filp->private_data;

    pr_info("lseek requested\n");
    pr_info("Current file position = %lld\n", filp->f_pos);

    switch (whence)
    {
        case SEEK_SET:
            if ((offset > pcdev_data->size) || (offset < 0))
                return -EINVAL;

            filp->f_pos = offset;
            break;

        case SEEK_CUR:
            temp = filp->f_pos + offset;

            if ((temp > pcdev_data->size) || (temp < 0))
                return -EINVAL;

            filp->f_pos = temp;
            break;

        case SEEK_END:
            temp = pcdev_data->size + offset;

            if ((temp > pcdev_data->size) || (temp < 0))
                return -EINVAL;

            filp->f_pos = temp;
            break;

        default:
            return -EINVAL;
    }

    pr_info("Updated file position = %lld\n", filp->f_pos);

    return filp->f_pos;
}




ssize_t pcd_read(struct file *filp,
                 char __user *buff,
                 size_t count,
                 loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data;

    pcdev_data = (struct pcdev_private_data *)filp->private_data;

    pr_info("Read requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /* Adjust count */
    if ((*f_pos + count) > pcdev_data->size)
        count = pcdev_data->size - *f_pos;

    /* End of device memory */
    if (!count)
        return 0;

    /* Copy to user */
    if (copy_to_user(buff,
                     pcdev_data->buffer + (*f_pos),
                     count))
    {
        return -EFAULT;
    }

    /* Update file position */
    *f_pos += count;

    pr_info("Number of bytes successfully read = %zu\n", count);
    pr_info("Updated file position = %lld\n", *f_pos);

    return count;
}

ssize_t pcd_write(struct file *filp,
                  const char __user *buff,
                  size_t count,
                  loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data;

    pcdev_data = (struct pcdev_private_data *)filp->private_data;

    pr_info("Write requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /* Adjust count */
    if ((*f_pos + count) > pcdev_data->size)
        count = pcdev_data->size - *f_pos;

    if (!count) {
        pr_err("No space left on device\n");
        return -ENOMEM;
    }

    /* Copy from user */
    if (copy_from_user(pcdev_data->buffer + (*f_pos),
                       buff,
                       count))
    {
        return -EFAULT;
    }

    /* Update file position */
    *f_pos += count;

    pr_info("Number of bytes successfully written = %zu\n", count);
    pr_info("Updated file position = %lld\n", *f_pos);

    return count;
}

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR   0x11
int check_permission(int dev_perm, int acc_mode)
{
    if (dev_perm == RDWR)
        return 0;

    if ((dev_perm == RDONLY) &&
        ((acc_mode & FMODE_READ) &&
         !(acc_mode & FMODE_WRITE)))
        return 0;

    if ((dev_perm == WRONLY) &&
        ((acc_mode & FMODE_WRITE) &&
         !(acc_mode & FMODE_READ)))
        return 0;

    return -EPERM;

}
int pcd_open(struct inode *inode, struct file *filp)
{
    int ret;
    int minor_n;

    struct pcdev_private_data *pcdev_data;

    /* Find which device is opened */
    minor_n = MINOR(inode->i_rdev);
    pr_info("minor access = %d\n", minor_n);

    /* Get device private data */
    pcdev_data = container_of(inode->i_cdev,
                              struct pcdev_private_data,
                              cdev);

    /* Save private data */
    filp->private_data = pcdev_data;

    /* Permission check */
    ret = check_permission(pcdev_data->perm,
                           filp->f_mode);

    if (!ret)
        pr_info("open was successful\n");
    else
        pr_info("open was unsuccessful\n");

    return ret;
}

int pcd_release(struct inode *inode, struct file *filp)
{
pr_info("release was successful\n");

    return 0;
}



/*fileoperations of the driver*/
struct file_operations pcd_fops =
{
    .open    = pcd_open,
    .read    = pcd_read,
    .write   = pcd_write,
    .llseek  = pcd_lseek,
    .release = pcd_release,
};

static int __init pcd_driver_init(void)
{
    int ret;
    int i;

    /* Allocate device numbers */
    ret = alloc_chrdev_region(&pcdrv_data.device_number,
                              0,
                              NO_OF_DEVICES,
                              "pcdevs");

    if (ret < 0) {
        pr_err("alloc_chrdev_region failed\n");
        return ret;
    }

    /* Create class */
    pcdrv_data.class_pcd = class_create("pcd_class");

    if (IS_ERR(pcdrv_data.class_pcd)) {
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto unreg_chrdev;
    }

    /* Create devices */
    for (i = 0; i < NO_OF_DEVICES; i++) {

      pr_info("Device %d ==> Major:%d Minor:%d\n",
            i + 1,
            MAJOR(pcdrv_data.device_number + i),
            MINOR(pcdrv_data.device_number + i)); 


       cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;

        ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev,
                       pcdrv_data.device_number + i,
                       1);

        if (ret < 0)
            goto cdev_del;

        device_create(pcdrv_data.class_pcd,
                      NULL,
                      pcdrv_data.device_number + i,
                      NULL,
                      "pcdev-%d",
                      i + 1);
    }

    pr_info("Module loaded successfully\n");
    return 0;

cdev_del:

    while (--i >= 0) {
        device_destroy(pcdrv_data.class_pcd,
                       pcdrv_data.device_number + i);

        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }

    class_destroy(pcdrv_data.class_pcd);

unreg_chrdev:
    unregister_chrdev_region(pcdrv_data.device_number,
                             NO_OF_DEVICES);

    return ret;
}


static void __exit pcd_driver_cleanup(void)
{
    int i;

    for (i = 0; i < NO_OF_DEVICES; i++) {

        device_destroy(pcdrv_data.class_pcd,
                       pcdrv_data.device_number + i);

        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }

    class_destroy(pcdrv_data.class_pcd);

    unregister_chrdev_region(pcdrv_data.device_number,
                             NO_OF_DEVICES);

    pr_info("Module unloaded\n");
}





module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SETHU");
MODULE_DESCRIPTION("PSEUDO CHARACTER DRIVER WHICH HANDLE N DEVICES");
