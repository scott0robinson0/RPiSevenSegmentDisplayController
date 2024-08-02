#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H


typedef struct lkm_data {
	unsigned char data[256];
	unsigned long len;
	char type;
} lkm_data;

typedef struct gpio_pin {
	char desc[16];
	unsigned int pin;
	int value;
	char opt;
} gpio_pin;

#define IOCTL_READ 0x65
#define IOCTL_WRITE 0x66
#define IOCTL_GPIO_READ 0x67
#define IOCTL_GPIO_WRITE 0x68


#define  DEVICE_NAME "displaydriverdev"
#define  CLASS_NAME  "displaydrivercls"

#endif
