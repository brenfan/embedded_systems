/* Driver for 14 seg display */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE_ADDRESS 0X20

#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define REGISTER_OUTPUT 0x00

#define IC2DRV_LINUX_BUS0 "/dev/i2c-0"
#define IC2DRV_LINUX_BUS1 "/dev/i2c-1"
#define IC2DRV_LINUX_BUS2 "/dev/i2c-2"

#define EXPORTS "/sys/class/gpio/export"

#define DIGIT1_GPIO 61
#define DIGIT2_GPIO 44


#define DIGIT1_VALUE "/sys/class/gpio/gpio61/value"
#define DIGIT2_VALUE "/sys/class/gpio/gpio44/value"

#define DIGIT1_DIRECTION "/sys/class/gpio/gpio61/direction"
#define DIGIT2_DIRECTION "/sys/class/gpio/gpio44/direction"

#define I2C_MAGICWORDS "BB-I2C1"
#define I2C_SLOT "/sys/devices/platform/bone_capemgr/slots"

#define OFF 0
#define ON 1

#define A_LITTLE_BIT 5000

/* I2C register patterns for digit display */
#define PATTERN_1A 0x80
#define PATTERN_1B 0x02
#define PATTERN_2A 0x31
#define PATTERN_2B 0x0e
#define PATTERN_3A 0xb0
#define PATTERN_3B 0x0e
#define PATTERN_4A 0x90
#define PATTERN_4B 0x8a
#define PATTERN_5A 0xb0
#define PATTERN_5B 0x8c
#define PATTERN_6A 0xb1
#define PATTERN_6B 0x8c
#define PATTERN_7A 0x80
#define PATTERN_7B 0x86
#define PATTERN_8A 0xb1
#define PATTERN_8B 0x8e
#define PATTERN_9A 0xb0
#define PATTERN_9B 0x8e
#define PATTERN_0A 0xa3
#define PATTERN_0B 0x96

const int patternsA[] = {
	PATTERN_0A,
	PATTERN_1A,
	PATTERN_2A,
	PATTERN_3A,
	PATTERN_4A,
	PATTERN_5A,
	PATTERN_6A,
	PATTERN_7A,
	PATTERN_8A,
	PATTERN_9A
};
const int patternsB[] = {
	PATTERN_0B,
	PATTERN_1B,
	PATTERN_2B,
	PATTERN_3B,
	PATTERN_4B,
	PATTERN_5B,
	PATTERN_6B,
	PATTERN_7B,
	PATTERN_8B,
	PATTERN_9B
};


static pthread_t thread;

static int i2cBus;
static int number_to_display;

static int left_segmentA = 0xFF;
static int left_segmentB = 0xFF;
static int right_segmentA = 0xFF;
static int right_segmentB = 0xFF;

static int running;

static void write_file(char *filename, char *content) {
	FILE *f = fopen(filename, "w");
	fprintf(f, "%s", content);
	fclose(f);
}

static void write_file_digit(char *filename, int content) {
	FILE *f = fopen(filename, "w");
	fprintf(f, "%d", content);
	fclose(f);
}

static int read_file(char *filename) {
	const int max_length = 8;
	char buff[max_length];

	FILE *f = fopen(filename, "r");
	fgets(buff, max_length, f);
	fclose(f);

	return atoi(buff);
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

static void update_segments(void) {
	/* set left_segments and right_segments here */
	int leftdigit = number_to_display / 10;
	int rightdigit = number_to_display % 10;

	left_segmentA = patternsA[leftdigit];
	left_segmentB = patternsB[leftdigit];

	right_segmentA = patternsA[rightdigit];
	right_segmentB = patternsB[rightdigit];
}

void display_setNumber(int n) {
	if (n <= 99 && n >= 0) {
		number_to_display = n;
		update_segments();
	}
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

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr) {
	// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("I2C: Unable to write to i2c register.");
		exit(1);
	}
	// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		perror("I2C: Unable to read from i2c register");
		exit(1);
	}
	return value;
}

static void *thread_fn(void *arg) {
	/* clear the display */
	write_file_digit(DIGIT1_VALUE, OFF);
	write_file_digit(DIGIT2_VALUE, OFF);

	do {
		/* write left digit */
		writeI2cReg(i2cBus, REG_OUTA, left_segmentA);
		writeI2cReg(i2cBus, REG_OUTB, left_segmentB);

		/* show to the digit */
		write_file_digit(DIGIT1_VALUE, ON);

		/* wait a little while */
		usleep(A_LITTLE_BIT);
		/* clear the display */
		write_file_digit(DIGIT1_VALUE, OFF);
		write_file_digit(DIGIT2_VALUE, OFF);


		/* write right digit */
		writeI2cReg(i2cBus, REG_OUTA, right_segmentA);
		writeI2cReg(i2cBus, REG_OUTB, right_segmentB);

		/* show to the digit */
		write_file_digit(DIGIT2_VALUE, ON);

		/* wait a little while */
		usleep(A_LITTLE_BIT);

		/* clear the display */
		write_file_digit(DIGIT1_VALUE, OFF);
		write_file_digit(DIGIT2_VALUE, OFF);

	} while(running);
}

void display_init(void) {
	/* exports */
	write_file_digit(EXPORTS, DIGIT1_GPIO);
	write_file_digit(EXPORTS, DIGIT2_GPIO);
	/* set direction */
	write_file(DIGIT1_DIRECTION, "out");
	write_file(DIGIT2_DIRECTION, "out");

	/* turn on the digits */
	write_file_digit(DIGIT1_VALUE, ON);
	write_file_digit(DIGIT2_VALUE, ON);

	/*enable /dev/i2c-1 */
	write_file(I2C_SLOT, I2C_MAGICWORDS);

	/* init the bus */
	i2cBus = init_I2cBus(IC2DRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

	usleep(A_LITTLE_BIT);
	writeI2cReg(i2cBus, REG_DIRA, REGISTER_OUTPUT);
	writeI2cReg(i2cBus, REG_DIRB, REGISTER_OUTPUT);

	/* launch thread */
	running = 1;
	pthread_create(&thread, NULL, thread_fn, NULL);
}

void display_destroy(void) {
	running = 0;
	pthread_join(thread, NULL);
	/* clear the registers */
	writeI2cReg(i2cBus, REG_DIRA, 0x00);
	writeI2cReg(i2cBus, REG_DIRA, 0x00);
	writeI2cReg(i2cBus, REG_OUTA, 0x00);
	writeI2cReg(i2cBus, REG_OUTB, 0x00);

	/* clear the display */
	write_file_digit(DIGIT1_VALUE, OFF);
	write_file_digit(DIGIT2_VALUE, OFF);

	close(i2cBus);
}
