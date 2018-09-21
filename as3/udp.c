/* Drive netcat service */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h> // for strncmp()
#include <unistd.h> // for close()
#include <pthread.h>

#include "udp.h"
#include "beat.h"

#define MSG_MAX_LEN 1024
#define PORT 12345

static pthread_t thread;
static struct sockaddr_in sin;

static int running;
static int killflag = 0;
static int sendarrayflag = 0;
static pthread_mutex_t *sortlock;

#define compare(x, y) strncmp(x, y, strlen(x)) == 0

static void killeverything(void) {
}


static void parse(char *message) {

	int i;
	if (compare("get beat", message)) {
		sprintf(message, "%d", beat_get_mode());
	}
	else if (compare("get volume", message)) {
		sprintf(message, "%d", beat_get_volume());
	}
	else if (compare("get tempo", message)) {
		sprintf(message, "%d", beat_get_tempo());
	}
	else if (compare("set beat", message)) {
		if (sscanf(message + strlen("set beat "), "%d", &i) != 1) {
			sprintf(message,
				"Invalid argument. Must be int.\n");
			return;
		}

		beat_mode_set(i);
	}
	else if (compare("set volume", message)) {
		if (sscanf(message + strlen("set beat "), "%d", &i) != 1) {
			sprintf(message,
				"Invalid argument. Must be int.\n");
			return;
		}

		beat_mode_set(i);
	}
	else if (compare("playsound ", message)) {
		if (sscanf(message + strlen("playsound "), "%d", &i) != 1) {
			sprintf(message,
				"Invalid argument. Must be int.\n");
			return;
		}

		beat_playsounds(i);
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


		sin_len = sizeof(sin);
		sendto( socketDescriptor, message, strlen(message), 0,
			(struct sockaddr *) &sin, sin_len);

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

	running = 1;
	pthread_create(&thread, NULL, thread_fn, NULL);
}

void udp_destroy(void) {
	running = 0;
	pthread_join(thread, NULL);
}
