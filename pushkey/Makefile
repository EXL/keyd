# This Makefile was edited by EXL, 26-Oct-2021
# Default MotoMAGX platform is Motorola ZINE ZN5

####### Config
NAME              := pushkey

####### SDK and Toolchains
TOOLPREFIX        := /arm-eabi
ARMLIB            := $(TOOLPREFIX)/arm-linux-gnueabi/lib

####### Default Platform
QTDIR             := $(TOOLPREFIX)/lib/qt-zn5
EZXDIR            := $(TOOLPREFIX)/lib/ezx-zn5
LINKLIB           := # -lqte-mt
TARGET             = $(NAME)

####### Build Tools and Options
MOC                = $(QTDIR)/bin/moc
CXX                = $(TOOLPREFIX)/bin/arm-linux-gnueabi-g++
STRIP              = $(TOOLPREFIX)/bin/arm-linux-gnueabi-strip
INCPATH            = -I. -I$(QTDIR)/include -I$(EZXDIR)/include -I$(TOOLPREFIX)/arm-linux-gnueabi/include
CXXFLAGS           = -pipe -fPIC -fno-exceptions -fno-rtti -Wall -W -O2 -DQWS -DNO_DEBUG $(INCPATH)
LDLAGS             = -Wl,-rpath-link,$(EZXDIR)/lib
LIBS               = -L$(EZXDIR)/lib -L$(ARMLIB) -L$(QTDIR)/lib $(LINKLIB)

####### Files
SOURCES            = pushkey.cpp
OBJECTS            = pushkey.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	$(STRIP) -s $(TARGET)

tar: $(TARGET)
	-tar -cvf $(NAME).tar $(TARGET)

zip: $(TARGET)
	-zip -r -9 $(NAME).zip $(TARGET)

rmobj:
	-rm -f $(OBJECTS)
	-rm -f *~ core
	$(RM) *.o

clean: rmobj
	$(RM) $(TARGET) *.tar *.zip
