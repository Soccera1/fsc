/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include <X11/keysym.h>

#define MODKEY Mod4Mask
#define TAGKEYS(KEY, TAG)                                                      \
  {MODKEY, KEY, view, {.ui = 1 << TAG}},                                       \
      {MODKEY | ControlMask, KEY, toggleview, {.ui = 1 << TAG}},               \
      {MODKEY | ShiftMask, KEY, tag, {.ui = 1 << TAG}},                        \
      {MODKEY | ControlMask | ShiftMask, KEY, toggletag, {.ui = 1 << TAG}},

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd)                                                             \
  {                                                                            \
    .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL }                       \
  }

/* Default scaling factor (e.g., 125 for 125%) */
#define DEFAULT_SCALE 100

/* Default screen resolution for Xephyr */
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

static void quit(const Arg *arg);

static Key keys[] = {
    {MODKEY | ShiftMask, XK_q, quit, {0}},
};
