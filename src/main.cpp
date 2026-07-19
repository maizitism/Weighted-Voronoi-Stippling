#include <iostream>
#include <cstdio>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>

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