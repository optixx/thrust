
/* Simplified SDL2-based sound playback */

#include <SDL.h>
#include <stdlib.h>

#include "helpers.h"
#include "soundIt.h"
#include "sound_state.h"

static Uint8 mix_sample(channel_state_t* chan)
{
    if (chan->sample == NULL)
        return 128; /* silence in unsigned 8-bit */

    const Sample* s = chan->sample;
    if (chan->position >= s->len)
    {
        if (s->loop)
        {
            chan->position = 0;
        }
        else
        {
            chan->sample = NULL;
            chan->position = 0;
            return 128;
        }
    }

    return s->data[chan->position++];
}

static void audio_callback(void* userdata, Uint8* stream, int len)
{
    sound_state_t* state = (sound_state_t*)userdata;
    int samples = len;
    int i;

    while (samples--)
    {
        int mixed = 0;
        for (i = 0; i < state->num_channels; i++)
        {
            mixed += (int)mix_sample(&state->channels[i]) - 128;
        }
        mixed += 128;
        if (mixed < 0)
            mixed = 0;
        if (mixed > 255)
            mixed = 255;
        *stream++ = (Uint8)mixed;
    }
}

int Snd_init(int num_snd, const Sample* sa, int frequency, int channels)
{
    SDL_AudioSpec desired, obtained;

    if (sa == NULL || num_snd <= 0 || channels <= 0)
        return EXIT_FAILURE;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        return EXIT_FAILURE;

    SDL_zero(desired);
    desired.freq = frequency > 0 ? frequency : 11025;
    desired.format = AUDIO_U8;
    desired.channels = 1;
    desired.samples = 512;
    desired.callback = audio_callback;
    desired.userdata = NULL;

    sound_state_t* state = sound_state();

    state->channels = (channel_state_t*)calloc((size_t)channels, sizeof(channel_state_t));
    if (state->channels == NULL)
    {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return EXIT_FAILURE;
    }

    desired.userdata = state;

    state->audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (state->audio == 0)
    {
        free(state->channels);
        state->channels = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return EXIT_FAILURE;
    }

    state->sounds = sa;
    state->num_sounds = num_snd;
    state->num_channels = channels;

    SDL_PauseAudioDevice(state->audio, 0);
    return EXIT_SUCCESS;
}

int Snd_restore(void)
{
    sound_state_t* state = sound_state();

    if (state->audio != 0)
    {
        SDL_CloseAudioDevice(state->audio);
        state->audio = 0;
    }
    free(state->channels);
    state->channels = NULL;
    state->sounds = NULL;
    state->num_sounds = 0;
    state->num_channels = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return EXIT_SUCCESS;
}

int Snd_effect(int sound_num, int channel)
{
    sound_state_t* state = sound_state();

    if (state->audio == 0 || state->channels == NULL)
        return EXIT_FAILURE;
    if (sound_num < 0 || sound_num >= state->num_sounds)
        return EXIT_FAILURE;
    if (channel < 0 || channel >= state->num_channels)
        return EXIT_FAILURE;

    SDL_LockAudioDevice(state->audio);
    state->channels[channel].sample = &state->sounds[sound_num];
    state->channels[channel].position = 0;
    SDL_UnlockAudioDevice(state->audio);

    return EXIT_SUCCESS;
}

sound_state_t*
sound_state(void)
{
    static sound_state_t current_state = {0};
    return &current_state;
}
