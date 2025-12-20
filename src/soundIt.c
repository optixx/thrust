
/* Simplified SDL2-based sound playback */

#include <SDL.h>
#include <stdlib.h>

#include "soundIt.h"
#include "helpers.h"

typedef struct
{
    const Sample* sample;
    int position;
} ChannelState;

static const Sample* S_sounds = NULL;
static int S_num_sounds = 0;
static int S_num_channels = 0;
static SDL_AudioDeviceID S_audio = 0;
static ChannelState* S_channels = NULL;

static Uint8 mix_sample(ChannelState* chan)
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
    ChannelState* channels = (ChannelState*)userdata;
    int samples = len; /* AUDIO_U8 mono: 1 uint8_t per sample */
    int i;

    while (samples--)
    {
        int mixed = 0;
        for (i = 0; i < S_num_channels; i++)
        {
            mixed += (int)mix_sample(&channels[i]) - 128;
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
    desired.userdata = NULL; /* set below once channels are allocated */

    S_channels = (ChannelState*)calloc((size_t)channels, sizeof(ChannelState));
    if (S_channels == NULL)
    {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return EXIT_FAILURE;
    }

    desired.userdata = S_channels;

    S_audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (S_audio == 0)
    {
        free(S_channels);
        S_channels = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return EXIT_FAILURE;
    }

    S_sounds = sa;
    S_num_sounds = num_snd;
    S_num_channels = channels;

    SDL_PauseAudioDevice(S_audio, 0);
    return EXIT_SUCCESS;
}

int Snd_restore(void)
{
    if (S_audio != 0)
    {
        SDL_CloseAudioDevice(S_audio);
        S_audio = 0;
    }
    free(S_channels);
    S_channels = NULL;
    S_sounds = NULL;
    S_num_sounds = 0;
    S_num_channels = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return EXIT_SUCCESS;
}

int Snd_effect(int sound_num, int channel)
{
    if (S_audio == 0 || S_channels == NULL)
        return EXIT_FAILURE;
    if (sound_num < 0 || sound_num >= S_num_sounds)
        return EXIT_FAILURE;
    if (channel < 0 || channel >= S_num_channels)
        return EXIT_FAILURE;

    SDL_LockAudioDevice(S_audio);
    S_channels[channel].sample = &S_sounds[sound_num];
    S_channels[channel].position = 0;
    SDL_UnlockAudioDevice(S_audio);

    return EXIT_SUCCESS;
}
