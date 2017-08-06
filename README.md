# png2raw

Converts a simple RGB 8-bit PNG image file to a 8 bit raw data file. During the conversion a pairs
of parameters are chosen to map the input pixels to a values.

Format:
`png2raw <in png> <out raw> ["#RRGGBB" <8 bit num>]...`

Example:
`png2raw my.png my.r8 "#00C080" 0 "#FFCC00" 1 "#808080" 2`
