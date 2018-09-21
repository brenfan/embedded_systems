/* Controls LEDs and Joystick IO */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "io.h"
#include "beat.h"

#define EXPORTS "/sys/class/gpio/export"

#define IC2DRV_LINUX_BUS0 "/dev/i2c-0"
#define IC2DRV_LINUX_BUS1 "/dev/i2c-1"
#define IC2DRV_LINUX_BUS2 "/dev/i2c-2"

#define I2C_DEVICE_ADDRESS 0X1C

#define I2C_MAGICWORDS "BB-I2C1"
#define I2C_SLOT "/sys/devices/platform/bone_capemgr/slots"

#define REG_STATUS 0x00
#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06

#define CTRL_REG1 0x2A

#define X_THRESHOLD 3000
#define Y_THRESHOLD 3000
#define Z_THRESHOLD 3000

static int i2cBus;

#define GPIO_JOY_UP 26
#define GPIO_JOY_DOWN 46
#define GPIO_JOY_LEFT 65
#define GPIO_JOY_RIGHT 47
#define GPIO_JOY_IN 27

#define JOY_UP_DIRECTION "/sys/class/gpio/gpio26/direction"
#define JOY_DOWN_DIRECTION "/sys/class/gpio/gpio46/direction"
#define JOY_LEFT_DIRECTION "/sys/class/gpio/gpio65/direction"
#define JOY_RIGHT_DIRECTION "/sys/class/gpio/gpio47/direction"
#define JOY_IN_DIRECTION "/sys/class/gpio/gpio27/direction"

#define JOY_UP_VALUE "/sys/class/gpio/gpio26/value"
#define JOY_DOWN_VALUE "/sys/class/gpio/gpio46/value"
#define JOY_LEFT_VALUE "/sys/class/gpio/gpio65/value"
#define JOY_RIGHT_VALUE "/sys/class/gpio/gpio47/value"
#define JOY_IN_VALUE "/sys/class/gpio/gpio27/value"

#define POLL_SPEED_DELAY 100000 /* 0.1 seconds */

static pthread_t thread;

static void init_accelerometer(void);
static void *thread_fn(void *arg);

static void write_file(char *filename, char *content) {
	FILE *f = fopen(filename, "w");
	fprintf(f, "%s", content);
	fclose(f);
}

static int init_I2cBus(char *bus, int address) {
	/* shamelessly stolen from the I2CGuide.pdf */

	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc,
	unsigned char regAddr, unsigned char value) {
		unsigned char buff[2];
		buff[0] = regAddr;
		buff[1] = value;
		int res = write (i2cFileDesc, buff, 2);
		if (res != 2) {
			perror("I2C: Unable to write i2c register.");
			exit (1);
		}
	}

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char *buff, int size) {
		// To read a register, must first write the address
		int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
		if (res != sizeof(regAddr)) {
			perror("I2C: Unable to write to i2c register.");
			exit(1);
		}
		// Now read the value and return it
		char value = 0;
		res = read(i2cFileDesc, buff, size);
		if (res != size) {
			perror("I2C: Unable to read from i2c register");
			exit(1);
		}
		return value;
	}


static void init_accelerometer(void) {

		/*enable /dev/i2c-1 */
		write_file(I2C_SLOT, I2C_MAGICWORDS);

		/* init the bus */
		i2cBus = init_I2cBus(IC2DRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

		writeI2cReg(i2cBus, CTRL_REG1, 0x01); // set active bit
		usleep(POLL_SPEED_DELAY);

	}

static void write_gpio(char *filename, char *payload) {
	FILE *file = fopen(filename, "w");
	if (file == NULL) {
		printf("ERROR: Unable to open `%s` for read\n", filename);
		exit(-1);
	}
	fprintf(file, "%s", payload);
	fclose(file);
}

static int read_gpio(char *filename) {
	const int max_length = 16;
	char buff[max_length];

	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Error reading file `%s`\n", filename);
		exit(1);
	}
	fgets(buff, max_length, file);
	fclose(file);

	return atoi(buff);
}

static void init_gpio() {
	FILE *export = fopen(EXPORTS, "w");
	if (export == NULL) {
		printf("ERROR: Unable to open export file.\n");
		exit(-1);
	}

	fprintf(export, "%d", GPIO_JOY_UP);
	fflush(NULL);
	fprintf(export, "%d", GPIO_JOY_DOWN);
	fflush(NULL);
	fprintf(export, "%d", GPIO_JOY_LEFT);
	fflush(NULL);
	fprintf(export, "%d", GPIO_JOY_RIGHT);
	fflush(NULL);
	fprintf(export, "%d", GPIO_JOY_IN);

	fclose(export);
}

static void init_joystick() {
	write_gpio(JOY_UP_DIRECTION, "in");
	write_gpio(JOY_DOWN_DIRECTION, "in");
	write_gpio(JOY_LEFT_DIRECTION, "in");
	write_gpio(JOY_RIGHT_DIRECTION, "in");
	write_gpio(JOY_IN_DIRECTION, "in");

}

void io_init(void) {
	init_gpio();
	init_joystick();
	init_accelerometer();

	pthread_create(&thread, NULL, thread_fn, NULL);
}

static int getJoystickInput() {
	if (read_gpio(JOY_UP_VALUE) == 0)
		return UP;
	if (read_gpio(JOY_DOWN_VALUE) == 0)
		return DOWN;
	if (read_gpio(JOY_LEFT_VALUE) == 0)
		return LEFT;
	if (read_gpio(JOY_RIGHT_VALUE) == 0)
		return RIGHT;
	if (read_gpio(JOY_IN_VALUE) == 0)
		return IN;
	return -1;
}

static int anyJoystickIsPressed(){
	if (read_gpio(JOY_UP_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_DOWN_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_LEFT_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_RIGHT_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_IN_VALUE) == 0)
		return 1;
	return 0;
}

static void read_accel(void) {
	unsigned char buff[7];
	//read the first 7 registers
	readI2cReg(i2cBus, REG_STATUS, buff, 7);

	int16_t x = (buff[OUT_X_MSB] << 8) | (buff[OUT_X_LSB]);
	int16_t y = (buff[OUT_Y_MSB] << 8) | (buff[OUT_Y_LSB]);
	int16_t z = (buff[OUT_Z_MSB] << 8) | (buff[OUT_Z_LSB]);

	z -= 16000; // compensate for gravity

	x = x < 0 ? -x : x;
	y = y < 0 ? -y : y;
	z = z < 0 ? -z : z; // make not negative

	//printf("x %d, y %d, z %d\n", x, y ,z);
	if (x > X_THRESHOLD) {
		beat_playsounds(BASE);
	}
	if (y > Y_THRESHOLD) {
		beat_playsounds(SNARE);
	}
	if (z > Z_THRESHOLD) {
		beat_playsounds(HI_HAT);
	}
}

static void *thread_fn(void *arg) {
	do {
		int response = getJoystickInput();

		switch (response) {
			case RIGHT:
			beat_tempo_up();
			break;
			case LEFT:
			beat_tempo_down();
			break;
			case UP:
			beat_volume_up();
			break;
			case DOWN:
			beat_volume_down();
			break;
			case IN:
			beat_mode_cycle();
			break;
		}

		read_accel();
		usleep(POLL_SPEED_DELAY);
	} while (1);
}
