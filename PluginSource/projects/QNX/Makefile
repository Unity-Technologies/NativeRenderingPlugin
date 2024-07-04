# UNITY_QNX_TARGET_LIB allows to specify the path to the QNX target libraries
# e.g. for imx8qm, use "~/qnx710/target/qnx7/aarch64le/usr/lib/graphics/iMX8QM/"
UNITY_QNX_TARGET_LIB ?= $(QNX_TARGET)/aarch64le/usr/lib

# UNITY_QNX_GLES2_LIB allows to link against a device/driver-specific library
# e.g. for imx8qm, use "GLESv2_viv"
UNITY_QNX_GLES2_LIB ?= GLESv2

SRCDIR = ../../source
SRCS = $(SRCDIR)/RenderingPlugin.cpp \
$(SRCDIR)/RenderAPI.cpp \
$(SRCDIR)/RenderAPI_OpenGLCoreES.cpp
OBJS = ${SRCS:.cpp=.o}
UNITY_DEFINES = -DUNITY_QNX=1
CXXFLAGS = $(UNITY_DEFINES) -O2 -fPIC
LDFLAGS = -shared -rdynamic -static-libstdc++ -Wl,-soname,RenderingPlugin -Wl,-L$(UNITY_QNX_TARGET_LIB) -Wl,-l$(UNITY_QNX_GLES2_LIB) 
LIBS = 
PLUGIN_SHARED = libRenderingPlugin.so
CXX = ntoaarch64-g++

ifeq (, $(shell echo $(QNX_TARGET)))
$(error "QNX_TARGET is not set.")
endif

ifeq (, $(shell echo $(QNX_HOST)))
$(error "QNX_HOST is not set.")
endif

# Check if the CXX variable is set to a valid compiler
ifeq (, $(shell which $(CXX)))
$(error "No $(CXX) in PATH. Did you set '$$QNX_HOST/usr/bin' in your PATH?")
endif

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean shared

cleanobjs:
	rm -f $(OBJS)

cleanlib:
	rm -f $(PLUGIN_SHARED)

clean: cleanobjs cleanlib

shared: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(PLUGIN_SHARED) $(OBJS) $(LIBS)
