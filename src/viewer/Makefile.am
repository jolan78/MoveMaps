noinst_PROGRAMS = mmap-viewer

mmap_viewer_CPPFLAGS = \
-I$(top_srcdir)/dep/include \
-I$(top_srcdir)/src/vmap \
-I$(top_srcdir)/src/mmap \
-I$(srcdir)/src

mmap_viewer_SOURCES = \
main.cpp \
ModelContainerView.h \
ModelContainerView.cpp

mmap_viewer_LDADD = -lSDL -lSDLmain -lmmaps -lvmaps -lGLG3D -lG3D -lSDL -lSDLmain -lzip -ljpeg -lpng12 -lGLU

mmap_viewer_LDFLAGS = \
-L$(top_builddir)/dep/src/GLG3D \
-L$(top_builddir)/dep/src/zip \
-L$(top_builddir)/dep/src/G3D \
-L$(top_builddir)/src/vmap \
-L$(top_builddir)/src/mmap



