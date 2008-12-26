noinst_PROGRAMS = vmap-viewer

vmap_viewer_CPPFLAGS = \
-I$(top_srcdir)/dep/include \
-I$(top_srcdir)/src/vmap2 

vmap_viewer_SOURCES = \
main.cpp \
ViewApp.cpp \
ViewApp.h

vmap_viewer_LDADD = \
$(top_builddir)/src/vmap2/libvmap2.a \
-lGLG3D -lG3D -lSDL -lSDLmain -lzip -ljpeg -lpng12 -lGLU -lX11 

vmap_viewer_LDFLAGS = \
-L$(top_builddir)/dep/src/GLG3D \
-L$(top_builddir)/dep/src/zip \
-L$(top_builddir)/dep/src/G3D 
