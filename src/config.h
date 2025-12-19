/* src/config.h.  Generated automatically by configure.  */

/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef CONFIG_H
#define CONFIG_H
#undef HAVE_SOUND
#ifdef HAVE_SOUND
/* # undef HAVE_16BIT_SOUND */

/* # undef HAVE_LINUX_SOUND */
# ifdef HAVE_LINUX_SOUND
/* #  undef HAVE_LINUX_SOUNDCARD_H */
# endif /* HAVE_LINUX_SOUND */

/* # undef HAVE_SUN_SOUND */
# ifdef HAVE_SUN_SOUND
/* #  undef HAVE_SYS_AUDIOIO_H */
/* #  undef HAVE_SUN_AUDIOIO_H */
/* #  undef HAVE_SUN_SPEAKER */
/* #  undef HAVE_SUN_HEADPHONE */
/* #  undef HAVE_SUN_LINE_OUT */
# endif /* HAVE_SUN_SOUND */

/* # undef HAVE_HP_SOUND */
# ifdef HAVE_HP_SOUND
/* #  undef HAVE_SYS_AUDIO_H */
/* #  undef HAVE_HP_INTERNAL_SPEAKER */
/* #  undef HAVE_HP_EXTERNAL_SPEAKER */
/* #  undef HAVE_HP_LINE_OUT */
# endif /* HAVE_HP_SOUND */

#endif /* HAVE_SOUND */
#endif /* CONFIG_H */
