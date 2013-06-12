dnl Process this file with autoconf to produce a configure script.

AC_INIT([UltraGrid], [1.1], [ultragrid-dev@cesnet.cz])
AM_INIT_AUTOMAKE([1.10])
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

PARENT=`echo $PWD | sed -e 's%/[[^/]]*$%%'`

AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_HEADER_STDC
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AC_C_CHAR_UNSIGNED
AC_HEADER_STDBOOL

AH_BOTTOM([
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
])

AC_CHECK_HEADERS(stropts.h sys/filio.h sys/wait.h)

# -------------------------------------------------------------------------------------------------
POST_COMPILE_MSG=""
CFLAGS="$CFLAGS -I."
CPPFLAGS="$CPPFLAGS -I."
CXXLAGS="$CXXLAGS -I."
LIB_MODULES=
AC_SUBST(LIBS)
AC_SUBST(OBJS)
AC_SUBST(POST_COMPILE_MSG)

AC_CHECK_SIZEOF([int *])

AC_C_BIGENDIAN(
        AC_DEFINE([WORDS_BIGENDIAN], 1, [This is big endian system]),
        AC_DEFINE([WORDS_SMALLENDIAN], 1, [This is little endian system])
)

AC_DEFUN([HM_VERSION_WARNING], [
          AC_MSG_WARN([If you still want to try with unsupported version of dependency,
                       you may want to try --disable-depends-version-check option.
                       If you find out that some unsupported version is working,
                       please let us know at ultragrid-dev@cesnet.cz.
                               ])
          ])

AC_MSG_CHECKING([GIT presence])
if which git
then
        AC_MSG_CHECKING([unreleased changes in GIT tree])
        git_version=`git describe | tr -d '\012'`
        if test "$git_version" -a "$git_version" != "$VERSION" ; then
                AC_DEFINE_UNQUOTED([GIT_VERSION], "$git_version", [Current GIT revision])
                AC_MSG_RESULT($git_version);
        else
                AC_MSG_RESULT(no);
        fi
fi

case "$host" in
	*darwin9*)
		AC_DEFINE([HAVE_MACOSX_LEOPARD], [1], [This is Mac X OS Leopard])
	;;
  	x86_64-*-linux-*)
		AC_DEFINE([HAVE_64B_LINUX], [1], [We have 64-bit Linux])
	;;
	i*86-*-linux-*)
		AC_DEFINE([HAVE_32B_LINUX], [1], [We have 32-bit Linux])
	;;
esac

AC_MSG_CHECKING([OS family])

if expr "$host_os" : ".*darwin.*" > /dev/null; then
        system=MacOSX
	AC_DEFINE([HAVE_MACOSX], [1], [This is Mac X OS])
	CFLAGS="$CFLAGS -arch i386 -I/opt/local/include"
	CPPFLAGS="$CPPFLAGS -m32 -I/opt/local/include"
	CXXFLAGS="$CXXFLAGS -m32 -I/opt/local/include"
	LDFLAGS="$LDFLAGS -m32 -mmacosx-version-min=10.5"
	X_CFLAGS="$X_CFLAGS -I/usr/X11R6/include"
        OBJS="$OBJS src/utils/autorelease_pool.o"
else if expr "$host_os" : ".*mingw32.*" > /dev/null; then
        system=Windows
	AC_DEFINE([WIN32], [1], [This is an Windows OS])
	LIBS="$LIBS -lsetupapi -lWs2_32 -lIphlpapi"
        LDFLAGS="$LDFLAGS -static-libgcc -static-libstdc++"
else
        system=Linux
	LDFLAGS="$LDFLAGS -Wl,--dynamic-list-data"
	AC_DEFINE([HAVE_LINUX], [1], [This is Linux])
fi
fi

AC_MSG_RESULT($system);

AC_MSG_CHECKING([OS kernel version (major)])
os_version_major=`uname -r |cut -d . -f 1`
AC_MSG_RESULT($os_version_major);
AC_MSG_CHECKING([OS kernel version (minor)])
os_version_minor=`uname -r |cut -d . -f 2`
AC_MSG_RESULT($os_version_minor);

AC_DEFINE_UNQUOTED([OS_VERSION_MAJOR], $os_version_major, [OS kernel major version])
AC_DEFINE_UNQUOTED([OS_VERSION_MINOR], $os_version_minor, [OS kernel minor version])

LINKER=$CXX

AC_SUBST(LINKER)
AC_SUBST(LDFLAGS)
AC_SUBST(CXXFLAGS)

if test $system = Windows
then
        TARGET="bin/uv.exe"
else
        TARGET="bin/uv"
fi
AC_SUBST(TARGET)

AC_ARG_ENABLE(depends-version-check,
        AS_HELP_STRING([--disable-depends-version-check], [Disable libraries version check]),
	[version_check=$enableval],
        [version_check=yes]
        )

if test $system = MacOSX -a \( $os_version_major -lt 10 -o $os_version_major -gt 11 \) -a $version_check = yes
then
        AC_MSG_WARN([*** Currently only Mac OS X versions 10.6 to 10.7 are supported.])
        HM_VERSION_WARNING
        AC_MSG_ERROR([Exiting due to unsatisfied dependency.])
fi

if test $system = MacOSX -o $system = Linux
then
        CFLAGS="$CFLAGS -fPIC"
        CXXFLAGS="$CXXFLAGS -fPIC"
        NVCCFLAGS="$NVCCFLAGS -Xcompiler -fPIC"
fi

AH_BOTTOM([
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
])

if test "$ac_cv_prog_cc_c99" = no
then
        AC_MSG_ERROR([Your compiler does not accept C99.])
fi

AC_CHECK_SIZEOF([int *])

if test $ac_cv_sizeof_int_p -eq 8
then
        WORD_LEN=64
else
        WORD_LEN=32
fi

# -------------------------------------------------------------------------------------------------
# Licensing
# -------------------------------------------------------------------------------------------------
# default UltraGrid license
license="BSD License"

gpl=no
AC_ARG_ENABLE(gpl,
              AS_HELP_STRING([--enable-gpl], [allow use of GPL code, ]
              [resulting binary will be unredistributable]),
	[gpl=$enableval])
if test $gpl = yes
then
          AC_DEFINE([HAVE_GPL], 1, [We can link with GPL software])
          license=unredistributeable
fi


# -------------------------------------------------------------------------------------------------
# Additional macros
# -------------------------------------------------------------------------------------------------

AC_DEFUN([DEFINE_GL], [
          AC_DEFINE([HAVE_LIBGL], 1, [We use GL libraries])

          if test "$system" = MacOSX; then
                  GL_COMMON_OBJ="src/mac_gl_common.o src/gl_context.o"
          else if test "$system" = Linux; then
                  GL_COMMON_OBJ="src/glx_common.o src/gl_context.o"
          else
                  GL_COMMON_OBJ="src/win32_gl_common.o src/gl_context.o"
          fi
          fi
          AC_SUBST(GL_COMMON_OBJ)
          ])

AC_MSG_CHECKING([if_nametoindex])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <windows.h>
#include <iphlpapi.h>
					]], [[
if_nametoindex("eth");]]
)], HAVE_IF_NAMETOINDEX=yes, HAVE_IF_NAMETOINDEX=no)
	AC_MSG_RESULT([$HAVE_IF_NAMETOINDEX])

AC_CHECK_FUNCS(strtok_r)

AC_CHECK_FUNCS(drand48)
if test $ac_cv_func_drand48 = no
then
	AC_DEFINE([NEED_DRAND48], 1, [We need custom implementation of drand48.])
fi


# -------------------------------------------------------------------------------------------------
# Check for standard size types.  The defaults are only valid on some
# systems so we hope that <inttypes.h> exists when they're wrong.

AC_CHECK_TYPE(int8_t,  signed char)
AC_CHECK_TYPE(int16_t, short)
AC_CHECK_TYPE(int32_t, long)
AC_CHECK_TYPE(int64_t, long long)

# Some systems have these in <stdint.h>, just to be difficult...
AC_CACHE_CHECK(for uint8_t in <stdint.h>, ucl_cv_uint8_t_in_stdint_h,
	AC_EGREP_HEADER(uint8_t,
			stdint.h,
			ucl_cv_uint8_t_in_stdint_h=yes,
			ucl_cv_uint8_t_in_stdint_h=no))
if test $ucl_cv_uint8_t_in_stdint_h = "no"
then
	 AC_CHECK_TYPE(uint8_t,  unsigned char)
fi

AC_CACHE_CHECK(for uint16_t in <stdint.h>, ucl_cv_uint16_t_in_stdint_h,
	AC_EGREP_HEADER(uint16_t,
			stdint.h,
			ucl_cv_uint16_t_in_stdint_h=yes,
			ucl_cv_uint16_t_in_stdint_h=no))
if test $ucl_cv_uint16_t_in_stdint_h = "no"
then
	 AC_CHECK_TYPE(uint16_t,  unsigned short)
fi

AC_CACHE_CHECK(for uint32_t in <stdint.h>, ucl_cv_uint32_t_in_stdint_h,
	AC_EGREP_HEADER(uint32_t,
			stdint.h,
			ucl_cv_uint32_t_in_stdint_h=yes,
			ucl_cv_uint32_t_in_stdint_h=no))
if test $ucl_cv_uint32_t_in_stdint_h = "no"
then
	 AC_CHECK_TYPE(uint32_t,  unsigned int)
fi

# -------------------------------------------------------------------------------------------------
# GCC-specific warning flags
if test "$GCC" = yes
then
    CFLAGS="$CFLAGS -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations"
fi

# -------------------------------------------------------------------------------------------------
# Check for libraries

AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(inet_addr, nsl)
AC_CHECK_LIB(rt, timer_create)

AC_CHECK_FUNC(sin, MATHLIBS="", MATHLIBS="-lm")
AC_CHECK_LIB(ieee, main, [MATHLIBS="-lieee $MATHLIBS"])
AC_CHECK_FUNCS(inet_ntop)
AC_CHECK_FUNCS(inet_pton)
AC_SUBST(MATHLIBS)

# -------------------------------------------------------------------------------------------------
# See if this system supports sched_setscheduler()

AC_CHECK_FUNCS(sched_setscheduler)
use_rt=no
AC_ARG_ENABLE(rt,
              AS_HELP_STRING([--enable-rt], [enable RT priority]),
	[use_rt=$enableval])
if test "$use_rt" = "yes"; then
        AC_DEFINE([USE_RT], [1], [We use RT priority])
fi

# -------------------------------------------------------------------------------------------------
# Try to find the pthread library...

case "$host" in
	*-*-freebsd4*)
		CFLAGS="$CFLAGS -pthread"
		;;
	*)
		AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", AC_MSG_ERROR([Require POSIX thread]))
		;;
esac

# -------------------------------------------------------------------------------------------------
debug_output=no
profile=no
ipv6=yes

AC_ARG_ENABLE(profile,
	[  --enable-profile        enable profiling],
	[if test $enableval = yes
	then
		if test "$GCC" = yes
		then
			CFLAGS="$CFLAGS -pg"
                        profile=yes
		else
			AC_MSG_ERROR([Do not know how to enable profiling for $CC])
		fi
	fi])

AC_ARG_ENABLE(debug,
	[  --enable-debug          enable debug messages and code],
	[if test $enableval = yes
	then
		AC_DEFINE([DEBUG], [1], [We build with debug messages])
                debug_output=yes
	fi])

AC_ARG_ENABLE(ipv6,
              AS_HELP_STRING([--disable-ipv6], [disable ipv6]),
              [if test $enableval = no
               then
                       ipv6=no
               fi])

if test "$ipv6" != no
then
        AC_DEFINE([HAVE_IPv6], [1], [Enable IPv6 support])
fi

# ------------------------------------------------------------------------------------------------
# Dynamic libraries stuff
#
build_libraries=no
AC_ARG_ENABLE(dlsym, AS_HELP_STRING([--enable-dlsym], [build modules as libraries (default is off)]),
	[build_libraries=$enableval])

if test "$build_libraries" = yes
then
        if test "$system" = "Linux"
        then
                LIBS+=' -ldl'
        else
                AC_MSG_WARN([*** Building libraries is not supported with Mac OS X])
                build_libraries=no
        fi
        AC_DEFINE([BUILD_LIBRARIES], [1], [Build drivers as a standalone libraries])
fi

common_lib_abi_version=1
AC_DEFINE_UNQUOTED([COMMON_LIB_ABI_VERSION], $common_lib_abi_version, [Specifies ABI version for shared objects])
AC_SUBST(common_lib_abi_version)
AC_SUBST(COMMON_LIB_TARGET, "lib/ultragrid/ug_lib_common.so.$common_lib_abi_version")
LIB_TARGETS="$LIB_TARGETS $COMMON_LIB_TARGET"

video_display_abi_version=5
video_capture_abi_version=2
video_compress_abi_version=2
video_decompress_abi_version=3
audio_capture_abi_version=3
audio_playback_abi_version=4
audio_codec_abi_version=1
vo_pp_abi_version=3
AC_DEFINE_UNQUOTED([VIDEO_DISPLAY_ABI_VERSION], $video_display_abi_version, [Specifies ABI version for video displays])
AC_DEFINE_UNQUOTED([VIDEO_CAPTURE_ABI_VERSION], $video_capture_abi_version, [Specifies ABI version for video capture devices])
AC_DEFINE_UNQUOTED([VIDEO_COMPRESS_ABI_VERSION], $video_compress_abi_version, [Specifies ABI version for video compression])
AC_DEFINE_UNQUOTED([VIDEO_DECOMPRESS_ABI_VERSION], $video_decompress_abi_version, [Specifies ABI version for video decompression])
AC_DEFINE_UNQUOTED([AUDIO_CAPTURE_ABI_VERSION], $audio_capture_abi_version, [Specifies ABI version for audio capture])
AC_DEFINE_UNQUOTED([AUDIO_PLAYBACK_ABI_VERSION], $audio_playback_abi_version, [Specifies ABI version for audio playback])
AC_DEFINE_UNQUOTED([AUDIO_CODEC_ABI_VERSION], $audio_codec_abi_version, [Specifies ABI version for audio codec])
AC_DEFINE_UNQUOTED([VO_PP_ABI_VERSION], $vo_pp_abi_version, [Specifies ABI version for video postprocess])
AC_SUBST(video_display_abi_version)
AC_SUBST(video_capture_abi_version)
AC_SUBST(video_compress_abi_version)
AC_SUBST(video_decompress_abi_version)
AC_SUBST(audio_capture_abi_version)
AC_SUBST(audio_playback_abi_version)
AC_SUBST(audio_codec_abi_version)
AC_SUBST(vo_pp_abi_version)

# ------------------------------------------------------------------------------------------------
# environment variables
#
AC_ARG_VAR([CUDA_DIRECTORY], [Directory of your Nvidia toolkit instalation.])
AC_ARG_VAR([SAGE_DIRECTORY], [Directory of your SAGE installation.])
AC_ARG_VAR([DVS_DIRECTORY], [Path to DVS installation.])
AC_ARG_VAR([DELTACAST_DIRECTORY], [Placement of VideoMasterHD directory (Deltacast).])
AC_ARG_VAR([BLUE_LATEST_SDK], [Placement of Bluefish444 SDK.])

# -------------------------------------------------------------------------------------------------
# UYVY
uyvy=no

AC_ARG_ENABLE(uyvy,
      AS_HELP_STRING([--disable-uyvy], [disable dummy RGB to UYVY compression (default is auto)]),
	[uyvy_req=$enableval],
        [uyvy_req=auto])

SAVED_LIBS=$LIBS

# Linux
if test $uyvy_req != no -a "$ac_cv_lib_X11_XCreateWindow" = yes -a $FOUND_GLEW = yes -a "$FOUND_GLX_L" = yes -a "$FOUND_GLX_H" = yes \
        -a "$FOUND_GL_H" = yes -a "$system" = Linux
then
        UYVY_LIB=" -lGLEW -lGL -lX11 $X_LIBS"
        uyvy=yes
fi

# Mac
if test "$system" = MacOSX
then
        uyvy=no
fi

# common
if test $uyvy = yes
then
        AC_DEFINE([HAVE_COMPRESS_UYVY], [1], [Build with dummy UYVY compression])
        UYVY_COMPRESS_OBJ="src/video_compress/uyvy.o"
        AC_SUBST(UYVY_COMPRESS_LIB_TARGET, "lib/ultragrid/vcompress_uyvy.so.$video_compress_abi_version")
        LIB_TARGETS="$LIB_TARGETS $UYVY_COMPRESS_LIB_TARGET"
        LIB_OBJS="$LIB_OBJS $UYVY_COMPRESS_OBJ"
        DEFINE_GL
fi

if test $uyvy_req = yes -a $uyvy = no; then
        AC_MSG_ERROR([UYVY not found]);
fi

LIB_MODULES="$LIB_MODULES $UYVY_LIB"

AC_SUBST(UYVY_COMPRESS_OBJ)
AC_SUBST(UYVY_LIB)


# -------------------------------------------------------------------------------------------------
# CUDA staff
#
# CUDA lookup order:
# 1) --with-cuda
# 2) CUDA_DIRECTORY env variable
# 3) default system path (PATH, LD_LIBRARY_PATH, C_INCLUDE_PATH/CPATH)
# 4) default CUDA path

CUDA_INC=
CUDA_LIB=
NVCC=
NVCCFLAGS=
CUDA_LIB_PATH=
FOUND_CUDA=no
HAVE_CUDA=no
CUDA_PATH=$CUDA_DIRECTORY

AC_ARG_WITH(cuda,
            [  --with-cuda=DIR         specify location of CUDA Toolkit],
            [CUDA_PATH=$withval
            ])

SAVED_LIBS=$LIBS
LIBS="$LIBS -lcudart"
if test -z "$CUDA_PATH"
then
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cuda_runtime_api.h>
                           ]],
                           [[int devices_count;
                            cudaGetDeviceCount(&devices_count);
                            ]])],FOUND_CUDA=yes,FOUND_CUDA=no)
        AC_CHECK_PROG([NVCC], [nvcc], [nvcc],
                      [])
fi

if test $FOUND_CUDA = no -o -z "$NVCC"
then
        FOUND_CUDA=no
        NVCC=
        if test -z "$CUDA_PATH"; then
                CUDA_PATH=/usr/local/cuda
        fi
        SAVED_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -I$CUDA_PATH/include/"
        if test $WORD_LEN -eq 64 -a $system = Linux
        then
                LIBS="$LIBS -L$CUDA_PATH/lib64"
        else
                LIBS="$LIBS -L$CUDA_PATH/lib"
        fi
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cuda_runtime_api.h>
                           ]],
                           [[int devices_count;
                            cudaGetDeviceCount(&devices_count);
                            ]])],FOUND_CUDA=yes,FOUND_CUDA=no)
        CFLAGS=$SAVED_CFLAGS
        AC_CHECK_PROG([NVCC], [nvcc], [$CUDA_PATH/bin/nvcc],
                      [],
                      [$CUDA_PATH/bin])
fi

LIBS=$SAVED_LIBS

if test -n "$NVCC" -a $FOUND_CUDA = yes
then
        HAVE_CUDA=yes
	AC_MSG_CHECKING([CUDA Toolkit version - major])
        nvcc_major=`$NVCC --version |grep release|sed  's/^.*release \(@<:@0-9@:>@@<:@0-9@:>@*\).*$/\1/'`
	AC_MSG_RESULT($nvcc_major)
	AC_MSG_CHECKING([CUDA Toolkit version - minor])
        nvcc_minor=`$NVCC --version |grep release|sed  's/^.*release @<:@0-9@:>@@<:@0-9@:>@*\.\(@<:@0-9@:>@@<:@0-9@:>@*\).*$/\1/'`
	AC_MSG_RESULT($nvcc_minor)

        if test -n "$CUDA_PATH"; then
                CUDA_INC=-I$CUDA_PATH/include
        else
                CUDA_INC=
        fi
        if test $ac_cv_sizeof_int_p -eq 8 -a $system = Linux
        then
                CUDA_LIB_PATH="$CUDA_PATH/lib64"
        else
                CUDA_LIB_PATH="$CUDA_PATH/lib"
        fi

        CUDA_LIB="-L$CUDA_LIB_PATH -lcudart"
fi

AC_SUBST(HAVE_CUDA)
AC_SUBST(NVCC)
AC_SUBST(FOUND_CUDA)
AC_SUBST(NVCCFLAGS)
AC_SUBST(CUDA_PATH)
AC_SUBST(CUDA_LIB)
AC_SUBST(CUDA_INC)

# -------------------------------------------------------------------------------------------------
# JPEG
JPEG_OBJ=
JPEG_COMPRESS_OBJ=
JPEG_DECOMPRESS_OBJ=
JPEG_INC=
JPEG_LIB=

jpeg=no
jpeg_static=no

AC_ARG_ENABLE(jpeg,
      AS_HELP_STRING([--disable-jpeg], [disable JPEG compression (auto if GPUJPEG library in system)]),
	[jpeg_req=$enableval],
        [jpeg_req=auto])

SAVED_CXXFLAGS=$CXXFLAGS
SAVED_CPPFLAGS=$CPPFLAGS
CXXFLAGS="$CXXFLAGS $CUDA_INC"
CPPFLAGS="$CPPFLAGS $CUDA_INC"
SAVED_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS $CUDA_INC"
CFLAGS=$SAVED_CFLAGS
#AC_CHECK_FILE(libgpujpeg/libgpujpeg.a)
CXXFLAGS=$SAVED_CXXFLAGS
CPPFLAGS=$SAVED_CPPFLAGS

if test $HAVE_CUDA = yes
then
        if test \( $nvcc_major -gt 4 \) -o \( $nvcc_major -eq 4 -a $nvcc_minor -ge 1 \) -o $version_check = no
        then
                jpeg_cuda_version_ok=yes
        else
                AC_MSG_WARN([*** CUDA Toolkit older than 4.1 detected. Please install at least v4.1 to use JPEG.])
                HM_VERSION_WARNING
        fi

        if test $version_check = no -o \
                \( $system = Linux -o $os_version_major -ge 10 \) # Linux or Mac at least Snow Leopard
        then
                jpeg_os_version_ok=yes
        else
                AC_MSG_WARN([*** Detected unsupported OS version for CUDA.])
                HM_VERSION_WARNING
        fi
fi

SAVED_LIBS=$LIBS
AC_CHECK_LIB([gpujpeg], [gpujpeg_encoder_create])
LIBS=$SAVED_LIBS
if test $ac_cv_lib_gpujpeg_gpujpeg_encoder_create = no; then
        jpeg_static=yes
fi

if test \( "$jpeg_req" = yes -o \( "$jpeg_req" = auto -a $jpeg_static = no \) \) -a \
        $HAVE_CUDA = yes -a "$jpeg_cuda_version_ok" = yes -a \
        "$jpeg_os_version_ok" = yes
then
        jpeg=yes

        JPEG_LIB=" $CUDA_LIB"
        JPEG_INC=$CUDA_INC
        JPEG_COMMON_OBJ="src/video_compress/jpeg.o"
        if test $jpeg_static = yes; then
                JPEG_COMMON_OBJ="$JPEG_COMMON_OBJ gpujpeg/.libs/libgpujpeg.a"
                JPEG_INC="$JPEG_INC -Igpujpeg"
        else
                JPEG_LIB="$JPEG_LIB -lgpujpeg"
        fi
        JPEG_DECOMPRESS_OBJ="src/video_decompress/jpeg.o "
        LINKER=$CXX
        AC_DEFINE([HAVE_CUDA], [1], [CUDA is present on the system])
        AC_DEFINE([HAVE_JPEG], [1], [Build with JPEG support])
        AC_SUBST(JPEG_COMPRESS_LIB_TARGET, "lib/ultragrid/vcompress_jpeg.so.$video_compress_abi_version")
        AC_SUBST(JPEG_DECOMPRESS_LIB_TARGET, "lib/ultragrid/vdecompress_jpeg.so.$video_decompress_abi_version")
        LIB_TARGETS="$LIB_TARGETS $JPEG_COMPRESS_LIB_TARGET $JPEG_DECOMPRESS_LIB_TARGET"
        LIB_OBJS="$LIB_OBJS $JPEG_DECOMPRESS_OBJ $JPEG_COMMON_OBJ"
        POST_COMPILE_MSG="$POST_COMPILE_MSG\n***\nYou have compiled in JPEG support.\nIn order to use use JPEG compression and \
decompression, you will need to have CUDA libraries visible to your OS.\n\
If not done so, you can accomplish this by adding line:\n\
        export LD_LIBRARY_PATH=$CUDA_LIB_PATH:\\\$\$LD_LIBRARY_PATH\n\
to your .bashrc file (in home directory). To take effect immediatelly, you will need to enter:\n\
        exec bash\n***\n"
fi

if test $jpeg_req = yes -a $jpeg = no; then
        AC_MSG_ERROR([JPEG not found]);
fi

LIB_MODULES="$LIB_MODULES $JPEG_LIB"

AC_SUBST(JPEG_COMMON_OBJ)
AC_SUBST(JPEG_COMPRESS_OBJ)
AC_SUBST(JPEG_DECOMPRESS_OBJ)
AC_SUBST(JPEG_INC)
AC_SUBST(JPEG_LIB)

# -------------------------------------------------------------------------------------------------
# AUDIO stuff
# -------------------------------------------------------------------------------------------------
# SPEEX

speex=yes
SPEEX_PATH=speex-1.2rc1
SPEEX_INC=-I${SPEEX_PATH}/include
SPEEX_OBJ="src/audio/resample.o src/audio/preprocess.o src/audio/filterbank.o src/audio/fftwrap.o src/audio/smallft.o src/audio/mdf.o"
SPEEX_OBJ="$SPEEX_OBJ src/audio/echo.o"
SPEEX_LIB=
AC_DEFINE([HAVE_SPEEX], [1], [Build with SPEEX support])

AC_SUBST(SPEEX_INC)
AC_SUBST(SPEEX_LIB)
AC_SUBST(SPEEX_OBJ)

# -------------------------------------------------------------------------------------------------
# Scale Stuff
# -------------------------------------------------------------------------------------------------
SCALE_INC=
SCALE_LIB=
SCALE_OBJ=
scale=no

AC_ARG_ENABLE(scale,
      AS_HELP_STRING([--disable-scale], [disable scale support(default is auto)]),
    [scale_req=$enableval],
    [scale_req=auto]
    )

if test $scale_req != no -a $OPENGL = yes
then
        scale=yes
        SCALE_LIB="$SCALE_LIB $OPENGL_LIB $X11_LIB"
        SCALE_OBJ="$SCALE_OBJ src/vo_postprocess/scale.o"
        AC_SUBST(SCALE_LIB_TARGET, "lib/ultragrid/vo_pp_scale.so.$vo_pp_abi_version")
        LIB_TARGETS="$LIB_TARGETS $SCALE_LIB_TARGET"
        LIB_OBJS="$LIB_OBJS $SCALE_OBJ"
        AC_DEFINE([HAVE_SCALE], [1], [Build scale postprocessor])
        DEFINE_GL
fi

if test $scale_req = yes -a $scale = no; then
        AC_MSG_ERROR([Scale not found]);
fi

LIB_MODULES="$LIB_MODULES $SCALE_LIB"

AC_SUBST(SCALE_INC)
AC_SUBST(SCALE_LIB)
AC_SUBST(SCALE_OBJ)

# -------------------------------------------------------------------------------------------------
# Libav
# -------------------------------------------------------------------------------------------------
libavcodec=no
libavcodec_audio=no
AC_ARG_ENABLE(libavcodec,
AS_HELP_STRING([--disable-libavcodec], [disable libavcodec support(default is auto)]),
[libavcodec_req=$enableval],
[libavcodec_req=auto]
)

if test $system = Windows; then
AC_CHECK_HEADERS([libavcodec/avcodec.h libavutil/imgutils.h libavutil/opt.h])
AC_CHECK_LIB(avcodec, avcodec_open2)
AC_CHECK_LIB(avutil, av_free)
if test $ac_cv_header_libavcodec_avcodec_h = yes -a \
        $ac_cv_header_libavutil_imgutils_h = yes -a \
        $ac_cv_header_libavutil_opt_h = yes -a \
        $ac_cv_lib_avcodec_avcodec_open2 = yes -a \
        $ac_cv_lib_avutil_av_free = yes
then
        found_libavcodec=yes
        found_libavutil=yes
        libavcodec_ge_55=yes # TODO
        LIBAVCODEC_LIBS="-lavcodec"
        LIBAVUTIL_LIBS="-lavutil"
else
        found_libavcodec=no
fi
else # Linux, OS X
PKG_CHECK_MODULES([LIBAVCODEC], [libavcodec >= 53.35.0 ], [found_libavcodec=yes], [found_libavcodec=no])
PKG_CHECK_MODULES([LIBAVCODEC_GE_55], [libavcodec >= 55.0.0 ], [libavcodec_ge_55=yes], [libavcodec_ge_55=no])
PKG_CHECK_MODULES([LIBAVUTIL], [libavutil >= 51.22.1 ], [found_libavutil=yes], [found_libavutil=no])
fi

if test $libavcodec_req != no -a $found_libavcodec = yes -a $found_libavutil = yes
then
        LIBAVCODEC_OBJ="src/video_compress/libavcodec.o"
        AC_DEFINE([HAVE_LAVC], [1], [Build with LAVC support])
        AC_SUBST(LIBAVCODEC_COMPRESS_LIB_TARGET, "lib/ultragrid/vcompress_libavcodec.so.$video_compress_abi_version")
        AC_SUBST(LIBAVCODEC_DECOMPRESS_LIB_TARGET, "lib/ultragrid/vdecompress_libavcodec.so.$video_decompress_abi_version")
        if test $libavcodec_ge_55 = yes; then
                AC_SUBST(LIBAVCODEC_AUDIO_CODEC_LIB_TARGET, "lib/ultragrid/acodec_libavcodec.so.$audio_codec_abi_version")
                AC_DEFINE([HAVE_LAVC_AUDIO], [1], [Build with LAVC audio support])
                libavcodec_audio=yes
                LIBAVCODEC_AUDIO_CODEC_OBJ=src/audio/codec/libavcodec.o
        fi
        LIB_TARGETS="$LIB_TARGETS $LIBAVCODEC_COMPRESS_LIB_TARGET $LIBAVCODEC_DECOMPRESS_LIB_TARGET $LIBAVCODEC_AUDIO_CODEC_LIB_TARGET"
        LIBAVCODEC_COMPRESS_OBJ=src/video_compress/libavcodec.o
        LIBAVCODEC_DECOMPRESS_OBJ=src/video_decompress/libavcodec.o
        LIB_OBJS="$LIB_OBJS $LIBAVCODEC_COMPRESS_OBJ $LIBAVCODEC_DECOMPRESS_OBJ $LIBAVCODEC_AUDIO_CODEC_OBJ"
        CFLAGS="$CFLAGS $LIBAVCODEC_CFLAGS"
        libavcodec=yes
        SAVED_LIBS=$LIBS
        LIBAVCODEC_LIBS="$LIBAVCODEC_LIBS $LIBAVUTIL_LIBS" # append libavutil
        LIBS="$LIBS $LIBAVCODEC_LIBS"
        AC_CHECK_FUNCS(avcodec_encode_video2)
        LIBS=$SAVED_LIBS
else
        LIBAVCODEC_LIBS=
        LIBAVCODEC_OBJ=
fi

if test $libavcodec_req = yes -a $libavcodec = no; then
        AC_MSG_ERROR([Libavcodec not found]);
fi

AC_SUBST(LIBAVCODEC_LIBS) # set by pkg-config
AC_SUBST(LIBAVCODEC_COMPRESS_OBJ)
AC_SUBST(LIBAVCODEC_DECOMPRESS_OBJ)
AC_SUBST(LIBAVCODEC_AUDIO_CODEC_OBJ)

LIB_MODULES="$LIB_MODULES $LIBAVCODEC_LIBS"

# -------------------------------------------------------------------------------------------------
# use shared decoder
# -------------------------------------------------------------------------------------------------

# the shared behaviour is faulty, but it saves resources when restarting sender (different SSRC).
shared_decoder=no
if test $shared_decoder = yes
then
        AC_DEFINE([SHARED_DECODER], [1], [use shared decoder for all participants])
fi

# -------------------------------------------------------------------------------------------------
# We need to add libraries then
# -------------------------------------------------------------------------------------------------

# this is only needed when passing to "clean" make target
if test "$build_libraries" = yes
then
        LIB_SUPPORT_OBJ="src/lib_common.o"
else
        LIBS="$LIBS $LIB_MODULES"
        OBJS="$OBJS $LIB_OBJS $GL_COMMON_OBJ $X_OBJ"
        HEADERS="$HEADERS $LIB_HEADERS"
        LIB_OBJS=
        LIB_TARGETS=
        LIB_HEADERS=
fi

AC_SUBST(LIB_SUPPORT_OBJ)

AC_SUBST(LIB_OBJS)
AC_SUBST(LIB_TARGETS)
AC_SUBST(HEADERS)
AC_SUBST(LIB_HEADERS)

# -------------------------------------------------------------------------------------------------
# Import control - keyboard
# -------------------------------------------------------------------------------------------------
FOUND_NCURSES=no

if test $system = Linux -o $system = MacOSX; then
        PKG_CHECK_MODULES([NCURSES], [ncursesw], [FOUND_NCURSES=yes], [FOUND_NCURSES=no])
        if test $FOUND_NCURSES = no; then
                PKG_CHECK_MODULES([NCURSES], [ncurses], [FOUND_NCURSES=yes], [FOUND_NCURSES=no])
        fi

        if test $FOUND_NCURSES = yes; then
                AC_DEFINE([HAVE_NCURSES], [1], [Build with ncurses support])
                IMPORT_CONTROL_KEYBOARD_LIBS="$NCURSES_LIBS"
        fi
fi

AC_SUBST(IMPORT_CONTROL_KEYBOARD_LIBS)

# -------------------------------------------------------------------------------------------------
# Run subsequent configure scripts
# -------------------------------------------------------------------------------------------------
if test $jpeg = yes -a $jpeg_static = yes
then
        AC_MSG_NOTICE(
[
Configuring JPEG
])
        /bin/sh -c "cd gpujpeg; ./configure --with-cuda=$CUDA_PATH"
fi

LIB_MODULES="$LIB_MODULES $V4L2_LIBS"

# -------------------------------------------------------------------------------------------------
# Finally, substitute things into the Makefile and config.h

RESULT=\
"  Target ...................... $host
  Debug output ................ $debug_output
  Profiling support ........... $profile
  IPv6 support ................ $ipv6
  RT priority ................. $use_rt
  Standalone modules .......... $build_libraries
  License ..................... $license

  Portaudio ................... $portaudio
  ALSA ........................ $alsa
  CoreAudio ................... $coreaudio

  JPEG ........................ $jpeg (static: $jpeg_static)
  UYVY dummy compression ...... $uyvy
  Libavcodec .................. $libavcodec (audio: $libavcodec_audio)

  scale postprocessor ......... $scale
  "

AC_MSG_RESULT(
[
UltraGrid Configuration summary:
])
AC_MSG_RESULT([$RESULT])

RES_STR=`printf "$RESULT" | sed 's/$/\\\\n/' | tr -d '\n'`

AC_DEFINE_UNQUOTED([AUTOCONF_RESULT], "$RES_STR", [Autoconf result])

AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

