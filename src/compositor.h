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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

struct output {
    char *name;
    int width;
    int height;
    int scale; // e.g. 125 for 125%
};

#include <X11/extensions/Xcomposite.h>

struct client {
    Window window;
    Picture picture; // Picture for the redirected window content
    int scale;
};

struct compositor {
    Display *display;
    Window root;
    int output_count;
    struct output *outputs;
    struct client *clients;
    int client_count;
};

int compositor_init(struct compositor *compositor);
void compositor_run(struct compositor *compositor);
void compositor_fini(struct compositor *compositor);

#endif // COMPOSITOR_H
