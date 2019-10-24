# png2raw

Converts a simple RGB 8-bit PNG image file to a 8 bit raw data file. During the conversion a pairs
of parameters are chosen to map the input pixels to a values.

Format:
`png2raw <in png> <out raw> ["#RRGGBB" <8 bit num>]...`

Example:
`png2raw my.png my.r8 "#00C080" 0 "#FFCC00" 1 "#808080" 2`

# png-remap

Maps a PNG file with pixel values 0 to N, to RGBA colors listed on 0 to N rows in text file. Outputs
an RGBA PNG file as result. Basically, it is kind of pixel precise color remapper. This is useful
for rasters (mostly grayscale) where each pixel denotes an object id and they need to be
visualized. 

Existing tools, like ImageMagick, easily intervene on each stage at such application. For example,
the LUT table is interpreted as different number of colors and though it can be avoided, special
care must be taken the LUT not to be interpolated. Another issue is that the conversions between
perceptual and linear colors can easily add more troubles even in innocent operations. So, while
great for applications like art, where precisions is not required, it is not okay for niche ops
requiring pixel perfect operations.

Build with Python:
```sh
./waf configure
./waf
```
or use directly C++ compiler:
```sh
c++ remap-png.cpp lodepng.cpp -o remap-png
```

Usage:
```sh
./remap-png lut.txt input.png output.png
```

Where lut.txt is a simple text file, with hex color per line:
```
# ImageMagick pixel enumeration: 848,1,255,srgba
0,0:   (0  , 0  , 0  ,   0)   #00000000
1,0:   (226, 226, 224, 255)   #E2E2E0FF
2,0:   (48 , 74 , 0  , 255)   #304A00FF
3,0:   (112, 168, 0  , 255)   #70A800FF
4,0:   (93 , 150, 0  , 255)   #5D9600FF
5,0:   (76 , 115, 0  , 255)   #4C7300FF
6,0:   (108, 148, 0  , 255)   #6C9400FF
7,0:   (108, 148, 0  , 255)   #6C9400FF
8,0:   (152, 229, 0  , 255)   #98E500FF
9,0:   (19 , 237, 0  , 255)   #13ED00FF
10,0:  (38 , 116, 0  , 255)   #267400FF
11,0:  (38 , 116, 0  , 255)   #267400FF
```
This file is the ImageMagick TXT file format. Each color, after the `#` sign, is the pixel value
which should be searched in the input png file. For example, if in the input greyscale PNG (8 or 16
bit) there is a value of zero, the the zero color will be used. If that pixel had a value of 1, 2
or 3, then the 1, 2 and the 3rd color would be used. So, the order of colors in the LUT file maps
directly to the values in the input raster.

The input raster can be also palletted or rgb/rgba. In the later case though the pixel values might
be too big. For example, white #FFFFFFFF, would mean that the LUT file needs to have 2^32 colors!
