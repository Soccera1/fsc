CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I.
LDFLAGS = -lX11 -lXrandr -lXrender -lXcomposite
PREFIX ?= /usr/local

DWM_PATH ?= $(shell which dwm)
ifeq ($(DWM_PATH),)
$(warning "dwm not found in PATH, dwm integration will likely fail")
CFLAGS += -DDWM_PATH="\"dwm\""
else
CFLAGS += -DDWM_PATH="\"$(DWM_PATH)\""
endif

# Enable Xephyr support by passing XEPHYR_SUPPORT=1
ifeq ($(XEPHYR_SUPPORT),1)
CFLAGS += -DXEPHYR_SUPPORT
endif

SRC = src/main.c src/compositor.c
OBJ = $(SRC:.c=.o)
EXEC = fsc

all: config.h $(EXEC)

config.h: config.def.h
	cp config.def.h config.h

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

$(OBJ): config.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(EXEC) $(DESTDIR)$(PREFIX)/bin/$(EXEC)
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(EXEC)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXEC)

run: $(EXEC)
	Xephyr :1 -screen $(SCREEN_WIDTH)x$(SCREEN_HEIGHT) & sleep 1 && DISPLAY=:1 ./fsc

dwm: $(EXEC)
	Xephyr :1 -screen $(SCREEN_WIDTH)x$(SCREEN_HEIGHT) & sleep 1 && DISPLAY=:1 ./fsc & sleep 2 && DISPLAY=:1 dwm

.PHONY: start-dwm
start-dwm: $(EXEC)
	xinit $(CURDIR)/$(EXEC) --dwm -- :1
