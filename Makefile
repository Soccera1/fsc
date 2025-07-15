CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I.
LDFLAGS = -lX11 -lXrandr -lXrender -lXcomposite

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

run: $(EXEC)
	Xephyr :1 -screen $(SCREEN_WIDTH)x$(SCREEN_HEIGHT) & sleep 1 && DISPLAY=:1 ./fsc

dwm: $(EXEC)
	Xephyr :1 -screen $(SCREEN_WIDTH)x$(SCREEN_HEIGHT) & sleep 1 && DISPLAY=:1 ./fsc & sleep 2 && DISPLAY=:1 dwm
