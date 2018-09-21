/* Drive netcat service */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h> // for strncmp()
#include <unistd.h> // for close()
#include <pthread.h>

#include "udp.h"
#include "sort.h"

#define MSG_MAX_LEN 1024
#define PORT 12345

static pthread_t thread;
static struct sockaddr_in sin;

static int running;
static int killflag = 0;
static int sendarrayflag = 0;
static pthread_mutex_t *sortlock;

static void killeverything(void) {
	sort_stop();
}

static void parse(char *message) {
	if (strncmp("help", message, strlen("help")) == 0) {
		sprintf(message,
		"Accepted command examples:\n"
		"count\t-- display number arrays sorted.\n"
		"get length\t-- display length of array current being sorted.\n"
		"get array\t-- display the full array being sorted.\n"
		"get 10\t-- display the tenth element of array currently being sorted.\n"
		"stop\t-- cause the server program to end.\n");
	}
	else if (strncmp("count", message, strlen("count")) == 0) {
		sprintf(message, "Number of arrays sorted = %d.\n",
			sort_getArraysSorted());
	}
	else if (strncmp("stop", message, strlen("stop")) == 0) {
		/* ABANDON SHIP */
		sprintf(message, "Program Terminating\n");
		killflag = 1;
	}
	else if (strncmp("get array", message, strlen("get array")) == 0) {
		sendarrayflag = 1;
	}
	else if (strncmp("get length", message, strlen("get length")) == 0) {
		sprintf(message, "Current array length = %d.\n",
			sort_getArrayLength());
	}
	else if(strncmp("get ", message, strlen("get ")) == 0) {
		int index;
		if ((sscanf(message + strlen("get "), "%d", &index) != 1) ||
		 (index > sort_getArrayLength() || index < 1)) {
			sprintf(message,
				"Invalid argument. Must be between 1 and %d (array length).\n",
				sort_getArrayLength());
			return;
		}

		/* arrays start at 0 */
		sprintf(message, "Value %d = %d\n", index, sort_getArrayIndex(index - 1));
	}
	else {
		sprintf(message, "Command not recognized.\n");
	}
}

static void *thread_fn(void * arg) {
	// Buffer to hold packet data:
	char message[MSG_MAX_LEN];

	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	do {
		unsigned int sin_len = sizeof(sin);
		int bytesRx = recvfrom(socketDescriptor, message, MSG_MAX_LEN,
			0, (struct sockaddr *) &sin, &sin_len);
		if (bytesRx < 0) {
			continue;
		}

		message[bytesRx] = '\0';

		parse(message);

		if (sendarrayflag) {
			pthread_mutex_lock(sortlock);
			{
			int length = sort_getArrayLength();
			int *array = sort_getArray();
			int offset = 0;
			int i= 0;
			while (i < length) {
				/* limit 100 numbers per packet */
				offset += sprintf(message + offset, "%d, ",
					array[i]);
				if ((i + 1) % 10 == 0) {
					offset += sprintf(message + offset, "\n");
				}
				if (i % 100 == 0) {
					sin_len = sizeof(sin);
					sendto( socketDescriptor, message, strlen(message), 0,
						(struct sockaddr *) &sin, sin_len);
					offset = 0;
				}
				i++;
			}
			sprintf(message + offset - 2, "\n");
			sin_len = sizeof(sin);
			sendto( socketDescriptor, message, strlen(message), 0,
				(struct sockaddr *) &sin, sin_len);
			} /* end critical section */
			pthread_mutex_unlock(sortlock);
			sendarrayflag = 0;
		} else {
			sin_len = sizeof(sin);
			sendto( socketDescriptor, message, strlen(message), 0,
				(struct sockaddr *) &sin, sin_len);
		}

		if (killflag) {
			killeverything();
			running = 0;
		}
	} while(running);

	close(socketDescriptor);

	return NULL;
}

void udp_init(void) {
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET; // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to Network long
	sin.sin_port = htons(PORT); // Host to Network short

	sortlock = sort_getLock();
	running = 1;
	pthread_create(&thread, NULL, thread_fn, NULL);
}

void udp_destroy(void) {
	running = 0;
	pthread_join(thread, NULL);
}
