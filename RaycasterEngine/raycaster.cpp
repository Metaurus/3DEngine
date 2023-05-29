#define _USE_MATH_DEFINES // use mostly for using math constants such as PI
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <math.h>
#include <sstream>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // this is a basic library to import images according to the tutorial

// takes 8-bit RGBA values, packs all the values into one 32 bit integer.
// Alpha - most significant.
// R - least significant 
uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a<<24) + (b<<16) + (g<<8) + r;
}

// takes a 32-bit packed color value, and provides its RGBA values to corresponding memory addresses
void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color>>0) & 255;
    g = (color>>8) & 255;
    b = (color>>16) & 255;
    a = (color>>24) & 255;
}

// outputs a ppm image
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

// draws a rectangle (duh)
void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size() == img_w * img_h); // Ensure the image size equals the given width x height
    for (size_t i = 0; i < w; i++) { // Fill the rectangle according to its width and height
        for (size_t j = 0; j < h; j++) {
            size_t cx = x + i;
            size_t cy = y + j;
            if (cx >= img_w || cy >= img_h) continue; // no need to check for negative values (unsigned variables)
            img[cx + cy * img_w] = color; // Set the corresponding image pixel to the color of the rectangle
        }
    }
}

// loads a texture based on a file of sequential textures
bool load_texture(const std::string filename, std::vector<uint32_t> &texture, size_t &texture_size, size_t &texture_cnt) {
    int nchannels = -1, w, h;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0); // loads the file into a pixelmap

    // if there is an error loading the file, print an error message
    if (!pixmap) {
        std::cerr << "ERROR: cannot load the textures!" << std::endl;
        return false;
    }

    // if the image is not 32 bits, print an error message and free the pixelmap
    if (nchannels != 4) {
        std::cerr << "ERROR: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }

    texture_cnt = w/h; // calculates how many textures are present in the file
    texture_size = w/texture_cnt; // determines the size of the texture by dividing the width of the file by how many textures are present

    // if the file does not have properly packed textures, print an error message and free the pixelmap
    if (w != h*(int)texture_cnt) {
        std::cerr << "ERROR: the texture file must contain N square textures packed horizontally" << std::endl;
        stbi_image_free(pixmap);
        return false;
    } 

    texture = std::vector<uint32_t>(w*h); // initialize our texture

    // for every pixel in the file
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {

            // get the RGBA channels and pack them into the texture
            uint8_t r = pixmap[(i+j*w)*4+0];
            uint8_t g = pixmap[(i+j*w)*4+1];
            uint8_t b = pixmap[(i+j*w)*4+2];
            uint8_t a = pixmap[(i+j*w)*4+3];
            texture[i+j*w] = pack_color(r, g, b, a);
        }
    }
    stbi_image_free(pixmap); // don't forget to free our pixelmap!!!
    return true;
}

// returns the column of a texture
std::vector<uint32_t> texture_column(const std::vector<uint32_t> &img, const size_t texture_size, const size_t ntextures, const size_t texture_id, const size_t texture_coord, const size_t column_height) {
    const size_t img_w = texture_size * ntextures;
    const size_t img_h = texture_size;
    assert(img.size() == img_w*img_h && texture_coord < texture_size && texture_id < ntextures);
    std::vector<uint32_t> column(column_height);
    for (size_t y = 0; y < column_height; y++) {
        size_t pix_x = texture_size * texture_id + texture_coord;
        size_t pix_y = (y*texture_size) / column_height;
        column[y] = img[pix_x + pix_y * img_w];
    }
    return column;
}

int main() {
    const size_t win_w = 1024; // image width
    const size_t win_h = 512; // image height

    std::vector<uint32_t> framebuffer(win_w * win_h, pack_color(255, 255, 255)); // the image itself, initialized to white

    const size_t map_w = 16; //map width
    const size_t map_h = 16; //map height
    const char map[] = "0000222222220000"\
                    "1              0"\
                    "1      11111   0"\
                    "1     0        0"\
                    "0     0  1110000"\
                    "0     3        0"\
                    "0   10000      0"\
                    "0   3   11100  0"\
                    "5   4   0      0"\
                    "5   4   1  00000"\
                    "0       1      0"\
                    "2       1      0"\
                    "0       0      0"\
                    "0 0000000      0"\
                    "0              0"\
                    "0002222222200000"; // our game map
    assert(sizeof(map) == map_w * map_h + 1); // +1 for terminated null string

    float player_x = 4.0;
    float player_y = 3.0;
    float player_a = 2.0; // the angle from the x-axis (in radians) to mark which way the player is facing
    const float fov = M_PI / 3.0; // the fov in terms of radians (in this case the player can see pi/3 radians)

    std::vector<uint32_t> walltexture; // textures for walls
    size_t walltexture_size; // texture dimensions (it is a square)
    size_t walltexture_cnt; // number of different textures in the image
    if (!load_texture("../Assets/textures/walltext.png", walltexture, walltexture_size, walltexture_cnt)) {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

//No longer want a color gradient background
/*
    for (size_t j = 0; j < win_h; j++) { // fill the screen with color gradients
        for (size_t i = 0; i < win_w; i++) {
            uint8_t r = 255 * j / (float) win_h; // varies between 0 and 255 as j sweeps the vertical
            uint8_t g = 255 * i / (float) win_w; // varies between 0 and 255 as i sweeps the horizontal
            uint8_t b = 255 * j / (float) win_h;
            framebuffer[i+j*win_w] = pack_color(r, g, b); // convert the 2D image into a 1D array, store the packed color values
        }
    }
*/
    const size_t rect_w = win_w / (map_w*2); // rectangle width and height is scaled according to map size
    const size_t rect_h = win_h / map_h; 

/*
    for (size_t frame = 0; frame < 360; frame++) { // move camera in a circle for 360 frames
        std::stringstream ss; // string stream to set the .ppm file names
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player_a += 2*M_PI/360; // add one degree of movement (scaled to radians) for every iteration
        framebuffer = std::vector<uint32_t>(win_w*win_h, pack_color(255, 255, 255)); // clear the screen
        for (size_t j = 0; j < map_h; j++) { // draw the map
            for (size_t i = 0; i < map_w; i++) {
                if (map[i + j * map_w] == ' ') continue; // skip empty spaces
                size_t rect_x = i * rect_w; // determine rectangle x and y coordinates
                size_t rect_y = j * rect_h;
                size_t icolor = map[i+j*map_w] - '0'; // change the 2d map into a 1d map, remove all the 0s, and use the remaining numbers as a color value
*/      
    for (size_t j = 0; j < map_h; j++) { // draw the map
        for (size_t i = 0; i < map_w; i++) {
            if (map[i+j*map_w] == ' ') continue; // skip the empty spaces
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            size_t texture_id = map[i+j*map_w] - '0';
            assert(texture_id < walltexture_cnt);
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, walltexture[texture_id * walltexture_size]); // color is taken from upper left pixel of texture
        }
    }

    // draw the player on the map
    draw_rectangle(framebuffer, win_w, win_h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255));

    // logic to draw rays
    // t is a constant to slide up and down the ray that is cast from the player's facing direction
    // the incrementing value of t controls how many points we mark along the ray (smaller = more)
    for (size_t i = 0; i < win_w/2; i++) { // draw the visibility cone and 3D view
        float angle = player_a - fov/2 + fov*i/(float)(win_w/2); // calculate the ray angle given the fov and number of rays being cast
        for (float t = 0; t < 20; t += 0.01) {
            float cx = player_x + t * cos(angle); // x-coord plus scaled x-angle (cos) adjustment
            float cy = player_y + t * sin(angle); // y-coord plus scaled y-angle (sin) adjustment
            
            int pix_x = cx * rect_w; // scale coords to draw the ray point
            int pix_y = cy * rect_h;
            framebuffer[pix_x + pix_y*win_w] = pack_color(160, 160, 160); // draw a dot to make the ray

            if (map[(int)cx + (int)cy * map_w] != ' ') { // if we reach a wall, draw the vertical column to create a 3D illusion (magic!)
                size_t texture_id = map[(int)cx + (int)cy * map_w] - '0';
                assert(texture_id < walltexture_cnt);
                // the column height will be smaller if t is bigger (i.e. further away)
                // to fix the fisheye effect when looking at a wall, t is multiplied by the cosine of the difference between
                // the ray angle and the angle of where the player is facing
                size_t column_height = win_h/(t*cos(angle-player_a)); 
                
                float hitx = cx - floor(cx + 0.5); // hitx and hity contain (signed) fractional parts of cx and cy,
                float hity = cy - floor(cy + 0.5); // they vary between -0.5 and 0.5, one of them is supposed to be very close to 0
                int x_texture_coord = hitx * walltexture_size;
                if (std::abs(hity) > std::abs(hitx)) { // need to determine if we hit a vertical or horizontal wall (w.r.t. the map)
                    x_texture_coord = hity * walltexture_size;
                }
                if (x_texture_coord < 0) x_texture_coord += walltexture_size; // do not forget x_texture_coord can be negative, fix that
                assert(x_texture_coord >= 0 && x_texture_coord < (int) walltexture_size);

                std::vector<uint32_t> column = texture_column(walltexture, walltexture_size, walltexture_cnt, texture_id, x_texture_coord, column_height);
                pix_x = win_w/2+i; // find the correct x pixel
                for (size_t j = 0; j < column_height; j++) { // draw every pixel of the column
                    pix_y = j + win_h/2 - column_height/2;
                    if (pix_y < 0 || pix_y > (int) win_h) continue; // only draw what is on the screen
                    framebuffer[pix_x+pix_y*win_w] = column[j];
                }

                break;
            } 
        }
    }
    
    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}