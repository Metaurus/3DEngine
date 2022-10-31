#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

// Takes 8-bit RGBA values, packs all the values into one 32 bit integer.
// Alpha - most significant.
// R - least significant 
uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a<<24) + (b<<16) + (g<<8) + r;
}


// Takes a 32-bit packed color value, and provides its RGBA values to corresponding memory addresses
void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color>>0) & 255;
    g = (color>>8) & 255;
    b = (color>>16) & 255;
    a = (color>>24) & 255;
}

void drop_ppm_image(const std::string filename, const std::vector<uint32_t> &image, const size_t w, const size_t h) {
    assert(image.size() == w*h); // Ensure the image size equals the given width x height

    // Initialize an Output File Stream by providing a file name
    // and an ios binary?
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << w << " " << h << "\n255\n";
    for (size_t i = 0; i < w*h; ++i) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a); // Get RGBA value of every pixel in the image
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b); // Write every RGB value to the Output File Stream
    }
    ofs.close();
}

void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size() == img_w * img_h); // Ensure the image size equals the given width x height
    for (size_t i = 0; i < w; i++) { // Fill the rectangle according to its width and height
        for (size_t j = 0; j < h; j++) {
            size_t cx = x + i;
            size_t cy = y + j;
            assert(cx < img_w && cy < img_h); // Ensure we are within bounds
            img[cx + cy * img_w] = color; // Set the corresponding image pixel to the color of the rectangle
        }
    }
}

int main() {
    const size_t win_w = 512; // image width
    const size_t win_h = 512; // image height

    std::vector<uint32_t> framebuffer(win_w * win_h, 255); // the image itself, initialized to red

    const size_t map_w = 16; //map width
    const size_t map_h = 16; //map height
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000"; // our game map
    assert(sizeof(map) == map_w * map_h + 1); // +1 for terminated null string

    float player_x = 4.0;
    float player_y = 3.0;

    for (size_t j = 0; j < win_h; j++) {
        for (size_t i = 0; i < win_w; i++) {
            uint8_t r = 255 * j / (float) win_h; // varies between 0 and 255 as j sweeps the vertical
            uint8_t g = 255 * i / (float) win_w; // varies between 0 and 255 as i sweeps the horizontal
            uint8_t b = 255 * j / (float) win_h;
            framebuffer[i+j*win_w] = pack_color(r, g, b); // convert the 2D image into a 1D array, store the packed color values
        }
    }

    const size_t rect_w = win_w / map_w; // rectangle width and height is scaled according to map size
    const size_t rect_h = win_h / map_h; 
    for (size_t j = 0; j < map_h; j++) { // draw the map
        for (size_t i = 0; i < map_w; i++) {
            if (map[i + j * map_w] == ' ') continue; // skip empty spaces
            size_t rect_x = i * rect_w; // determine rectangle x and y coordinates
            size_t rect_y = j * rect_h;
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 255, 255)); // draw the rectangle as a cyan color
        }
    }

    // draw the player on the map
    draw_rectangle(framebuffer, win_w, win_h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255));

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}