#include <cmath>
#include <vector>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }
    int dx = p1.x-p0.x;
    int dy = p1.y-p0.y;
    float derror = std::abs(dy)*2;
    float error = 0;
    int y = p0.y;

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
        for(int x = p0.x; x<=p1.x; ++x) {
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
        for(int x = p0.x; x<=p1.x; ++x) {
            image.set(x, y, color);
            error += derror;
            if(error > dx) {
                y += (y1>y0? 1 : -1);
                error -= dx*2;
            }
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
    if (t0.y>t1.y) std::swap(t0, t1); 
    if (t0.y>t2.y) std::swap(t0, t2); 
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y - t0.y;

    for (int i=0; i<total_height; i++) { 
        bool second_half = i>t1.y-t0.y || t1.y==t0.y; 
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y; 
        float alpha = (float)i/total_height; 
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here 
        Vec2i A =               t0 + (t2-t0)*alpha; 
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
        if (A.x>B.x) std::swap(A, B); 
        for (int j=A.x; j<=B.x; j++) { 
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y 
        } 
    } 
}

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
}