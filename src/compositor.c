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

#include "compositor.h"
#include <stdio.h>
#include <stdlib.h>

static void compositor_get_outputs(struct compositor *compositor) {
    XRRScreenResources *res = XRRGetScreenResources(compositor->display, compositor->root);
    if (res == NULL) {
        fprintf(stderr, "Cannot get screen resources\n");
        return;
    }

    compositor->output_count = res->noutput;
    compositor->outputs = calloc(compositor->output_count, sizeof(struct output));

    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo *output_info = XRRGetOutputInfo(compositor->display, res, res->outputs[i]);
        if (output_info == NULL) {
            continue;
        }

        compositor->outputs[i].name = output_info->name;
        if (output_info->crtc) {
            XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(compositor->display, res, output_info->crtc);
            if (crtc_info) {
                compositor->outputs[i].width = crtc_info->width;
                compositor->outputs[i].height = crtc_info->height;
                XRRFreeCrtcInfo(crtc_info);
            }
        }
        compositor->outputs[i].scale = 100; // Default to 100%

        XRRFreeOutputInfo(output_info);
    }

    XRRFreeScreenResources(res);
}

int compositor_init(struct compositor *compositor) {
    compositor->display = XOpenDisplay(NULL);
    if (compositor->display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 0;
    }
    compositor->root = DefaultRootWindow(compositor->display);
    compositor->clients = NULL;
    compositor->client_count = 0;
    XSelectInput(compositor->display, compositor->root, SubstructureRedirectMask | SubstructureNotifyMask);

    compositor_get_outputs(compositor);
    return 1;
}

static void compositor_handle_map_request(struct compositor *compositor, XMapRequestEvent *event) {
    XWindowAttributes attr;
    XGetWindowAttributes(compositor->display, event->window, &attr);

    int scale = 100; // Default scale
    // Find the output the window is on and get its scale
    for (int i = 0; i < compositor->output_count; i++) {
        // A more robust method would be needed for multi-monitor setups
        scale = compositor->outputs[i].scale;
        break;
    }

    int scaled_width = attr.width * scale / 100;
    int scaled_height = attr.height * scale / 100;

    Pixmap pixmap = XCreatePixmap(compositor->display, event->window, scaled_width, scaled_height, attr.depth);

    XRenderPictFormat *format = XRenderFindVisualFormat(compositor->display, attr.visual);
    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    Picture picture = XRenderCreatePicture(compositor->display, event->window, format, CPSubwindowMode, &pa);

    compositor->clients = realloc(compositor->clients, sizeof(struct client) * (compositor->client_count + 1));
    compositor->clients[compositor->client_count].window = event->window;
    compositor->clients[compositor->client_count].pixmap = pixmap;
    compositor->clients[compositor->client_count].picture = picture;
    compositor->clients[compositor->client_count].scale = scale;
    compositor->client_count++;

    XMapWindow(compositor->display, event->window);
}

static void compositor_handle_configure_request(struct compositor *compositor, XConfigureRequestEvent *event) {
    // Find the client
    for (int i = 0; i < compositor->client_count; i++) {
        if (compositor->clients[i].window == event->window) {
            int scale = compositor->clients[i].scale;
            int scaled_width = event->width * scale / 100;
            int scaled_height = event->height * scale / 100;

            XFreePixmap(compositor->display, compositor->clients[i].pixmap);
            XWindowAttributes attr;
            XGetWindowAttributes(compositor->display, event->window, &attr);
            compositor->clients[i].pixmap = XCreatePixmap(compositor->display, event->window, scaled_width, scaled_height, attr.depth);
            break;
        }
    }

    XWindowChanges wc;
    wc.x = event->x;
    wc.y = event->y;
    wc.width = event->width;
    wc.height = event->height;
    wc.border_width = event->border_width;
    wc.sibling = event->above;
    wc.stack_mode = event->detail;
    XConfigureWindow(compositor->display, event->window, event->value_mask, &wc);
}

static void compositor_redraw(struct compositor *compositor, Window window) {
    for (int i = 0; i < compositor->client_count; i++) {
        if (compositor->clients[i].window == window) {
            XWindowAttributes attr;
            XGetWindowAttributes(compositor->display, window, &attr);

            // Create a temporary pixmap and picture for scaling
            Pixmap temp_pixmap = XCreatePixmap(compositor->display, window, attr.width, attr.height, attr.depth);
            XRenderPictFormat *format = XRenderFindVisualFormat(compositor->display, attr.visual);
            Picture temp_picture = XRenderCreatePicture(compositor->display, temp_pixmap, format, 0, NULL);

            // Set the transform
            XTransform transform;
            transform.matrix[0][0] = XDoubleToFixed(1.0);
            transform.matrix[0][1] = XDoubleToFixed(0.0);
            transform.matrix[0][2] = XDoubleToFixed(0.0);
            transform.matrix[1][0] = XDoubleToFixed(0.0);
            transform.matrix[1][1] = XDoubleToFixed(1.0);
            transform.matrix[1][2] = XDoubleToFixed(0.0);
            transform.matrix[2][0] = XDoubleToFixed(0.0);
            transform.matrix[2][1] = XDoubleToFixed(0.0);
            transform.matrix[2][2] = XDoubleToFixed((double)compositor->clients[i].scale / 100.0);
            XRenderSetPictureTransform(compositor->display, compositor->clients[i].picture, &transform);

            // Scale the picture
            XRenderComposite(compositor->display, PictOpSrc, compositor->clients[i].picture, None, temp_picture, 0, 0, 0, 0, 0, 0, attr.width, attr.height);

            // Copy the scaled picture to the window
            XCopyArea(compositor->display, temp_pixmap, window, XDefaultGC(compositor->display, 0), 0, 0, attr.width, attr.height, 0, 0);

            // Free the temporary pixmap and picture
            XFreePixmap(compositor->display, temp_pixmap);
            XRenderFreePicture(compositor->display, temp_picture);
            break;
        }
    }
}

void compositor_run(struct compositor *compositor) {
    XEvent event;
    while (1) {
        XNextEvent(compositor->display, &event);
        switch (event.type) {
            case MapRequest:
                compositor_handle_map_request(compositor, &event.xmaprequest);
                break;
            case ConfigureRequest:
                compositor_handle_configure_request(compositor, &event.xconfigurerequest);
                break;
            case Expose:
                compositor_redraw(compositor, event.xexpose.window);
                break;
        }
    }
}

void compositor_fini(struct compositor *compositor) {
    free(compositor->outputs);
    free(compositor->clients);
    XCloseDisplay(compositor->display);
}