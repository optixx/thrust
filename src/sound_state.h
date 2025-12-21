#ifndef SOUND_STATE_H
#define SOUND_STATE_H

#include <SDL.h>

typedef struct
{
    const Sample* sample;
    int position;
} channel_state_t;

typedef struct
{
    const Sample* sounds;
    int num_sounds;
    int num_channels;
    SDL_AudioDeviceID audio;
    channel_state_t* channels;
} sound_state_t;

sound_state_t* sound_state(void);

#endif /* SOUND_STATE_H */
