/*
remap-png version 20191024

Copyright (c) 2019 ryobg@users.noreply.github.com

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "lodepng.h"
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

//--------------------------------------------------------------------------------------------------

struct lodepng_raster
{
    std::vector<std::uint32_t> data;
    unsigned width, height;
    unsigned depth;
    LodePNGColorType type;
};

//--------------------------------------------------------------------------------------------------

static std::vector<std::uint32_t>
read_map_file (const char* path)
{
    using namespace std;

    vector<uint32_t> out;

    size_t total = 0;
    ifstream fi (path);
    for (string line; getline (fi, line); ++total)
    {
        auto n = line.find ('#');
        if (string::npos == n)
            continue;
        uint32_t v;
        try {
            v = static_cast<uint32_t> (stoll (line.substr (n+1), nullptr, 16));
            std::reverse ((uint8_t*) &v, ((uint8_t*) &v) + 4);
        }
        catch (std::exception const&) {
            continue;
        }
        out.push_back (v);
    }

    cout << "Found " << out.size () << " hex colors out of " << total << " text lines." << endl;
    if (out.empty ())
        throw std::runtime_error ("No colors to remap with. Bailing out.");
    return out;
}

//--------------------------------------------------------------------------------------------------

[[noreturn]] static void
throw_lodpng_error (unsigned error)
{
    throw std::runtime_error (
            std::string ("LodePNG decoder error #") + std::to_string (error) +
            std::string (": ") + lodepng_error_text (error));
}

//--------------------------------------------------------------------------------------------------

static lodepng_raster
read_input_raster (const char* path)
{
    using namespace std;

    lodepng_raster out;

    unsigned char* image = nullptr;
    unsigned char* blob = nullptr;
    size_t blob_size = 0;
    LodePNGState state;
    lodepng_state_init (&state);

    auto free_locals = [&] ()
    {
        free (image);
        free (blob);
        lodepng_state_cleanup (&state);
    };
    auto ensure = [&] (unsigned error)
    {
        if (error)
        {
            free_locals ();
            throw_lodpng_error (error);
        }
    };

    ensure (lodepng_load_file (&blob, &blob_size, path));
    ensure (lodepng_inspect (&out.width, &out.height, &state, blob, blob_size));

    unsigned original_depth = state.info_png.color.bitdepth;
    out.depth = max (original_depth, 8u);
    out.type = state.info_png.color.colortype;
    state.info_raw.colortype = out.type;
    state.info_raw.bitdepth = out.depth;

    int num_channels = 1;
    if (out.type == LCT_GREY_ALPHA) num_channels = 2;
    if (out.type == LCT_RGB) num_channels = 3;
    if (out.type == LCT_RGBA) num_channels = 4;

    int pixel_size = num_channels * out.depth / 8;
    if (pixel_size > 4)
    {
        free_locals ();
        throw runtime_error ("Pixels bigger than 4 bytes are not supported!");
    }

    ensure (lodepng_decode (&image, &out.width, &out.height, &state, blob, blob_size));

    out.data.resize (out.height * out.width);
    for (size_t i = 0, n = out.data.size (); i < n; ++i)
    {
        uint8_t const* src = &image[i * pixel_size];
        uint32_t v = 0;
        for (int c = 0; c < num_channels; ++c)
            std::reverse_copy (&src[c], &src[c] + out.depth / 8, &((uint8_t*) &v)[c]);
        out.data[i] = v;
    }

    free_locals ();

    cout << "Decoded " << out.width << "x" << out.height << " " << original_depth << "-bit ";
    switch (out.type) {
        case LCT_GREY: cout << "1 channel (grayscale)"; break;
        case LCT_RGB: cout << "3 channels (rgb)"; break;
        case LCT_PALETTE : cout << " 1 channel (indexed)"; break;
        case LCT_GREY_ALPHA: cout << "2 channels (grey-alpha)"; break;
        case LCT_RGBA: cout << "4 channels (rgba)"; break;
        default: cout << " unknown(!)"; break;
    };
    cout << " input PNG file." << endl;

    return out;
}

//--------------------------------------------------------------------------------------------------

static void
remap_values (
        std::vector<std::uint32_t> const& lut,
        std::uint32_t default_value,
        lodepng_raster& raster)
{
    using namespace std;
    size_t c = 0;
    for (auto& p: raster.data)
    {
        if (p < lut.size ())
            p = lut[p];
        else p = default_value, c++;
    }
    cout << "Default #" <<hex<< default_value <<dec<< " assigned to " << c << " pixels." << endl;
}

//--------------------------------------------------------------------------------------------------

static void
write_output (lodepng_raster& out, const char* path)
{
    auto e = lodepng_encode32_file (path, (uint8_t const*) out.data.data (), out.width, out.height);
    if (e) throw_lodpng_error (e);
}

//--------------------------------------------------------------------------------------------------

int
main (int argc, char const* argv[])
{
    using namespace std;

    if (argc != 4)
    {
        cerr << "Usage: png-remap <lut> <in> <out>" << endl;
        return 1;
    }

    try
    {
        auto map = read_map_file (argv[1]);
        auto buf = read_input_raster (argv[2]);
        remap_values (map, 0, buf);
        write_output (buf, argv[3]);
    }
    catch (std::exception const& ex)
    {
        cerr << ex.what () << endl;
        return 1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------

