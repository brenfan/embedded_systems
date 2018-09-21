#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sort.h"
#include "display.h"
#include "pot.h"
#include "udp.h"

int main() {
	/* Init potentiometer */
	pot_init();

	/* Start array sort thread */
	sort_init();
	sort_start();
	/* Init UDP socket for host */
	udp_init();
	/* Init display */
	display_init();
	/* Wait for threads to close */

	while (sort_isRunning()) {
		sleep(1);
	}

	display_destroy();
	udp_destroy();
	sort_destroy();

	return 0;
}
