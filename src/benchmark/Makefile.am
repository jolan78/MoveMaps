noinst_PROGRAMS = mmap-benchmark

mmap_benchmark_CPPFLAGS = \
-I$(top_srcdir)/dep/include \
-I$(top_srcdir)/src/vmap \
-I$(top_srcdir)/src/mmap \
-I$(srcdir)/src

mmap_benchmark_SOURCES = \
main.cpp

mmap_benchmark_LDADD = @DISPLAY_MANAGER_LIBS@ -lmmaps -lvmaps -lGLG3D -lG3D -lzip -ljpeg -lpng12 -lGLU

mmap_benchmark_LDFLAGS = @DISPLAY_MANAGER_LDFLAGS@ \
-L$(top_builddir)/dep/src/GLG3D \
-L$(top_builddir)/dep/src/zip \
-L$(top_builddir)/dep/src/G3D \
-L$(top_builddir)/src/vmap \
-L$(top_builddir)/src/mmap



