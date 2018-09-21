#ifndef __BEAT_H__
#define __BEAT_H__

enum Beat {
	OFF = 0,
	STANDARD = 1,
	CUSTOM = 2
};

#define HI_HAT 0x01
#define BASE 0x02
#define SNARE 0x4

void beat_init(void);
void beat_destroy(void);

void beat_playsounds(int sounds);
void beat_set(int beat);
void beat_volume_up(void);
void beat_volume_down(void);
void beat_tempo_up(void);
void beat_tempo_down(void);

void beat_mode_cycle(void);

int beat_get_volume(void);

int beat_get_tempo(void);

int beat_get_mode(void);

void beat_mode_set(int mode);

#endif
