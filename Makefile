CC ?= gcc
CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
LIBS = $(shell pkg-config --libs gtk+-3.0) -lm -lsoundio
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

SRC = main.c delune-application.c delune-window.c ui.c delune-signal-component.c delune-signal-chooser.c signal.c sound.c utils.c
SRC_UI = ui/delune-window.ui ui/delune-signal-chooser.ui ui/delune-signal-component.ui
BUILT_SRC = resources.c

OBJS = $(BUILT_SRC:.c=.o) $(SRC:.c=.o)

all : delune

resources.c : delune.gresource.xml $(SRC_UI)
	$(GLIB_COMPILE_RESOURCES) delune.gresource.xml --target=$@ --generate-source

%.o : %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

delune : $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean :
	rm -f $(BUILT_SRC)
	rm -f $(OBJS)
	rm -f delune

