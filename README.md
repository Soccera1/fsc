# X Fractional Scaling Compositor (fsc)

`fsc` is a skeleton X compositor designed to handle fractional scaling using a method of scaling up and then scaling down. This project serves as a basic example and starting point for implementing more advanced fractional scaling techniques in an X environment.

## Features

- Basic X compositor functionality.
- Fractional scaling using XRender (scaling up a pixmap and then scaling down).
- Integration with Xephyr for easy testing in a nested X server.

## Building

To build the compositor, you need the following development libraries:

- `libx11-dev`
- `libxrandr-dev`
- `libxrender-dev`

On Debian/Ubuntu, you can install them using:

```bash
sudo apt-get install libx11-dev libxrandr-dev libxrender-dev
```

Once the dependencies are installed, navigate to the project root directory and run `make`:

```bash
make
```

This will compile the `fsc` executable.

## Running and Testing

`fsc` is designed to be run within a nested X server like Xephyr for safe testing without affecting your primary desktop environment.

First, ensure you have `Xephyr` installed. On Debian/Ubuntu:

```bash
sudo apt-get install xserver-xephyr
```

To run the compositor and open a Xephyr window, use the `run` target in the Makefile:

```bash
make run
```

This command will:
1. Start a Xephyr instance on display `:1` with a resolution of 1024x768.
2. Introduce a 1-second delay to allow Xephyr to initialize.
3. Launch `fsc` on the `:1` display.

You will see a black Xephyr window appear. This window is now being managed by your `fsc` compositor.

### Testing Fractional Scaling

By default, the compositor is set to 100% scaling. To test fractional scaling:

1.  Open `src/compositor.c`.
2.  Locate the line:
    ```c
    compositor->outputs[i].scale = 100; // Default to 100%
    ```
3.  Change `100` to your desired scaling factor (e.g., `125` for 125%, `150` for 150%).
4.  Save the file.
5.  Stop the running compositor (if any) by pressing `Ctrl+C` in the terminal where `make run` is active.
6.  Recompile the project:
    ```bash
    make
    ```
7.  Restart the compositor:
    ```bash
    make run
    ```

Now, open a **new terminal window** (do not close the one running `make run`) and launch an X application, directing it to the Xephyr display:

```bash
DISPLAY=:1 xterm
# Or for st:
DISPLAY=:1 st
# Or for xclock:
DISPLAY=:1 xclock
```

The application window should appear inside the Xephyr window, scaled according to the factor you set in `src/compositor.c`.


## License

This project is licensed under the GNU General Public License v3.0. See the `LICENSE` file for more details.
