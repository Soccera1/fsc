# X Fractional Scaling Compositor (fsc)

## Current Status

**Disclaimer:** This project is currently a work in progress and does not yet implement fractional scaling as intended. At the moment, it primarily serves as a foundation for testing and development, with the following capabilities:

- Spawning a new window on a separate TTY.
- Launching the `dwm` window manager within the compositor's environment.

The core fractional scaling functionality is under active development. I am currently dogfooding the project by using the `dwm` integration for my daily workflow.

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
- `libxcomposite-dev`

On Debian/Ubuntu, you can install them using:

```bash
sudo apt-get install libx11-dev libxrandr-dev libxrender-dev libxcomposite-dev
```

### Configuration

The compositor's behavior can be configured via `config.h`. A default configuration file, `config.def.h`, is provided. To create your `config.h`, simply copy `config.def.h`:

```bash
cp config.def.h config.h
```

You can then edit `config.h` to adjust parameters such as:

- `DEFAULT_SCALE`: The default scaling factor (e.g., `125` for 125%).
- `SCREEN_WIDTH`, `SCREEN_HEIGHT`: The default resolution for the Xephyr window.

After making changes to `config.h`, you need to recompile the compositor.

Once the dependencies are installed and `config.h` is set up, navigate to the project root directory and run `make`:

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
1. Start a Xephyr instance on display `:1` with the resolution defined in `config.h`.
2. Introduce a 1-second delay to allow Xephyr to initialize.
3. Launch `fsc` on the `:1` display.

You will see a black Xephyr window appear. This window is now being managed by your `fsc` compositor.

### Testing with dwm

Since `fsc` is now a pure compositor, it can run alongside a window manager like `dwm`. To test this, you can use the `dwm` target:

```bash
make dwm
```

This command will:
1. Start a Xephyr instance.
2. Launch `fsc`.
3. Launch `dwm` within the same Xephyr display.

You should see the `dwm` interface appear in the Xephyr window. You can then launch applications (e.g., `DISPLAY=:1 xterm`) and `dwm` will manage them as usual within that nested environment, with `fsc` applying the scaling.

### Testing Fractional Scaling

To test fractional scaling, modify the `DEFAULT_SCALE` in your `config.h` file. After saving `config.h`, recompile and restart `fsc` (or `make dwm`). Then, launch an X application as described above. The application window should appear inside the Xephyr window, scaled according to the factor you set.

## Credits

This project was developed collaboratively by a human user and a Gemini-powered AI assistant.

## License

This project is licensed under the GNU General Public License v3.0. See the `LICENSE` file for more details.
