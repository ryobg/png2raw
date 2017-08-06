/*
png2raw version 20170806

Copyright (c) 2017 ryobg@users.noreply.github.com

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
#include <map>
#include <string>
#include <fstream>

//--------------------------------------------------------------------------------------------------

int main (int argc, char const* argv[])
{
    using namespace std;

    if (argc < 3 || (argc - 3) & 1)
    {
        cerr << "Pass <png> <raw> [\"#RRGGBB\" <8-bit number>]... " << argc << endl;
        return 1;
    }

    map<uint32_t, uint8_t> cnv;
    for (int i = 3; i < argc; i += 2)
    {
        string px (argv[i]);
        while (px.size ())
        {
            try {
                auto v = stoul (px, 0, 16);
                auto n = stoul (argv[i+1], 0, 0);
                cnv[v] = uint8_t (n);
                break;
            }
            catch (std::exception const&)
            {
                px = px.substr (1);
            }
        }
        if (px.empty ())
        {
            cerr << "Invalid param " << argv[i] << endl;
            return 2;
        }
    }

    vector<uint8_t> in;
    unsigned w, h;
    auto err = lodepng::decode (in, w, h, string (argv[1]), LCT_RGB, 8);
    if (err)
    {
        cerr << "LodePNG error: " << lodepng_error_text (err) << endl;
        return err;
    }

    vector<uint8_t> out;
    for (size_t i = 0, n = in.size (); i < n; i += 3)
    {
        uint32_t p = 0;
        p |= uint32_t (in[i]) << 16;
        p |= uint32_t (in[i+1]) << 8;
        p |= uint32_t (in[i+2]);
        out.push_back (cnv[p]);
    }

    ofstream raw (argv[2], ios_base::binary);
    for (auto v: out)
        raw.write ((char*) &v, sizeof v);

    return 0;
}

//--------------------------------------------------------------------------------------------------

