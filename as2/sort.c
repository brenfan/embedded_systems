/* Thread for sorting arrays */
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "sort.h"
#include "pot.h"
#include "display.h"

/* length of array to sort */
static int length = 100;
static int running = 0;

static pthread_t thread;

static int arrays_sorted = 0;
static int *array;
pthread_mutex_t arraylock;

/* As specified in the Assignment Doc */
#define AMOUNT_OF_DATA_POINTS 10
int data_points[] = {0, 500, 1000, 1500, 2000, 2500, 3000, 2500, 4000, 4100};
int converted_points[] = {1, 20, 60, 120, 250, 300, 500, 800, 1200, 2100};

#define DISPLAY_LIMIT 99

int *sort_getArray(void) {
	return array;
}

pthread_mutex_t *sort_getLock(void) {
	return &arraylock;
}

int sort_getArrayIndex(int i) {
	if (i > 0 && i < length) {
		return array[i];
	}
	return -1;
}

int sort_getArraysSorted(void) {
	return arrays_sorted;
}

static int conversion_function(int x) {
	/* special case to make things easier */
	if (x == 0) {
		return 1;
	}

	/* uses piecewise linear formula */
	int i = 1;
	int last_point = 0;
	int last_data = 0;
	while (i < AMOUNT_OF_DATA_POINTS) {
		/* quik maths */
		if (x <= data_points[i]) {
			return round(last_point + (((x - last_data) *
				(converted_points[i] - last_point)) /
				(data_points[i] - last_data)));
		}
		last_data = data_points[i];
		last_point = converted_points[i];
		i++;
	}
	/* number out of range */
	printf("Warning: Attempted to convert input `%d`, out of range\n", x);
	return -1;
}

void sort_setArrayLength(int a) {
	if (a > 0) {
		length = a;
	}
	/* disallow negative numbers */
	return;
}

int sort_getArrayLength(void) {
	return length;
}

/* Return malloced array of specified size */
static int *init_array(int size) {
	int *arr;
	int i;

	arr = malloc(size * sizeof(int));
	i = size;
	while(i--) {
		arr[i] = i + 1;
	};

	return arr;
}

/* Shuffles contents of array */
static void shuffle_array(int *arr, int size) {
	int i;
	int r;
	int tmp;

	i = size;
	while (i--) {
		r = rand() % size;
		/* swap */
		tmp = arr[i];
		arr[i] = arr[r];
		arr[r] = tmp;
	}

	return;
}

/* The legendary Bubble sort Algorithm*/
static void bubble_sort(int *arr, int size) {
	int n;
	int flag; /* swap flag */

	n = length;
	do {
		flag = 0;
		int i;
		int tmp;

		i = 0;
		while (i < n - 1) {
			if (arr[i] > arr[i+1]) {
				/* critical section */
				pthread_mutex_lock(&arraylock);
				{
				tmp = arr[i];
				arr[i] = arr[i+1];
				arr[i+1] = tmp;
				}
				pthread_mutex_unlock(&arraylock);
				flag = 1; /* set flag */
			}
			i++;
		}
	} while (flag);
	return;
}

static void *sort_thread(void *arg) {
	int last_sorted = 0;
	int last_time = time(NULL);
	int now;
	length = conversion_function(pot_getVoltageReading());

	do {
		/* Initialize array */
		array = init_array(length);

		/* Shuffle Array */
		shuffle_array(array, length);

		/* Sort Array */
		bubble_sort(array, length);

		free(array);
		/* Update statistics */
		arrays_sorted ++;

		/* check time */
		now = time(NULL);
		if (now != last_time) {
			/* read the size to sort from potentiometer */
			int delta = arrays_sorted - last_sorted;
			delta = delta > DISPLAY_LIMIT ? DISPLAY_LIMIT : delta;
			printf("Size: %d\tSorted: %d\n",
				length, delta);
			last_sorted = arrays_sorted;
			length = conversion_function(pot_getVoltageReading());
			display_setNumber(delta);
			last_time = now;
		}
	} while (running);

	return NULL;
}

void sort_init(void) {
	pthread_mutex_init(&arraylock, NULL);
}

void sort_start(void) {
	running = 1;
	pthread_create(&thread, NULL, sort_thread, NULL);
}

void sort_stop(void) {
	running = 0;
	pthread_join(thread, NULL);
}

int sort_isRunning(void) {
	return running;
}

void sort_destroy() {
	pthread_mutex_destroy(&arraylock);
}
