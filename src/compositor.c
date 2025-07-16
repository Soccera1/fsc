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
#include "config.h"
#include <X11/XKBlib.h>
#include <stdio.h>
#include <stdlib.h>

#define CLEANMASK(mask) (mask & ~(LockMask | Mod2Mask))

static void keypress(struct compositor *compositor, XEvent *e);
static void grabkeys(struct compositor *compositor);

static void compositor_get_outputs(struct compositor *compositor) {
  XRRScreenResources *res =
      XRRGetScreenResources(compositor->display, compositor->root);
  if (res == NULL) {
    fprintf(stderr, "Cannot get screen resources\n");
    return;
  }

  compositor->output_count = res->noutput;
  DPRINTF("Found %d outputs.\n", compositor->output_count);
  compositor->outputs =
      calloc(compositor->output_count, sizeof(struct output));

  for (int i = 0; i < res->noutput; i++) {
    XRROutputInfo *output_info =
        XRRGetOutputInfo(compositor->display, res, res->outputs[i]);
    if (output_info == NULL) {
      continue;
    }

    compositor->outputs[i].name = output_info->name;
    if (output_info->crtc) {
      XRRCrtcInfo *crtc_info =
          XRRGetCrtcInfo(compositor->display, res, output_info->crtc);
      if (crtc_info) {
        compositor->outputs[i].width = crtc_info->width;
        compositor->outputs[i].height = crtc_info->height;
        DPRINTF("  Output %d (%s): %dx%d\n", i, compositor->outputs[i].name,
                compositor->outputs[i].width, compositor->outputs[i].height);
        XRRFreeCrtcInfo(crtc_info);
      }
    }
    compositor->outputs[i].scale = DEFAULT_SCALE; // Default to 100%

    XRRFreeOutputInfo(output_info);
  }

  XRRFreeScreenResources(res);
}

int compositor_init(struct compositor *compositor) {
  DPRINTF("Initializing compositor...\n");
  compositor->display = XOpenDisplay(NULL);
  if (compositor->display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return 0;
  }
  compositor->root = DefaultRootWindow(compositor->display);
  compositor->clients = NULL;
  compositor->client_count = 0;
  compositor->running = 1;

  int composite_event_base, composite_error_base;
  if (!XCompositeQueryExtension(compositor->display, &composite_event_base,
                                &composite_error_base)) {
    fprintf(stderr, "XComposite extension not found\n");
    return 0;
  }
  DPRINTF("XComposite extension found.\n");

  XCompositeRedirectSubwindows(compositor->display, compositor->root,
                               CompositeRedirectAutomatic);
  grabkeys(compositor);
  compositor_get_outputs(compositor);
  DPRINTF("Compositor initialized successfully.\n");
  return 1;
}

static void compositor_redraw(struct compositor *compositor, Window window) {
  DPRINTF("Redrawing window 0x%lx\n", window);
  for (int i = 0; i < compositor->client_count; i++) {
    if (compositor->clients[i].window == window) {
      XWindowAttributes attr;
      XGetWindowAttributes(compositor->display, window, &attr);

      Picture source_picture = XRenderCreatePicture(
          compositor->display, window,
          XRenderFindVisualFormat(compositor->display, attr.visual), 0, NULL);

      Pixmap temp_pixmap = XCreatePixmap(compositor->display, window,
                                         attr.width, attr.height, attr.depth);
      XRenderPictFormat *format =
          XRenderFindVisualFormat(compositor->display, attr.visual);
      Picture temp_picture =
          XRenderCreatePicture(compositor->display, temp_pixmap, format, 0, NULL);

      XTransform transform;
      transform.matrix[0][0] = XDoubleToFixed(1.0);
      transform.matrix[0][1] = XDoubleToFixed(0.0);
      transform.matrix[0][2] = XDoubleToFixed(0.0);
      transform.matrix[1][0] = XDoubleToFixed(0.0);
      transform.matrix[1][1] = XDoubleToFixed(1.0);
      transform.matrix[1][2] = XDoubleToFixed(0.0);
      transform.matrix[2][0] = XDoubleToFixed(0.0);
      transform.matrix[2][1] = XDoubleToFixed(0.0);
      transform.matrix[2][2] =
          XDoubleToFixed((double)compositor->clients[i].scale / 100.0);
      XRenderSetPictureTransform(compositor->display, source_picture, &transform);

      XRenderComposite(compositor->display, PictOpSrc, source_picture, None,
                       temp_picture, 0, 0, 0, 0, 0, 0, attr.width, attr.height);

      XCopyArea(compositor->display, temp_pixmap, window,
                XDefaultGC(compositor->display, 0), 0, 0, attr.width,
                attr.height, 0, 0);

      XFreePixmap(compositor->display, temp_pixmap);
      XRenderFreePicture(compositor->display, temp_picture);
      XRenderFreePicture(compositor->display, source_picture);
      break;
    }
  }
}

void compositor_run(struct compositor *compositor) {
  XEvent event;
  while (compositor->running) {
    XNextEvent(compositor->display, &event);
    switch (event.type) {
    case Expose:
      compositor_redraw(compositor, event.xexpose.window);
      break;
    case KeyPress:
      keypress(compositor, &event);
      break;
    }
  }
}

void compositor_fini(struct compositor *compositor) {
  DPRINTF("Finalizing compositor...\n");
  free(compositor->outputs);
  free(compositor->clients);
  XCloseDisplay(compositor->display);
  DPRINTF("Compositor finalized.\n");
}

void quit(const Arg *arg) {
  DPRINTF("Quit function called.\n");
  struct compositor *compositor = (struct compositor *)arg->v;
  compositor->running = 0;
}

void keypress(struct compositor *compositor, XEvent *e) {
  unsigned int i;
  KeySym keysym;
  XKeyEvent *ev;

  ev = &e->xkey;
  keysym = XkbKeycodeToKeysym(compositor->display, ev->keycode, 0, 0);
  DPRINTF("Keypress event: keysym=0x%lx, state=0x%x\n", keysym, ev->state);

  for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    DPRINTF("  Checking against key: keysym=0x%lx, mod=0x%x\n",
            keys[i].keysym, keys[i].mod);
    if (keysym == keys[i].keysym &&
        CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func) {
      DPRINTF("    Match found! Calling function.\n");
      Arg arg = keys[i].arg;
      arg.v = compositor;
      keys[i].func(&arg);
    }
  }
}

void grabkeys(struct compositor *compositor) {
  unsigned int i, j;
  unsigned int modifiers[] = {0, LockMask, Mod2Mask, Mod2Mask | LockMask};
  KeyCode code;

  DPRINTF("Grabbing keys...\n");
  XUngrabKey(compositor->display, AnyKey, AnyModifier, compositor->root);
  for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    if ((code = XKeysymToKeycode(compositor->display, keys[i].keysym))) {
      DPRINTF("  Grabbing key: keysym=0x%lx, mod=0x%x\n", keys[i].keysym,
              keys[i].mod);
      for (j = 0; j < sizeof(modifiers) / sizeof(modifiers[0]); j++) {
        XGrabKey(compositor->display, code, keys[i].mod | modifiers[j],
                 compositor->root, True, GrabModeAsync, GrabModeAsync);
      }
    }
  }
}