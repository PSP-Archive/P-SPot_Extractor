TARGET = P-SPot_Extractor
OBJS = ioapi.o unzip.o unzipToDir.o config.o main.o

#To build for custom firmware:
BUILD_PRX = 1
PSP_FW_VERSION=371

#CFLAGS = -O3 -G0 -Wall
CFLAGS = -Os -frename-registers -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

LIBS = -lpspsdk -lz
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = P-SPot Extractor
#PSP_EBOOT_ICON = ICON0.PNG
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
