
# ASCII renderer that takes filepath to image as input and returns a .txt

This is a small project to take an image file and render it as ASCII.

## How to run locally

Using CMakeLists.txt you can execute the following commands to run the converter locally:

```
mkdir release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

Use `./asciirenderer -h` to see all commands

This project requires [Magick++](https://imagemagick.org/script/magick++.php) to run.
