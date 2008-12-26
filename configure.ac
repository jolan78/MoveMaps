AC_INIT( [mmaps], [0.0.1] )

AC_CONFIG_SRCDIR([dep/src/G3D/LineSegment.cpp])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_CC_C_O

# configure options
AC_ARG_ENABLE([debug],
[AS_HELP_STRING([--enable-debug],[Turn on debugging])],
[case "${enableval}" in
yes) debug=true ;;
no)  debug=false ;;
*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])

AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_CONFIG_FILES([
    Makefile
    dep/Makefile
    dep/src/G3D/Makefile
    dep/src/GLG3D/Makefile
    dep/src/zip/Makefile
    dep/src/Makefile
    src/Makefile
    src/vmap/Makefile
    src/mmap/Makefile
    src/generator/Makefile
    src/viewer/Makefile
    src/vmap2/Makefile
    src/vmap_assembler/Makefile
    src/vmap_debugger/Makefile
])

## Output files.
AC_OUTPUT
