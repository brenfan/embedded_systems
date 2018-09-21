/* Potentiometer */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define A2D_VIRTUAL_CAPE "/sys/devices/platform/bone_capemgr/slots"

#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

#define A2D_MAGIC_WORDS "BB-ADC"

static void write_file(char *filename, char *content) {
	FILE *f = fopen(filename, "w");
	fprintf(f, "%s", content);
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

void pot_init(void) {
	/* Enable A2D */
	write_file(A2D_VIRTUAL_CAPE, A2D_MAGIC_WORDS);
	sleep(1);
}

int pot_getVoltageReading(void) {
	return read_file(A2D_FILE_VOLTAGE0);
}
