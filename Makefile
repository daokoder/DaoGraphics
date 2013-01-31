PROJECT = dao_graphics

include ../../Makefile.common

DAO_INCS += -I../../kernel
DAO_LIBS += -L../..

GL_INC =

# Build libdao_graphics target, so that other program or module can link against it:
TARGET = lib$(PROJECT).$(DAO_DLL_EXT)

UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
  DAO_CFLAGS += -DUNIX
  DAO_LFLAGS += -fPIC -shared -Wl,-soname,lib$(PROJECT).so
  GL_INC += -I/usr/include/GL
  DAO_LFLAGS += -lGL -lGLU
  DAO_LFLAGS += -lglut
endif

ifeq ($(UNAME), Darwin)
  TARGET = lib$(PROJECT).dylib
  DAO_CFLAGS += -DUNIX -DMAC_OSX
  DAO_LFLAGS += -fPIC -dynamiclib -undefined dynamic_lookup -install_name lib$(PROJECT).dylib

  GL_INC += -I/System/Library/Frameworks/OpenGL.framework/Headers
  GL_INC += -I/System/Library/Frameworks/GLUT.framework/Headers
  DAO_LIBS += -framework OpenGL
  DAO_LIBS += -framework GLUT
endif

DAO_INCS += $(GL_INC)

OBJECTS  = source/dao_common.o
OBJECTS += source/dao_font.o source/dao_image.o
OBJECTS += source/dao_path.o source/dao_canvas.o
OBJECTS += source/dao_mesh.o source/dao_scene.o
OBJECTS += source/dao_triangulator.o
OBJECTS += source/dao_painter.o
OBJECTS += source/dao_renderer.o
OBJECTS += source/dao_graphics.o
OBJECTS += source/dao_resource.o source/dao_xml.o
OBJECTS += source/dao_collada.o
OBJECTS += source/dao_opengl.o source/dao_glut.o



all: $(TARGET)

.c.o:
	$(DAO_CC) -c $(DAO_CFLAGS) $(DAO_INCS) -o $@ $<

.cpp.o:
	$(DAO_CXX) -c $(DAO_CFLAGS) $(DAO_INCS) -o $@ $<

	
$(TARGET): $(OBJECTS)
	$(DAO_CC) $(DAO_DLLFLAGS) $(OBJECTS) $(DAO_LIBS) $(DAO_LFLAGS) -o $(TARGET)

#test: glsl-150-shader.o textfile.o glew/src/glew.o
#	$(DAO_CC) glsl-150-shader.o textfile.o glew/src/glew.o $(DAO_LIBS) -o glsl-test

clean:
	-$(DEL_FILE) $(OBJECTS)
