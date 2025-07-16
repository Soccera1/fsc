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
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int verbose = 0;

int main(int argc, char *argv[]) {
  int launch_dwm = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dwm") == 0) {
      launch_dwm = 1;
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0) {
      verbose = 1;
    }
  }

  if (verbose) {
    fprintf(stderr, "Verbose mode enabled.\n");
  }

  struct compositor compositor;
  if (!compositor_init(&compositor)) {
    return 1;
  }

  if (launch_dwm) {
    if (fork() == 0) {
      char *const dwm_argv[] = {DWM_PATH, NULL};
      execvp(dwm_argv[0], dwm_argv);
      perror("execvp " DWM_PATH);
      return 1;
    }
  }

  compositor_run(&compositor);
  compositor_fini(&compositor);
  return 0;
}
