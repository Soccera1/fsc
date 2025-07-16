/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FSC_COMPOSITOR_H
#define FSC_COMPOSITOR_H

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>

extern int verbose;

#define DPRINTF(...)                                                           \
  if (verbose) {                                                               \
    fprintf(stderr, __VA_ARGS__);                                              \
  }

struct output {
  char *name;
  int width;
  int height;
  int scale;
};

typedef union {
  int i;
  unsigned int ui;
  float f;
  const void *v;
} Arg;

struct client {
  Window window;
  int scale;
};

typedef struct {
  unsigned int mod;
  KeySym keysym;
  void (*func)(const Arg *);
  const Arg arg;
} Key;

struct compositor {
  Display *display;
  Window root;
  struct client *clients;
  int client_count;
  struct output *outputs;
  int output_count;
  int running;
};

int compositor_init(struct compositor *compositor);
void compositor_run(struct compositor *compositor);
void compositor_fini(struct compositor *compositor);

#endif // FSC_COMPOSITOR_H

