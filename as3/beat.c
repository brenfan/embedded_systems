#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "audio_mixer.h"
#include "beat.h"

static pthread_t beatThread;

#define DEFAULT_BEAT STANDARD

static int current_beat = DEFAULT_BEAT;

#define HI_HAT 0x01
#define BASE 0x02
#define SNARE 0x4

static const int rock_pattern[] = {
	HI_HAT | BASE,
	HI_HAT,
	HI_HAT | SNARE,
	HI_HAT,
	HI_HAT | BASE,
	HI_HAT,
	HI_HAT | SNARE,
	HI_HAT
};

static const int custom_pattern[] = {
	SNARE | BASE,
	0,
	SNARE,
	0,
	BASE,
	BASE,
	HI_HAT | SNARE,
	0
};

static int pos = 0;
static int bpm = 100;
wavedata_t hi_hat;
wavedata_t snare;
wavedata_t base;

#define WAV_FILEPATH "beatbox-wav-files/"
#define HI_HAT_PATH "100053__menegass__gui-drum-cc.wav"
#define BASE_PATH "100051__menegass__gui-drum-bd-hard.wav"
#define SNARE_PATH "100059__menegass__gui-drum-snare-soft.wav"

void *beat_thread(void *arg);
static void queuesounds (int sounds);

void beat_init(void) {
	/* load rock sounds */
	AudioMixer_readWaveFileIntoMemory(WAV_FILEPATH HI_HAT_PATH, &hi_hat);
	AudioMixer_readWaveFileIntoMemory(WAV_FILEPATH SNARE_PATH, &snare);
	AudioMixer_readWaveFileIntoMemory(WAV_FILEPATH BASE_PATH, &base);
	pthread_create(&beatThread, NULL, beat_thread, NULL);
}

void *beat_thread(void *arg) {
	printf("init beatthread\n");
	do {
		if (current_beat == OFF)
			sleep (1);
		if (current_beat == STANDARD)
			beat_playsounds(rock_pattern[pos++]);
		if (current_beat == CUSTOM)
			beat_playsounds(custom_pattern[pos++]);
		pos = (pos == 8) ? 0 : pos;
		//printf("pos = %d\n", pos);
		usleep(1000000 * 60 / bpm / 2); // formula given in assignment notes
	} while (1);
}

void beat_set (int beat) {
	current_beat = beat;
	pos = 0;
}

void beat_playsounds (int sounds) {
	if (sounds & HI_HAT)
		AudioMixer_queueSound(&hi_hat);
	if (sounds & SNARE)
		AudioMixer_queueSound(&snare);
	if (sounds & BASE)
		AudioMixer_queueSound(&base);
}

void beat_volume_up(void) {
	AudioMixer_setVolume(AudioMixer_getVolume() + 5);
}


void beat_volume_down(void) {
	AudioMixer_setVolume(AudioMixer_getVolume() - 5);
}

void beat_tempo_up(void) {
	bpm += 5;
	bpm = (bpm > 200) ? 200 : bpm; //upper bpm limit
}

void beat_tempo_down(void) {
	bpm -= 5;
	bpm = (bpm < 40) ? 40 : bpm; // lower bpm limit
}

void beat_mode_cycle(void) {
	current_beat++;
	current_beat %= 3; //number of beats
}

void beat_set_volume(int x) {
	AudioMixer_setVolume(x);
}

int beat_get_volume(void) {
	return AudioMixer_getVolume();
}

int beat_get_tempo(void) {
	return bpm;
}

void beat_set_tempo(int x) {
	bpm = (x < 200) && (x > 40) ? x : bpm;
}

int beat_get_mode(void) {
	return current_beat;
}

void beat_mode_set(int mode) {
	current_beat = mode % 3; //number of beats
}
