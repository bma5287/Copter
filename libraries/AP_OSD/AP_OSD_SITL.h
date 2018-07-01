/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#ifdef WITH_SITL_OSD

#include <AP_OSD/AP_OSD_Backend.h>
#include <SFML/Graphics.hpp>

class AP_OSD_SITL : public AP_OSD_Backend {

public:
    static AP_OSD_Backend *probe(AP_OSD &osd);

    //draw given text to framebuffer
    void write(uint8_t x, uint8_t y, const char* text, uint8_t char_attr) override;

    //initilize display port and underlying hardware
    bool init() override;

    //flush framebuffer to screen
    void flush() override;

    //clear framebuffer
    void clear() override;

private:
    //constructor
    AP_OSD_SITL(AP_OSD &osd);

    sf::RenderWindow *w;

    sf::Texture font[256];

    // setup to match MAX7456 layout
    static const uint8_t char_width = 12;
    static const uint8_t char_height = 18;
    static const uint8_t video_lines = 16; // PAL
    static const uint8_t video_cols = 30;
    static const uint8_t char_spacing = 1;

    // scaling factor to make it easier to read
    static const uint8_t char_scale = 2;
    
    uint8_t buffer[video_lines][video_cols];

    void load_font();
};

#endif // WITH_SITL_OSD
