# This Makefile was created and edited by EXL, 28-Oct-2021
# Default MotoMAGX platform is Motorola ZINE ZN5

####### Config
NAME              := pushkey

####### SDK and Toolchains
TOOLPREFIX        := /arm-eabi
ARMLIB            := $(TOOLPREFIX)/arm-linux-gnueabi/lib

####### Default Platform
TARGET             = $(NAME)

####### Build Tools and Options
CC                 = $(TOOLPREFIX)/bin/arm-linux-gnueabi-gcc
STRIP              = $(TOOLPREFIX)/bin/arm-linux-gnueabi-strip
INCPATH            = -I$(TOOLPREFIX)/arm-linux-gnueabi/include
CFLAGS             = -Wall -W -O2 -DNO_DEBUG $(INCPATH)

####### Files
SOURCES            = pushkey.c
OBJECTS            = pushkey.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)
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
