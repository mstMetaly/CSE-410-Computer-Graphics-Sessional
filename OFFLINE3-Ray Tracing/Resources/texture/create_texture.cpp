#include "bitmap_image.hpp"
#include <iostream>
#include <cmath>

int main() {
    int width = 256;
    int height = 256;
    
    bitmap_image texture(width, height);
    
    // Create a simple pattern texture
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create a brick-like pattern
            int brick_width = 32;
            int brick_height = 16;
            int mortar_size = 2;
            
            int brick_x = x / brick_width;
            int brick_y = y / brick_height;
            
            // Offset every other row
            if (brick_y % 2 == 1) {
                brick_x = (x + brick_width / 2) / brick_width;
            }
            
            int local_x = x % brick_width;
            int local_y = y % brick_height;
            
            // Check if we're in mortar area
            if (local_x < mortar_size || local_y < mortar_size) {
                // Mortar (gray)
                texture.set_pixel(x, y, 100, 100, 100);
            } else {
                // Brick (reddish brown with some variation)
                int r = 180 + (brick_x + brick_y * 7) % 50;
                int g = 80 + (brick_x * 3 + brick_y) % 30;
                int b = 60 + (brick_x + brick_y * 2) % 20;
                texture.set_pixel(x, y, r, g, b);
            }
        }
    }
    
    texture.save_image("texture.bmp");
    std::cout << "Test texture saved as texture.bmp" << std::endl;
    
    return 0;
} 