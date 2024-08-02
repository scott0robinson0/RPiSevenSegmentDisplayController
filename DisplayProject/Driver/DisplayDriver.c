#include <linux/kernel.h>
#include <linux/module.h>
// #include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
// #include <linux/device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "DisplayDriver.h"

#define IOCTL_GPIO_READ 0x67
#define IOCTL_GPIO_WRITE 0x68

#define MAJOR_NUM 100
#define STB 14
#define CLK 15
#define DIO 18
#define DEB 26

static int DevBusy = 0;
static int MajorNum = 100;
static struct class*  ClassName  = NULL;
static struct device* DeviceName = NULL;
static char byte[256];
lkm_data lkmdata;
gpio_pin stb;
gpio_pin clk;
gpio_pin dio;
gpio_pin deb;

static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "DisplayDriver: device_open(%p)\n", file);

	if (DevBusy)
		return -EBUSY;

	DevBusy++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "DisplayDriver: device_release(%p)\n", file);
	DevBusy--;

	module_put(THIS_MODULE);
	return 0;
}

static int device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int i;
    bool bit;
	printk("DisplayDriver: device_ioctl - Device IOCTL invoked : 0x%x - %u\n" , cmd , cmd);

	switch (cmd) {
	case IOCTL_READ:
		strcpy(lkmdata.data ,"DisplayDriver: this is from dd\0");
		lkmdata.len = 101;
		lkmdata.type = 'r';

		copy_to_user((void *)arg, &lkmdata, sizeof(lkm_data));
		printk("DisplayDriver: device IOCTL read\n");
		break;

	case IOCTL_WRITE:
		printk("DisplayDriver: device IOCTL write\n");
		copy_from_user(&lkmdata, (lkm_data *)arg, sizeof(lkm_data));
		printk("DisplayDriver: from user: %s - %lu - %c\n" , lkmdata.data , lkmdata.len , lkmdata.type);
		break;

	case IOCTL_GPIO_READ:
		memset(&stb , 0, sizeof(stb));
		copy_from_user(&stb, (gpio_pin *)arg, sizeof(gpio_pin));
		gpio_request(stb.pin, stb.desc);
		stb.value = gpio_get_value(stb.pin);
		strcpy(stb.desc, "LKMpin");
		copy_to_user((void *)arg, &stb, sizeof(gpio_pin));
		printk("DisplayDriver: IOCTL_GPIO_READ - pin:%u - val:%i - desc:%s\n" , stb.pin , stb.value , stb.desc);
		break;
	case IOCTL_GPIO_WRITE:
		// copy_from_user(&stb, (gpio_pin *)arg, sizeof(gpio_pin));
        // copy_from_user(&clk, (gpio_pin *)arg, sizeof(gpio_pin));
        // copy_from_user(&dio, (gpio_pin *)arg, sizeof(gpio_pin));
        copy_from_user(&deb, (gpio_pin *)arg, sizeof(gpio_pin));
        
        stb.pin = 14;
        clk.pin = 15;
        dio.pin = 18;
        // deb.pin = 26;
        strcpy(stb.desc, "aa");
        strcpy(clk.desc, "bb");
        strcpy(dio.desc, "cc");
        // deb.desc = "d";

		gpio_request(stb.pin, stb.desc);
        gpio_request(clk.pin, stb.desc);
        gpio_request(dio.pin, stb.desc);
        gpio_request(deb.pin, stb.desc);

		gpio_direction_output(stb.pin, 1);
		gpio_direction_output(clk.pin, 0);
        gpio_direction_output(dio.pin, 0);
        gpio_direction_output(deb.pin, 0);
        gpio_set_value(deb.pin, deb.value);
        gpio_set_value(stb.pin, 0);

        for (i = strlen(byte) - 1; i >= 0; i--) {
            if (byte[i] == '0') {
                bit = 0;
            }
            else {
                bit = 1;
            }
            mdelay(1);
            gpio_set_value(dio.pin, bit);
            gpio_set_value(clk.pin, 1);
            mdelay(1);
            gpio_set_value(clk.pin, 0);

            // gpio_set_value(deb.pin, bit);
            // mdelay(10);
            // gpio_set_value(deb.pin, 0);
            // mdelay(10);
        }
        mdelay(1);
        gpio_set_value(stb.pin, 1);

		printk("DisplayDriver: IOCTL_GPIO_WRITE - pin:%u - val:%i - desc:%s\n" , deb.pin , deb.value , deb.desc);
		break;
	default:
			printk("DisplayDriver: format error\n");
	}

	return 0;
}

static void send_byte(char byte[]) {
    int i;
    bool bit;

    gpio_request(DEB, "Debug");
    gpio_direction_output(26, 0);
    gpio_request(STB, "Strobe");
    gpio_request(CLK, "Clock");
    gpio_request(DIO, "Data I/O");
    gpio_direction_output(STB, 1);
    gpio_direction_output(CLK, 0);
    gpio_direction_output(DIO, 0);
    gpio_set_value(STB, 0);

    for (i = strlen(byte) - 1; i >= 0; i--) {
        if (byte[i] == '0') {
            bit = 0;
        }
        else {
            bit = 1;
        }

        mdelay(1);
        gpio_set_value(DIO, bit);
        gpio_set_value(CLK, 1);
        mdelay(1);
        gpio_set_value(CLK, 0);

        gpio_set_value(DEB, bit);
        mdelay(10);
        gpio_set_value(DEB, 0);
        mdelay(10);
    }

    mdelay(1);
    gpio_set_value(STB, 1);
}

// static int lkm02_open(struct inode *inode, struct file *file){
//     pr_info("CMP408: %s\n", __func__);
//     return 0;
// }
// static int lkm02_release(struct inode *inode, struct file *file){
//     pr_info("CMP408: %s\n", __func__);
//     return 0;
// }
// static ssize_t lkm02_read(struct file *file,
//             char *buffer, size_t length, loff_t * offset){
//     pr_info("CMP408: %s %u\n", __func__, length);
//     return 0;
// }
// static ssize_t lkm02_write(struct file *file,
//              const char *buffer, size_t length, loff_t * offset){
//     pr_info("CMP408: %s %u\n", __func__, length);
//     return length;
// }

struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

static int __init DeviceDriver_init(void){
    char test[] = "10011001";
    send_byte(test);
    printk(KERN_INFO "DeviceDriver: initializing the dd\n");
    MajorNum = register_chrdev(0, DEVICE_NAME, &Fops);
        if (MajorNum<0){
            printk(KERN_ALERT "DeviceDriver: failed to register with major number\n");
            return MajorNum;
        }
    printk(KERN_INFO "DeviceDriver: registered with major number %d\n", MajorNum);


    ClassName = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ClassName)){
        unregister_chrdev(MajorNum, DEVICE_NAME);
        printk(KERN_ALERT "DeviceDriver: failed to register device class\n");
        return PTR_ERR(ClassName);
    }
    printk(KERN_INFO "DeviceDriver: device class registered\n");


    DeviceName = device_create(ClassName, NULL, MKDEV(MajorNum, 0), NULL, DEVICE_NAME);
    if (IS_ERR(DeviceName)){
        class_destroy(ClassName);
        unregister_chrdev(MajorNum, DEVICE_NAME);
        printk(KERN_ALERT "DeviceDriver: failed to create the device\n");
        return PTR_ERR(DeviceName);
    }
    printk(KERN_INFO "DeviceDriver: device class created\n");


	return 0;

}

// int __init lkm02_init(void){
//     // int ret;
//     char test[] = "10011001";
//     pr_info("CMP408: %s\n", __func__);

//     // ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &lkm02_fops);
//     // if (ret != 0)
//     //     return ret;
    
//     send_byte(test);
//     printk("CMP408: lkm02 loaded\n");
//     return 0;
// }

// void __exit lkm02_exit(void){
//     unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
//     gpio_set_value(CLK, 0);
//     gpio_free(CLK);
//     gpio_set_value(DIO, 0);
//     gpio_free(DIO);
//     gpio_set_value(STB, 0);
//     gpio_free(STB);
//     gpio_set_value(DEB, 0);
//     gpio_free(DEB);
//     printk("CMP408: lkm02 unloaded\n");
// }

static void __exit DeviceDriver_exit(void){
        device_destroy(ClassName, MKDEV(MajorNum, 0));
        class_unregister(ClassName);
        class_destroy(ClassName);
        unregister_chrdev(MajorNum, DEVICE_NAME);
        gpio_free(stb.pin);
        gpio_free(clk.pin);
        gpio_free(dio.pin);
        gpio_free(deb.pin);
        gpio_set_value(CLK, 0);
        gpio_free(CLK);
        gpio_set_value(DIO, 0);
        gpio_free(DIO);
        gpio_set_value(STB, 0);
        gpio_free(STB);
        gpio_set_value(DEB, 0);
        gpio_free(DEB);
        printk(KERN_INFO "DeviceDriver: Module removed\n");
}

module_init(DeviceDriver_init);
module_exit(DeviceDriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("AR CMP408");
MODULE_DESCRIPTION("RPi Zero W GPIO device driver with linux kernel GPIO library");
MODULE_VERSION("0.3");