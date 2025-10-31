
# ASCII renderer that takes filepath to image as input and returns a .txt

This is a small project to take an image file and render it as ASCII.

## How to run locally

Using CMakeLists.txt you can execute the following commands to run the converter locally:

```bash
mkdir release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Command-line options

| Flag | Description |
|------|--------------|
| `-i`, `--input <arg>` | Path to the input file (**required**) |
| `-o`, `--output <arg>` | Path to the output `.txt` file (default: `out.txt`) |
| `-w`, `--width <arg>` | Target ASCII art character width |
| `-H`, `--height <arg>` | Target ASCII art character height. If both height and width are given, width is prioritized |
| `-s`, `--squishfactor <arg>` | Adjust vertical squish/stretch. Larger value → more squished (default: `1.0`) |
| `-n`, `--invert` | Inverts colors of ASCII art (white↔black) |
| `-h`, `--help` | Show this help page |
| `-c`, `--color` | Render image in terminal color |
