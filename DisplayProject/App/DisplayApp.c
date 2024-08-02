#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include "DisplayDriver.h"

/*

 *   Check gpio app to check the circuit for pin 23 - see lab02
 *   pi@raspberrypi:~ $ sudo gpio -g mode 23 out
 *   pi@raspberrypi:~ $ sudo gpio -g write 23 1


 * Now use the circuit with the device driver
 pi@raspberrypi:~ $ sudo insmod DisplayDriver.ko
 pi@raspberrypi:~ $ sudo chmod 777 /dev/piiodev
 pi@raspberrypi:~ $ sudo chmod 755 ./piiotest

 pi@raspberrypi:~ $ ./piiotest writepin 23 1
 User App
 WRITE:Requested pin:23 - val:1 - desc:
 Exit 0
 pi@raspberrypi:~ $ ./piiotest writepin 23 0
 User App
 WRITE:Requested pin:23 - val:0 - desc:
 Exit 0

 * Toggle the pin 23 for 5 times with initial state 0 with 90000 microsecond delay
 pi@raspberrypi:~ $ ./piiotest toggle 23 1 5 90000
 pi@raspberrypi:~ $ ./piiotest setirq

 *   Verify below commands with dmesg
 pi@raspberrypi:~ $ ./piiotest writemsg
 pi@raspberrypi:~ $ ./piiotest readmsg
 */

gpio_pin stb;
gpio_pin clk;
gpio_pin dio;
gpio_pin deb;
lkm_data lkmdata;
int Counter;

void write_to_driver(int fd) {
	int ret;
	/* Write to kernel space - see dmesg command*/
	strcpy(lkmdata.data, "This is from user application");
	lkmdata.len = 32;
	lkmdata.type = 'w';
	ret = ioctl(fd, IOCTL_WRITE, &lkmdata);

	if (ret < 0) {
		printf("Function failed:%d\n", ret);
		exit(-1);
	}

}

void read_from_driver(int fd) {
	int ret;

	/*Read from kernel space - see dmesg command*/
	strcpy(lkmdata.data, "");
	ret = ioctl(fd, IOCTL_READ, &lkmdata);

	if (ret < 0) {
		printf("Function failed:%d\n", ret);
		exit(-1);
	}

	printf("Message from driver: %s\n", lkmdata.data);
}

int main(int argc, char *argv[]) {
	printf("User App\n");
	int fd, ret;
	char *msg = "Message passed by ioctl\n";
	
	fd = open("//dev//displaydriverdev", O_RDWR);
	if (fd < 0) {
		printf("Can't open device file: %s\n", DEVICE_NAME);
		exit(-1);
	}

	if (argc > 1) {
		if (!strncmp(argv[1], "readmsg", 8)) {
			read_from_driver(fd);
		}

		if (!strncmp(argv[1], "writemsg", 9)) {
			write_to_driver(fd);
		}

		if (!strncmp(argv[1], "readpin", 8)) {
			/*  Pass GPIO struct with IO control */
			memset(&stb, 0, sizeof(stb));
			strcpy(stb.desc, "Details");
			stb.pin = strtol(argv[2], NULL, 10);
			/* Pass 'stb' struct to 'fd' with IO control*/
			ret = ioctl(fd, IOCTL_GPIO_READ, &stb);
			printf("READ:Requested  pin:%i - val:%i - desc:%s\n", stb.pin,
					stb.value, stb.desc);
		}

		if (!strncmp(argv[1], "writepin", 9)) {
			/*  Pass GPIO struct with IO control */
			memset(&stb, 0, sizeof(stb));
			stb.pin = strtol(argv[2], NULL, 10);
			stb.value = strtol(argv[3], NULL, 10);
			/* Pass 'stb' struct to 'fd' with IO control*/
			ret = ioctl(fd, IOCTL_GPIO_WRITE, &stb);
			printf("WRITE:Requested pin:%i - val:%i - desc:%s\n", stb.pin,
					stb.value, stb.desc);
		}
		if (!strncmp(argv[1], "toggle", 7)) {
			strcpy(stb.desc, "desc");
			stb.pin = strtol(argv[2], NULL, 10);
			stb.value = strtol(argv[3], NULL, 10);
			int times = strtol(argv[4], NULL, 10);
			int delay = strtol(argv[5], NULL, 10);
			if (times >= 30 || times <= 0)
				times = 10;
			if (delay >= 100000 * 10 || delay <= 0)
				delay = 100000;
			for (; (times--);) {
				printf("TOGGLE:Requesting pin:%i - val:%i - desc:%s\n",
						stb.pin, stb.value, stb.desc);
				ret = ioctl(fd, IOCTL_GPIO_WRITE, &stb);
				stb.value = !stb.value;
				usleep(delay);
			}
		}
        if (!strncmp(argv[1], "writebyte", 10)) {
			/*  Pass GPIO struct with IO control */
			memset(&deb, 0, sizeof(deb));
			deb.pin = strtol(argv[2], NULL, 10);
			deb.value = strtol(argv[3], NULL, 10);
            // memset(&clk, 0, sizeof(clk));
            // memset(&dio, 0, sizeof(dio));
            // memset(&deb, 0, sizeof(deb));
            // char byte[]; strcpy(byte, argv[2]);
			// stb.pin = strtol(argv[3], NULL, 10);
            // clk.pin = strtol(argv[4], NULL, 10);
            // dio.pin = strtol(argv[5], NULL, 10);
            // deb.pin = strtol(argv[6], NULL, 10);
			/* Pass 'stb' struct to 'fd' with IO control*/
			ret = ioctl(fd, IOCTL_GPIO_WRITE, &deb);
			printf("WRITE:Requested pin:%i - val:%i - desc:%s\n", deb.pin,
            deb.value, deb.desc);
		}
		if (!strncmp(argv[1], "test", 5)) {
			printf("test command");
		}
	}

	printf("Exit 0\n");

	close(fd);
	return 0;
}
