#include <stdio.h>
#include <unistd.h>

#include "audio_mixer.h"
#include "beat.h"
#include "io.h"
#include "udp.h"

int main() {
	/* launch listeners for Zen cape inputs */

	/* launch audio playback thread */
	AudioMixer_init();
	beat_init();
	io_init();
	udp_init();
	/* launch udp interface */

	/* set up the drum beat */
	do {
		sleep(1);
	}
	while(1);

}
