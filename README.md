# image2tiles
Cuts an image into OGC conformal XYZ-tiles.

# Usage
To split an image into tiles, it is necessary to determine on what location the image actually is.
This is done using the longitude/latitude degrees of two points in the image.
They are then used to determine what region should be cut out of the image and stored to disk.

## Finding coordinates
The ideal points in the image are diagonally opposed points (e.g. top-left and bottom-right corners).
To find the coordinates of two points in the image, I recommend [openstreetmap.org](https://openstreetmap.org) to find the geo-location in longitude/latitude degrees.

Also remember the position in the image (the location in pixels), it's needed below.

## Running the application
After finding the points, they can be passed to the application via the `--p1, -1` and `--p2, -2` parameters:
```bash
./image2tiles --p1=0,-23.45,0,64.53 --p2=10000,-24.35,5000,63.71 --file=scan.jpg --max-zoom-level=13
```
The format of the parameter is:
```
<pixel-x>,<longitude>,<pixel-y>,<latitude>
```

## General parameters and flags
Parameters expect an argument, flags do not.

| Parameter | Description |
| - | - |
| `-1, --p1` | First point using a point string (s. above) |
| `-2, --p2` | Second point using a point string (s. above) |
| `-z, --max-zoom-level` | Maximal zoom level (0..19). Tiles will have a zoom level less or equal to this. |
| `-t, --tile-size` | Size of a tile in pixel (default: 256) |
| `-o, --output-folder` | Output folder (defult: `.out/`) |
| `-f, --file` | The image file that should be cutted |

| Flag | Description |
| - | - |
| `-v, --verbose` | More detailed output |
| `-d, --debug` | Even more output including debug logging |
| `--version` | Version of this application |
| `-h, --help` | Prints this message |

# Build
To build the source, make sure OpenCV is installed and then execute `make`.
Also C++17 is used, so make sure you have GCC installed which supports this standard.

## Arch Linux
This includes the setup of OpenCV.
```bash
sudo pacman --needed -S opencv
ln -s /usr/include/opencv4/opencv2/ /usr/include/opencv2
git clone https://github.com/hauke96/image2tiles.git
make
./image2tiles ...
```

## Other Linux distributions
Installing OpenCV on Ubuntu is pretty difficult, therefore it is not covered here.
Make sure there's a `/usr/include/opencv2` folder containing the needed OpenCV files.
```bash
# Installation of OpenCV first

git clone https://github.com/hauke96/image2tiles.git
make
./image2tiles ...
```

## Documentation
There is a `Doxyfile` which can be used to generate a HTML documentation with the `doxygen` command.

This generates the documentation and starts an http-server:
```bash
cd ./image2tiles
doxygen
python3 -m http.server
```
After that open the URL `http://localhost:8000/doc/html/` (the port may be different).
