#include "tgaimage.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    float derror = std::abs(dy)*2;
    float error = 0;
    int y = y0;

    /* for (int x=x0; x<=x1; x++) {
        if (steep) {
            image.set(y, x, TGAColor(255, 1));
        } else {
            image.set(x, y, TGAColor(255, 1));
        }
        error += derror;

        if (error > dx) {
            y += (y1>y0?1:-1);
            error -= dx*2;
        }
    } */

    // Optimization of the commented code above...
    // apparently removing the conditional branching in the 'for' loops optimizes
    // by a factor of 2. Super cool!
    if(steep) {
        for(int x = x0; x<=x1; ++x) {
            image.set(y, x, color);
            error += derror;
            if(error > dx) {
                // this is technically also branching, 
                // it can be computed at the beginning of the function instead
                y += (y1>y0? 1 : -1); 
                error -= dx*2;
            }
        }
    } else {
        for(int x = x0; x<=x1; ++x) {
            image.set(x, y, color);
            error += derror;
            if(error > dx) {
                y += (y1>y0? 1 : -1);
                error -= dx*2;
            }
        }
    }
}

int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    line(13, 20, 80, 40, image, white);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}