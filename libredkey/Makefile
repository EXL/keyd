# This Makefile was edited by EXL, 28-Sep-2021
# Default MotoMAGX platform is Motorola ZINE ZN5

####### Config
NAME              := libredkey
LIBNAME           := $(NAME).so
LIBNAME_NO_CONFIG := $(NAME)_no_config.so
CONFIG            := $(NAME).cfg

####### SDK and Toolchains
TOOLPREFIX        := /arm-eabi
ARMLIB            := $(TOOLPREFIX)/arm-linux-gnueabi/lib

####### Default Platform
QTDIR             := $(TOOLPREFIX)/lib/qt-zn5
EZXDIR            := $(TOOLPREFIX)/lib/ezx-zn5
LINKLIB           := -ldl -lqte-mt
TARGET             = $(LIBNAME)
TARGET_NO_CONFIG   = $(LIBNAME_NO_CONFIG)

####### Build Tools and Options
MOC                = $(QTDIR)/bin/moc
CXX                = $(TOOLPREFIX)/bin/arm-linux-gnueabi-g++
STRIP              = $(TOOLPREFIX)/bin/arm-linux-gnueabi-strip
INCPATH            = -I. -I$(QTDIR)/include -I$(EZXDIR)/include -I$(TOOLPREFIX)/arm-linux-gnueabi/include
CXXFLAGS           = -pipe -fPIC -fno-exceptions -fno-rtti -Wall -W -O2 -DQWS -DNO_DEBUG $(INCPATH)
CXXFLAGS_NO_CONFIG = -Wno-unused-function -DNO_CONFIG 
LDLAGS             = -shared -Wl,-soname,$(TARGET) -Wl,-rpath-link,$(EZXDIR)/lib
LIBS               = -L$(EZXDIR)/lib -L$(ARMLIB) -L$(QTDIR)/lib $(LINKLIB)

####### Files
SOURCES            = libredkey.cpp
OBJECTS            = libredkey.o
OBJECTS_NO_CONFIG  = libredkey_no_config.o

all: $(TARGET) $(TARGET_NO_CONFIG)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	$(STRIP) -s $(TARGET)

$(TARGET_NO_CONFIG): $(OBJECTS_NO_CONFIG)
	$(CXX) $(LDLAGS) -o $(TARGET_NO_CONFIG) $(OBJECTS_NO_CONFIG) $(LIBS)
	$(STRIP) -s $(TARGET_NO_CONFIG)

tar: $(TARGET) $(TARGET_NO_CONFIG)
	-tar -cvf $(NAME).tar $(TARGET) $(TARGET_NO_CONFIG) $(CONFIG)

zip: $(TARGET) $(TARGET_NO_CONFIG)
	-zip -r -9 $(NAME).zip $(TARGET) $(TARGET_NO_CONFIG) $(CONFIG)

rmobj:
	-rm -f $(OBJECTS)
	-rm -f *~ core
	$(RM) *.o

clean: rmobj
	$(RM) $(TARGET) $(TARGET_NO_CONFIG) *.tar *.zip

libredkey_no_config.o: libredkey.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_NO_CONFIG) -c -o libredkey_no_config.o libredkey.cpp
