#include <iostream>
#include <cstdio>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"
#include <vector>
#include <random>
#include <algorithm>

struct RGBPixel{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Image {
    int width = 0;
    int height = 0;
    std::vector<RGBPixel> pixels;
};

void print_usage(){
    printf("Usage: [.bmp image file location] \n");
}

Image openImage(std::string& fileName){
    int width, height, channels;
    uint8_t* imageData = stbi_load(fileName.c_str(), &width, &height, &channels, 3); // force RGB output
    if(imageData == nullptr){
        printf("Image loading failed.\n");
        return {};
    }
    Image image = {
        width,
        height,
        {},
    };
    image.pixels.reserve(width*height);
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            // *width scanlines of *height pixels
            // first pixel - top left
            int index = (width * y + x) * 3;
            RGBPixel pixel = {
                imageData[index],
                imageData[index + 1],
                imageData[index + 2],
            };
            image.pixels.push_back(pixel);
        }
    }
    stbi_image_free(imageData);
    return image;
}

void writeImage(std::string fileName, Image &img){
    if(!stbi_write_bmp(fileName.c_str(), img.width, img.height, 3, img.pixels.data())){
        printf("Image failed to save.\n");
        return;
    }
}

std::vector<float> computeDarknessMap(Image &img){
    std::vector<float> darkness;
    darkness.reserve(img.height * img.width);
    for(int y = 0; y < img.height; y++){
        for(int x = 0; x < img.width; x++){
            int index = (img.width * y + x);
            float luminance = 0.299 * img.pixels[index].r + 0.587 * img.pixels[index].g + 0.114 * img.pixels[index].b;
            darkness.push_back((1-luminance/255));
        }
    }
    return darkness;
}

struct Position{
    int x;
    int y;
};

std::vector<Position> seedPoints(std::vector<float> &density, Image &img){
    int N = 10000; // how many points to seed
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distX(0, img.width  - 1);
    std::uniform_int_distribution<int> distY(0, img.height - 1);
    std::uniform_real_distribution<float> tDist(0.0f, 1.0f);
    std::vector<Position> seededPoints;
    seededPoints.reserve(N);

    while(N != 0){
        int x = distX(gen);
        int y = distY(gen);
        int index = (img.width * y + x);
        float darkness = density.data()[index];
        if (darkness > tDist(gen)){
            Position pos = {
                x, 
                y
            };
            seededPoints.push_back(pos);
            N--;
        }
    }
    return seededPoints;
}

std::vector<jcv_point> packPoints(std::vector<Position> &positions){
    std::vector<jcv_point> points;
    points.reserve(positions.size());
    for(Position &p : positions){
        points.push_back({static_cast<jcv_real>(p.x), static_cast<jcv_real>(p.y)});
    }
    return points;
}

int main(int argc, char *argv[])
{   
    if(argc < 2){
        print_usage();
        return 1;
    }
    std::string fileName = argv[1];
    if(!fileName.ends_with(".bmp")){
        printf("Given file is not a .BMP file. Refer to usage.");
        print_usage();
        return 1;
    }
    
    printf("Entered %d variables, the one we care about is %s", argc, argv[1]);

    // open file, close file and save as different one
    Image img = openImage(fileName);
    writeImage("src/written_image.bmp", img);

    return 0;
}