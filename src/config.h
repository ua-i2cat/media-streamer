/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Specifies ABI version for audio capture */
#define AUDIO_CAPTURE_ABI_VERSION 3

/* Specifies ABI version for audio playback */
#define AUDIO_PLAYBACK_ABI_VERSION 3

/* Autoconf result */
#define AUTOCONF_RESULT "  Target ...................... i686-pc-linux-gnu\n  Debug output ................ no\n  Profiling support ........... no\n  IPv6 support ................ yes\n  RT priority ................. no\n  Standalone modules .......... no\n\n  DeckLink .................... yes\n  DELTACAST ................... no\n  DVS ......................... no\n  Linsys capture .............. yes\n  OpenGL ...................... yes\n  Quicktime ................... no\n  SAGE ........................ no\n  SDL ......................... yes\n  screen capture .............. yes\n\n  Portaudio ................... no\n  ALSA ........................ yes\n  CoreAudio ................... no\n  JACK  ....................... no\n  JACK transport .............. no\n\n  FastDXT ..................... yes\n  Realtime DXT (OpenGL) ....... yes\n  JPEG ........................ no (static: no)\n\n  scale postprocessor ......... yes\n  testcard extras ............. no\n  \n"

/* Build drivers as a standalone libraries */
/* #undef BUILD_LIBRARIES */

/* Specifies ABI version for shared objects */
#define COMMON_LIB_ABI_VERSION 1

/* We build with debug messages */
/* #undef DEBUG */

/* Current GIT revision */
/* #undef GIT_VERSION */

/* We have 32-bit Linux */
#define HAVE_32B_LINUX 1

/* We have 64-bit Linux */
/* #undef HAVE_64B_LINUX */

/* Build with ALSA support */
#define HAVE_ALSA 1

/* Define to 1 if you have the <AudioUnit/AudioUnit.h> header file. */
/* #undef HAVE_AUDIOUNIT_AUDIOUNIT_H */

/* Define to 1 if you have the <Carbon/Carbon.h> header file. */
/* #undef HAVE_CARBON_CARBON_H */

/* Build with CoreAudio support */
/* #undef HAVE_COREAUDIO */

/* CUDA is present on the system */
/* #undef HAVE_CUDA */

/* Build with DeckLink support */
#define HAVE_DECKLINK 1

/* Build with DELTACAST support */
/* #undef HAVE_DELTACAST */

/* Build with DVS support */
/* #undef HAVE_DVS */

/* We have libdv */
#define HAVE_DV_CODEC 1

/* Build with DXT_GLSL support */
#define HAVE_DXT_GLSL 1

/* Build with support for FastDXT */
#define HAVE_FASTDXT 1

/* We have Firewire DV */
/* #undef HAVE_FIREWIRE_DV_FREEBSD */

/* Build with OpenGL output */
#define HAVE_GL 1

/* Define to 1 if you have the <GLUT/glut.h> header file. */
/* #undef HAVE_GLUT_GLUT_H */

/* Define to 1 if you have the <GL/glut.h> header file. */
#define HAVE_GL_GLUT_H 1

/* Define to 1 if you have the <GL/glx.h> header file. */
#define HAVE_GL_GLX_H 1

/* Define to 1 if you have the <GL/gl.h> header file. */
#define HAVE_GL_GL_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Enable IPv6 support */
#define HAVE_IPv6 1

/* Build with JACK support */
/* #undef HAVE_JACK */

/* Build with JACK transport support */
/* #undef HAVE_JACK_TRANS */

/* Build with JPEG support */
/* #undef HAVE_JPEG */

/* Define to 1 if you have the `asound' library (-lasound). */
#define HAVE_LIBASOUND 1

/* Define to 1 if you have the `GL' library (-lGL). */
#define HAVE_LIBGL 1

/* Define to 1 if you have the `GLEW' library (-lGLEW). */
#define HAVE_LIBGLEW 1

/* Define to 1 if you have the `gpujpeg' library (-lgpujpeg). */
/* #undef HAVE_LIBGPUJPEG */

/* Define to 1 if you have the `portaudio' library (-lportaudio). */
/* #undef HAVE_LIBPORTAUDIO */

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* Define to 1 if you have the `SDL_mixer' library (-lSDL_mixer). */
/* #undef HAVE_LIBSDL_MIXER */

/* Define to 1 if you have the `SDL_ttf' library (-lSDL_ttf). */
/* #undef HAVE_LIBSDL_TTF */

/* Define to 1 if you have the `videomasterhd' library (-lvideomasterhd). */
/* #undef HAVE_LIBVIDEOMASTERHD */

/* Define to 1 if you have the `X11' library (-lX11). */
#define HAVE_LIBX11 1

/* Build with Linsys support */
#define HAVE_LINSYS 1

/* This is Linux */
#define HAVE_LINUX 1

/* This is Mac X OS */
/* #undef HAVE_MACOSX */

/* This is Mac X OS Leopard */
/* #undef HAVE_MACOSX_LEOPARD */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <OpenGL/glext.h> header file. */
/* #undef HAVE_OPENGL_GLEXT_H */

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
/* #undef HAVE_OPENGL_GL_H */

/* Build with Portaudio support */
/* #undef HAVE_PORTAUDIO */

/* Define to 1 if you have the <QuickTime/QuickTime.h> header file. */
/* #undef HAVE_QUICKTIME_QUICKTIME_H */

/* Build with SAGE support */
/* #undef HAVE_SAGE */

/* Build scale postprocessor */
#define HAVE_SCALE 1

/* Define to 1 if you have the `sched_setscheduler' function. */
#define HAVE_SCHED_SETSCHEDULER 1

/* Build with screen_capture */
#define HAVE_SCREEN_CAP 1

/* Build with SDL support */
#define HAVE_SDL 1

/* We have SDL version >= 1.21 */
#define HAVE_SDL_1210 1

/* Define to 1 if you have the <SDL/SDL.h> header file. */
#define HAVE_SDL_SDL_H 1

/* Build with SPEEX support */
#define HAVE_SPEEX 1

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <stropts.h> header file. */
#define HAVE_STROPTS_H 1

/* Define to 1 if you have the <sys/filio.h> header file. */
/* #undef HAVE_SYS_FILIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Build with testcard2 capture */
/* #undef HAVE_TESTCARD */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <VideoMasterHD_Core.h> header file. */
/* #undef HAVE_VIDEOMASTERHD_CORE_H */

/* Define to 1 if you have the <VideoMasterHD_Sdi_Audio.h> header file. */
/* #undef HAVE_VIDEOMASTERHD_SDI_AUDIO_H */

/* Define to 1 if you have the <VideoMasterHD_Sdi.h> header file. */
/* #undef HAVE_VIDEOMASTERHD_SDI_H */

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* OS kernel major version */
#define OS_VERSION_MAJOR 3

/* OS kernel minor version */
#define OS_VERSION_MINOR 2

/* Name of package */
#define PACKAGE "ultragrid"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "ultragrid-dev@cesnet.cz"

/* Define to the full name of this package. */
#define PACKAGE_NAME "UltraGrid"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "UltraGrid 1.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ultragrid"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.1"

/* The size of `int *', as computed by sizeof. */
#define SIZEOF_INT_P 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* We want to use OpenGL pixel buffer objects */
#define USE_PBO_DXT_ENCODER 1

/* We use RT priority */
/* #undef USE_RT */

/* Version number of package */
#define VERSION "1.1"

/* Specifies ABI version for video capture devices */
#define VIDEO_CAPTURE_ABI_VERSION 2

/* Specifies ABI version for video compression */
#define VIDEO_COMPRESS_ABI_VERSION 1

/* Specifies ABI version for video decompression */
#define VIDEO_DECOMPRESS_ABI_VERSION 2

/* Specifies ABI version for video displays */
#define VIDEO_DISPLAY_ABI_VERSION 2

/* Specifies ABI version for video postprocess */
#define VO_PP_ABI_VERSION 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* This is little endian system */
#define WORDS_SMALLENDIAN 1

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `short' if <sys/types.h> does not define. */
/* #undef int16_t */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef int32_t */

/* Define to `long long' if <sys/types.h> does not define. */
/* #undef int64_t */

/* Define to `signed char' if <sys/types.h> does not define. */
/* #undef int8_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `unsigned short' if <sys/types.h> does not define. */
/* #undef uint16_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef uint32_t */

/* Define to `unsigned char' if <sys/types.h> does not define. */
/* #undef uint8_t */


#ifndef __cplusplus
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif
#endif // ! defined __cplusplus



/*
 * Mac OS X Snow Leopard does not have posix_memalign
 * so supplying a fake one (Macs allocate always aligned pointers
 * to a 16 byte boundry.
 */
#if defined HAVE_MACOSX && OS_VERSION_MAJOR <= 9
#include <errno.h>
#include <stdlib.h>
#ifndef POSIX_MEMALIGN
#define POSIX_MEMALIGN
static inline int posix_memalign(void **memptr, size_t alignment, size_t size);

static inline int posix_memalign(void **memptr, size_t alignment, size_t size)
{
       if(!alignment || (alignment & (alignment - 1)) || alignment > 16) return EINVAL;
       *memptr=malloc(size);
       if(!*memptr) return ENOMEM;
       return 0;
}
#endif // POSIX_MEMALIGN
#endif // defined HAVE_MACOSX && OS_VERSION_MAJOR <= 9

