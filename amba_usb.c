/*
 * amba264 USB driver - 1.0
 *
 *     This file is licensed under the GPL. See COPYING in the package.
 * Based on usb-skeleton.c 2.0 by Greg Kroah-Hartman (greg@kroah.com)
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>


#define DRIVER_AUTHOR "Lechee.Lai"
#define DRIVER_DESC "Quanta AmbaA2 USB Driver"

/* Define these values to match your devices */
#define USB_AMBA_VENDOR_ID	0x4255
#define USB_AMBA_PRODUCT_ID	0x0003

/* table of devices that work with this driver */
static struct usb_device_id amba_table [] = {
    { USB_DEVICE(USB_AMBA_VENDOR_ID, USB_AMBA_PRODUCT_ID) },
    { }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, amba_table);

/* to prevent a race between open and disconnect */
static DEFINE_MUTEX(amba_open_lock);


/* Get a minor range for your devices from the usb maintainer */
#define USB_AMBA_MINOR_BASE	192

/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER		(PAGE_SIZE - 512)
/* MAX_TRANSFER is chosen so that the VM is not stressed by
   allocations > PAGE_SIZE and the number of packets in a page
   is an integer 512 is the largest possible packet on EHCI */
#define WRITES_IN_FLIGHT	8
/* arbitrarily chosen */

/* Structure to hold all of our device specific stuff */
struct usb_amba {
    struct usb_device		*udev;			/* the usb device for this device */
    struct usb_interface	*interface;		/* the interface for this device */
    struct semaphore		limit_sem;		/* limiting the number of writes in progress */
    unsigned char           	*bulk_in_buffer;	/* the buffer to receive data */
    size_t			bulk_in_size;		/* the size of the receive buffer */
    __u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
    __u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
    struct kref			kref;
    struct mutex		io_mutex;		/* synchronize I/O with disconnect */
};

typedef struct _encode_size {
	unsigned int h264_size;
	unsigned int mjpeg_size;
} encode_size;
#define to_amba_dev(d) container_of(d, struct usb_amba, kref)

static struct usb_driver amba_driver;

int amba_SND(struct usb_amba *ambaA2,
        __u8 request, __u8 requesttype,
        __u16 value, __u16 index, __u8 len)
{
    char *dummy_buffer = kzalloc(4, GFP_KERNEL);
    int result;
    if (!dummy_buffer)
        return -ENOMEM;
    result = usb_control_msg(ambaA2->udev,
            usb_sndctrlpipe(ambaA2->udev, 0),
            request, requesttype, value, index,
            dummy_buffer, len, 1000);
    kfree(dummy_buffer);
    return result;
}

static inline void amba_RCV(struct usb_amba *ambaA2,
        __u8 request, __u8 requesttype,
        __u16 value, __u16 index,
        char *buf, __u8 len)
{
    int result;
    result = usb_control_msg(ambaA2->udev,
            usb_rcvctrlpipe(ambaA2->udev, 0),
            request, requesttype, value, index,
            buf, len, 1000);
}

int cmd_set_encode_format(struct usb_amba *ambaA2, unsigned int encode_format)
{
    unsigned char buf[0x10];
    buf[0]=encode_format;
    int ret;
    ret = usb_control_msg(ambaA2->udev,
                          usb_sndctrlpipe(ambaA2->udev, 0),
                          0x01, 0x21, 0x0b00, 0xf000,
                          (unsigned char *)&encode_format, sizeof(encode_format),
                          1000);
    if (ret != 4) {
        info("AMBA(E)::cmd_set_encode_format()");
    }
    return ret;
}

int cmd_set_encode_size(struct usb_amba *ambaA2, unsigned int h264_size, unsigned int mjpeg_size)
{
    struct _encode_size 	es;
    int 			ret;

    es.h264_size = h264_size;
    es.mjpeg_size = mjpeg_size;
    ret = usb_control_msg(ambaA2->udev,
                          usb_sndctrlpipe(ambaA2->udev, 0),
                          0x01, 0x21, 0x1500, 0xf000,
                          (unsigned char *)&es, sizeof(encode_size),
                          1000);
    if (ret != sizeof(encode_size)) {
        info("AMBA(E)cmd_set_encode_size()");
    }
    return ret;
}

void cmd_set_append_header(struct usb_amba *ambaA2, unsigned int value)
{
    unsigned char buf[4];
    int ret;

    buf[0] = value & 0x0ff;
    buf[1] = (value >> 8) & 0x0ff;
    buf[2] = (value >> 16) & 0x0ff;
    buf[3] = (value >> 24) & 0x0ff;

    ret = usb_control_msg(ambaA2->udev,
                          usb_sndctrlpipe(ambaA2->udev, 0),
                          0x01, 0x21, 0x3900, 0xf000,
                          buf, sizeof(buf),
                          1000);
    if (ret != 4) {
        info("AMBA(E)::cmd_set_append_header()");
    }
}

void cmd_set_h264_bitrate(struct usb_amba *ambaA2, unsigned int value)
{

    char buf[4];
    int ret = 0;

    buf[0] = value & 0x0ff;
    buf[1] = (value >> 8) & 0x0ff;
    buf[2] = (value >> 16) & 0x0ff;
    buf[3] = (value >> 24) & 0x0ff;
    ret = usb_control_msg(ambaA2->udev,
                          usb_sndctrlpipe(ambaA2->udev, 0),
                          0x01, 0x21, 0x1000, 0xf000,
                          buf, sizeof(buf),
                          1000);
    if (ret != 4) {
        info("(AMBA(E)::cmd_set_h264_bitrate()");
    } else {

    }
}

static void amba_delete(struct kref *kref)
{
    struct usb_amba *dev = to_amba_dev(kref);

    usb_put_dev(dev->udev);
    kfree(dev->bulk_in_buffer);
    kfree(dev);
}

static int amba_open(struct inode *inode, struct file *file)
{
    struct usb_amba *dev;
    struct usb_interface *interface;
    int subminor;
    int retval = 0;

    subminor = iminor(inode);
//    info("AMBA(i):: Amba open");
    mutex_lock(&amba_open_lock);
    interface = usb_find_interface(&amba_driver, subminor);
    if (!interface) {
        mutex_unlock(&amba_open_lock);
        err ("%s - error, can't find device for minor %d",
                __FUNCTION__, subminor);
        retval = -ENODEV;
        goto exit;
    }

    dev = usb_get_intfdata(interface);
    if (!dev) {
        mutex_unlock(&amba_open_lock);
        retval = -ENODEV;
        goto exit;
    }

    /* increment our usage count for the device */
    kref_get(&dev->kref);
    /* now we can drop the lock */
    mutex_unlock(&amba_open_lock);

    if (retval) {
        kref_put(&dev->kref, amba_delete);
        goto exit;
    }

    /* save our object in the file's private structure */
    file->private_data = dev;

exit:
    return retval;
}

static int amba_release(struct inode *inode, struct file *file)
{
    struct usb_amba *dev;

    dev = (struct usb_amba *)file->private_data;
    if (dev == NULL)
        return -ENODEV;

    /* allow the device to be autosuspended */
    mutex_lock(&dev->io_mutex);
    mutex_unlock(&dev->io_mutex);

    /* decrement the count on our device */
    kref_put(&dev->kref, amba_delete);
    return 0;
}

static ssize_t amba_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
    struct usb_amba *dev;
    int retval;
    int bytes_read;
    char *AMBA_buf;

    AMBA_buf = kzalloc(128*1024, GFP_KERNEL);
    if (!AMBA_buf) {
        err("Out of memory 128K");
        return -ENOMEM;
    }

    dev = (struct usb_amba *)file->private_data;

    mutex_lock(&dev->io_mutex);
    if (!dev->interface) {		/* disconnect() was called */
        retval = -ENODEV;
        goto exit;
    }

    /* start Encode */
//    amba_SND(dev,0x01,0x21,0x0700,0xf000,4);

//    info("AMBA(i):: Amba bulk read");
    /* do a blocking bulk read to get data from the device */
    dev->bulk_in_endpointAddr = 0x81;
    dev->bulk_in_size = 128*1024;
    retval = usb_bulk_msg(dev->udev,
            usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
            AMBA_buf,
            min(dev->bulk_in_size, count),
            &bytes_read, 5000);

    /* if the read was successful, copy the data to userspace */
    if (!retval) {
        if (copy_to_user(buffer, AMBA_buf, bytes_read))
            retval = -EFAULT;
        else
            retval = bytes_read;
    }

    /* Stop Encode */
//    amba_SND(dev,0x01,0x21,0x0800,0xf000,4);

exit:
    kfree(AMBA_buf);
    mutex_unlock(&dev->io_mutex);
    return retval;

}

/**
 * amba_ioctl
 */
static int amba_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    struct usb_amba *dev;
    dev = (struct usb_amba *)file->private_data;

    switch (cmd) {
    case 0:
        amba_SND(dev,0x01,0x21,0x0700,0xf000,4);
        info("AMBA(I)::ioctl 0 START encode");
        break;
    case 1:
        amba_SND(dev,0x01,0x21,0x0800,0xf000,4);
        info("AMBA(I)::ioctl 1 STOP encode");
        break;
    default:
        return -1;
    }
    return 0;
}

static const struct file_operations amba_fops = {
    .owner =		THIS_MODULE,
    .read =		amba_read,
    .open =		amba_open,
    .ioctl =        	amba_ioctl,
    .release =		amba_release,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver amba_class = {
    .name =		"ambaA2",
    .fops =		&amba_fops,
    .minor_base =	USB_AMBA_MINOR_BASE,
};


/*
 * Initialize device controls.
 */
int amba_init_device(struct usb_amba *dev)
{
    char *buf = kzalloc(4, GFP_KERNEL);
    int retval;
    if (!buf)
        return -ENOMEM;
    info("AMBA(i):: Amba INIT USB device");
    /* GetChipVersion */
    amba_RCV(dev,0x81,0xA1,0x0300,0xf000,buf,4);
    /* GetFirmware */
    amba_RCV(dev,0x81,0xA1,0x0400,0xf000,buf,12);
    /* SetEncodeFormat */
    /* set encode format 0:H264 1:MJPEG 2:H264+MJPEG*/
    cmd_set_encode_format(dev,0);
    /* SetAppendHeader */
    cmd_set_append_header(dev,1);
    /* SelEncodeSize */
    /* set encode size H264 2:1280x720 3:640x480 10:253x288    MJPEG 0:640x480 5: 253x288 */
    cmd_set_encode_size(dev, 2, 5);
    /* SetBitRate */
    /* set H264 bitrate 1000Kbps */
    cmd_set_h264_bitrate(dev, 1000);
    /* SetFrameRate */
    retval=amba_SND(dev,0x01,0x21,0x0c00,0xf000,4);
    /* SetVinDevice */
    retval=amba_SND(dev,0x01,0x21,0x1600,0xf000,4);
    /* SetVoutDevice */
    retval=amba_SND(dev,0x01,0x21,0x1900,0xf000,4);
    /* BootDSP */
    retval=amba_SND(dev,0x01,0x21,0x0600,0xf000,0);
    /* SetPara */
    retval=amba_SND(dev,0x01,0x21,0x3100,0xf000,20);
    kfree(buf);
    return 0;
}

static int amba_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_amba *dev;
    int retval = -ENOMEM;

    if (interface->num_altsetting == 1) {
        info("AMBA(i)::Skip this device");
        return -1;
    }
    /* allocate memory for our device state and initialize it */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev) {
        err("Out of memory");
        goto error;
    }
    kref_init(&dev->kref);
    sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
    mutex_init(&dev->io_mutex);

    dev->udev = usb_get_dev(interface_to_usbdev(interface));
    dev->interface = interface;
    usb_set_interface(dev->udev,interface->altsetting->desc.bInterfaceNumber,1);

    /* save our data pointer in this interface device */
    usb_set_intfdata(interface, dev);

    /* Initialize controls */
    if (amba_init_device(dev) < 0) {
        err("Cound not init amba device");
        goto error;
    }

    /* we can register the device now, as it is ready */
    retval = usb_register_dev(interface, &amba_class);
    if (retval) {
        /* something prevented us from registering this driver */
        err("Not able to get a minor for this device.");
        usb_set_intfdata(interface, NULL);
        goto error;
    }

    /* let the user know what node this device is now attached to */
    info("ambaUSB device now attached to ambaUSB-%d", interface->minor);
    return 0;

error:
    if (dev)
        /* this frees allocated memory */
        kref_put(&dev->kref, amba_delete);
    return retval;
}

static void amba_disconnect(struct usb_interface *interface)
{
    struct usb_amba *dev;
    int minor = interface->minor;

    /* prevent amba_open() from racing amba_disconnect() */
    mutex_lock(&amba_open_lock);

    dev = usb_get_intfdata(interface);
    usb_set_intfdata(interface, NULL);


    /* give back our minor */
    usb_deregister_dev(interface, &amba_class);
    mutex_unlock(&amba_open_lock);

    /* prevent more I/O from starting */
    mutex_lock(&dev->io_mutex);
    dev->interface = NULL;
    mutex_unlock(&dev->io_mutex);



    /* decrement our usage count */
    kref_put(&dev->kref, amba_delete);

    info("ambaUSB #%d now disconnected", minor);
}

static struct usb_driver amba_driver = {
    .name =		"AMBA264",
    .probe =	amba_probe,
    .disconnect =	amba_disconnect,
    .id_table =	amba_table,
    //	.supports_autosuspend = 1,
};

static int __init usb_amba_init(void)
{
    int result;

    /* register this driver with the USB subsystem */
    result = usb_register(&amba_driver);
    if (result)
        err("usb_register failed. Error number %d", result);

    return result;
}

static void __exit usb_amba_exit(void)
{
    /* deregister this driver with the USB subsystem */
    usb_deregister(&amba_driver);
}

module_init(usb_amba_init);
module_exit(usb_amba_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
