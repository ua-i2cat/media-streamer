/* Specifies ABI version for audio capture */
#define AUDIO_CAPTURE_ABI_VERSION 3

/* Specifies ABI version for audio codec */
#define AUDIO_CODEC_ABI_VERSION 1

/* Specifies ABI version for audio playback */
#define AUDIO_PLAYBACK_ABI_VERSION 4

/* Autoconf result */
#define AUTOCONF_RESULT "  Target ...................... i686-pc-linux-gnu\n  Debug output ................ no\n  Profiling support ........... no\n  IPv6 support ................ yes\n  RT priority ................. no\n  Standalone modules .......... no\n  License ..................... unredistributeable\n\n  Portaudio ................... \n  ALSA ........................ \n  CoreAudio ................... \n\n  JPEG ........................ no (static: yes)\n  UYVY dummy compression ...... no\n  Libavcodec .................. yes (audio: no)\n\n  scale postprocessor ......... no\n  \n"

/* Specifies ABI version for shared objects */
#define COMMON_LIB_ABI_VERSION 1

/* We have 32-bit Linux */
#define HAVE_32B_LINUX 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `drand48' function. */
#define HAVE_DRAND48 1

/* Define to 1 if you have the `inet_ntop' function. */
#define HAVE_INET_NTOP 1

/* Define to 1 if you have the `inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Enable IPv6 support */
#define HAVE_IPv6 1

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* This is Linux */
#define HAVE_LINUX 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `sched_setscheduler' function. */
#define HAVE_SCHED_SETSCHEDULER 1

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

/* Define to 1 if you have the `strtok_r' function. */
#define HAVE_STRTOK_R 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* OS kernel major version */
#define OS_VERSION_MAJOR 2

/* OS kernel minor version */
#define OS_VERSION_MINOR 6

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

/* use shared decoder for all participants */
/* #undef SHARED_DECODER */

/* The size of `int *', as computed by sizeof. */
#define SIZEOF_INT_P 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* We use RT priority */
/* #undef USE_RT */

/* Version number of package */
#define VERSION "1.1"

/* Specifies ABI version for video capture devices */
#define VIDEO_CAPTURE_ABI_VERSION 2

/* Specifies ABI version for video compression */
#define VIDEO_COMPRESS_ABI_VERSION 2

/* Specifies ABI version for video decompression */
#define VIDEO_DECOMPRESS_ABI_VERSION 3

/* Specifies ABI version for video displays */
#define VIDEO_DISPLAY_ABI_VERSION 5

/* Specifies ABI version for video postprocess */
#define VO_PP_ABI_VERSION 3

/* This is an Windows OS */
/* #undef WIN32 */

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

