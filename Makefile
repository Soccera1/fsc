CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lX11 -lXrandr -lXrender -lXcomposite

SRC = src/main.c src/compositor.c
OBJ = $(SRC:.c=.o)
EXEC = fsc

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

run: $(EXEC)
	Xephyr :1 -screen 1024x768 & sleep 1 && DISPLAY=:1 ./fsc

dwm: $(EXEC)
	Xephyr :1 -screen 1024x768 & sleep 1 && DISPLAY=:1 ./fsc & sleep 2 && DISPLAY=:1 dwm