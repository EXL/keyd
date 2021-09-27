# This Makefile was edited by EXL, 28-Sep-2021
# Default MotoMAGX platform is Motorola ZINE ZN5

####### Config
LIBNAME     := libredkey.so

####### SDK and Toolchains
TOOLPREFIX  := /arm-eabi
ARMLIB      := $(TOOLPREFIX)/arm-linux-gnueabi/lib

####### Default Platform
QTDIR       := $(TOOLPREFIX)/lib/qt-zn5
EZXDIR      := $(TOOLPREFIX)/lib/ezx-zn5
LINKLIB     := -ldl -lqte-mt
TARGET       = $(LIBNAME)

####### Build Tools and Options
MOC          = $(QTDIR)/bin/moc
CXX          = $(TOOLPREFIX)/bin/arm-linux-gnueabi-g++
STRIP        = $(TOOLPREFIX)/bin/arm-linux-gnueabi-strip
INCPATH      = -I. -I$(QTDIR)/include -I$(EZXDIR)/include -I $(TOOLPREFIX)/arm-linux-gnueabi/include
CXXFLAGS     = -pipe -fPIC -fno-exceptions -fno-rtti -Wall -W -O2 -DQWS -DNO_DEBUG $(INCPATH)
LDLAGS       = -shared -Wl,-soname,$(TARGET) -Wl,-rpath-link,$(EZXDIR)/lib
LIBS         = -L$(EZXDIR)/lib -L$(ARMLIB) -L$(QTDIR)/lib $(LINKLIB)

####### Files
SOURCES      = RedKey.cpp
OBJECTS      = RedKey.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	$(STRIP) -s $(TARGET)

tar: $(TARGET)
	-tar -cvf $(TARGET).tar $(TARGET)

rmobj:
	-rm -f $(OBJECTS)
	-rm -f *~ core
	$(RM) *.o

clean: rmobj
	$(RM) $(TARGET) *.tar
