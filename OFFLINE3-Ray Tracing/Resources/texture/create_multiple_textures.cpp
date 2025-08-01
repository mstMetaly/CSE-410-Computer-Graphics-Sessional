#include "bitmap_image.hpp"
#include <iostream>
#include <cmath>

void createBrickTexture(const std::string& filename) {
    int width = 256, height = 256;
    bitmap_image texture(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int brick_width = 32, brick_height = 16, mortar_size = 2;
            int brick_x = x / brick_width;
            int brick_y = y / brick_height;
            
            if (brick_y % 2 == 1) {
                brick_x = (x + brick_width / 2) / brick_width;
            }
            
            int local_x = x % brick_width;
            int local_y = y % brick_height;
            
            if (local_x < mortar_size || local_y < mortar_size) {
                texture.set_pixel(x, y, 100, 100, 100); // Gray mortar
            } else {
                int r = 180 + (brick_x + brick_y * 7) % 50;
                int g = 80 + (brick_x * 3 + brick_y) % 30;
                int b = 60 + (brick_x + brick_y * 2) % 20;
                texture.set_pixel(x, y, r, g, b);
            }
        }
    }
    texture.save_image(filename);
}

void createWoodTexture(const std::string& filename) {
    int width = 256, height = 256;
    bitmap_image texture(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Wood grain pattern
            double noise = sin(y * 0.1) * 0.3 + sin(x * 0.05) * 0.2;
            int base_brown = 139 + (int)(noise * 30);
            int r = std::min(255, std::max(0, base_brown));
            int g = std::min(255, std::max(0, (int)(base_brown * 0.7)));
            int b = std::min(255, std::max(0, (int)(base_brown * 0.4)));
            
            texture.set_pixel(x, y, r, g, b);
        }
    }
    texture.save_image(filename);
}

void createMarbleTexture(const std::string& filename) {
    int width = 256, height = 256;
    bitmap_image texture(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Marble veining pattern
            double pattern = sin((x + y) * 0.02) * 50 + 
                           sin(x * 0.03) * 30 + 
                           sin(y * 0.04) * 20;
            
            int base_white = 200 + (int)(pattern * 0.2);
            int r = std::min(255, std::max(180, base_white));
            int g = std::min(255, std::max(180, base_white - 10));
            int b = std::min(255, std::max(180, base_white - 20));
            
            texture.set_pixel(x, y, r, g, b);
        }
    }
    texture.save_image(filename);
}

void createTileTexture(const std::string& filename) {
    int width = 256, height = 256;
    bitmap_image texture(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int tile_size = 32;
            int grout_size = 2;
            
            int tile_x = x % tile_size;
            int tile_y = y % tile_size;
            
            if (tile_x < grout_size || tile_y < grout_size || 
                tile_x >= tile_size - grout_size || tile_y >= tile_size - grout_size) {
                // Grout
                texture.set_pixel(x, y, 180, 180, 180);
            } else {
                // Tile with slight color variation
                int base_color = 220 + ((x/tile_size + y/tile_size) % 3) * 10;
                texture.set_pixel(x, y, base_color, base_color - 20, base_color - 40);
            }
        }
    }
    texture.save_image(filename);
}

int main() {
    std::cout << "Creating multiple texture options..." << std::endl;
    
    createBrickTexture("texture_brick.bmp");
    std::cout << "✓ Brick texture saved as texture_brick.bmp" << std::endl;
    
    createWoodTexture("texture_wood.bmp");
    std::cout << "✓ Wood texture saved as texture_wood.bmp" << std::endl;
    
    createMarbleTexture("texture_marble.bmp");
    std::cout << "✓ Marble texture saved as texture_marble.bmp" << std::endl;
    
    createTileTexture("texture_tile.bmp");
    std::cout << "✓ Tile texture saved as texture_tile.bmp" << std::endl;
    
    std::cout << "\nTo use any texture, rename it to 'texture.bmp' or modify main.cpp" << std::endl;
    std::cout << "Current texture patterns available:" << std::endl;
    std::cout << "- texture_brick.bmp  (Red brick pattern)" << std::endl;
    std::cout << "- texture_wood.bmp   (Wood grain)" << std::endl;
    std::cout << "- texture_marble.bmp (Marble veining)" << std::endl;
    std::cout << "- texture_tile.bmp   (Ceramic tiles)" << std::endl;
    
    return 0;
} 