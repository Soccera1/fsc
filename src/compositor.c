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

    int composite_event_base, composite_error_base;
    if (!XCompositeQueryExtension(compositor->display, &composite_event_base, &composite_error_base)) {
        fprintf(stderr, "XComposite extension not found\n");
        return 0;
    }

    XCompositeRedirectSubwindows(compositor->display, compositor->root, CompositeRedirectAutomatic);

    compositor_get_outputs(compositor);
    return 1;
}



static void compositor_redraw(struct compositor *compositor, Window window) {
    for (int i = 0; i < compositor->client_count; i++) {
        if (compositor->clients[i].window == window) {
            XWindowAttributes attr;
            XGetWindowAttributes(compositor->display, window, &attr);

            // Get the redirected window content as a Picture
            // This is the key change: we're using the window itself as the source for the picture
            // because XCompositeRedirectSubwindows makes its content available.
            Picture source_picture = XRenderCreatePicture(compositor->display, window, XRenderFindVisualFormat(compositor->display, attr.visual), 0, NULL);

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
            XRenderSetPictureTransform(compositor->display, source_picture, &transform);

            // Scale the picture
            XRenderComposite(compositor->display, PictOpSrc, source_picture, None, temp_picture, 0, 0, 0, 0, 0, 0, attr.width, attr.height);

            // Copy the scaled picture to the window
            XCopyArea(compositor->display, temp_pixmap, window, XDefaultGC(compositor->display, 0), 0, 0, attr.width, attr.height, 0, 0);

            // Free the temporary pixmap and picture
            XFreePixmap(compositor->display, temp_pixmap);
            XRenderFreePicture(compositor->display, temp_picture);
            XRenderFreePicture(compositor->display, source_picture); // Free the source picture created from the window
            break;
        }
    }
}

void compositor_run(struct compositor *compositor) {
    XEvent event;
    while (1) {
        XNextEvent(compositor->display, &event);
        switch (event.type) {
            case Expose:
                compositor_redraw(compositor, event.xexpose.window);
                break;
            // Add handlers for CompositeRedirect events if needed, but Expose should trigger redraw.
            // For now, we'll rely on Expose events to trigger redrawing.
        }
    }
}

void compositor_fini(struct compositor *compositor) {
    free(compositor->outputs);
    free(compositor->clients);
    XCloseDisplay(compositor->display);
}