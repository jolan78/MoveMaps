noinst_PROGRAMS = vmap-assembler

vmap_assembler_CPPFLAGS = \
-I$(top_srcdir)/dep/include \
-I$(top_srcdir)/src/vmap2

vmap_assembler_SOURCES = \
main.cpp

vmap_assembler_LDADD = $(top_builddir)/src/vmap2/libvmap2.a -lG3D -lzip -lz -lX11

vmap_assembler_LDFLAGS = \
-L$(top_builddir)/dep/src/zip \
-L$(top_builddir)/dep/src/G3D



