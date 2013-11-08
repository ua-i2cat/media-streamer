#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ug-modules], [0.1.0])
#AC_CONFIG_SRCDIR([cuda_dxt/cuda_dxt.h])

# non-verbose output
AM_SILENT_RULES([yes])

LT_INIT

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([GL], [glutInit])
AC_CHECK_LIB([GLEW], [glewInit])
AC_CHECK_LIB([GLU], [glutInit])
AC_CHECK_LIB([avcodec], [avcodec_encode_video2])
AC_CHECK_LIB([avformat], [avformat_alloc_context])
AC_CHECK_LIB([avutil], [avutil_configuration])
AC_CHECK_LIB([crypto], [lbl_random])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([glut], [glutInit])
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([pthread], [pthread_mutex_init])
AC_CHECK_LIB([rt], [clock_gettime])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h mach/mach.h malloc.h memory.h netdb.h netinet/in.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyname gethostname gettimeofday inet_ntoa memset select socket sqrt strcasecmp strchr strcspn strdup strncasecmp strrchr strspn strstr strtoul uname])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 io_mngr/Makefile])
#AC_CONFIG_FILES([Makefile
#                 cuda_dxt/Makefile
#                 dxt_compress/makefile])
AC_CONFIG_SUBDIRS([gpujpeg])
AC_OUTPUT
