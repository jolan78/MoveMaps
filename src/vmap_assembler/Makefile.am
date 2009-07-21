noinst_PROGRAMS = vmap-assembler

vmap_assembler_CPPFLAGS = \
-I$(top_srcdir)/dep/include \
-I$(top_srcdir)/src/vmap2

vmap_assembler_SOURCES = \
main.cpp

vmap_assembler_LDADD = $(top_builddir)/src/vmap2/libvmap2.a @DISPLAY_MANAGER_LIBS@ -lG3D -lzip -lz @WINOW_MANAGER_LIBS@

vmap_assembler_LDFLAGS = \
@DISPLAY_MANAGER_LDFLAGS@ \
-L$(top_builddir)/dep/src/zip \
-L$(top_builddir)/dep/src/G3D



