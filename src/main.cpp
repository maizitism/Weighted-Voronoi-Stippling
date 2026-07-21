#include <iostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define JC_VORONOI_IMPLEMENTATION
#include "jcv.h"
#include "CLI11.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

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
    printf("Image loaded.\n");
    return image;
}

void writeImage(std::string &fileName, Image &img){
    if(!stbi_write_bmp(fileName.c_str(), img.width, img.height, 3, img.pixels.data())){
        printf("Image failed to save.\n");
        return;
    }
    printf("Image written.\n");
}

std::vector<float> computeDarknessMap(Image &img){
    std::vector<float> darkness;
    darkness.reserve(img.height * img.width);
    printf("Computing darkness map...\n");
    for(int y = 0; y < img.height; y++){
        for(int x = 0; x < img.width; x++){
            int index = (img.width * y + x);
            float luminance = 0.299 * img.pixels[index].r + 0.587 * img.pixels[index].g + 0.114 * img.pixels[index].b; // shoutout REC.601
            darkness.push_back((1-luminance/255));
        }
    }
    return darkness;
}

struct Positions{
    int x;
    int y;
};

std::vector<Positions> seedPoints(std::vector<float> &density, Image &img){
    int N = 10000; // how many points to seed
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distX(0, img.width  - 1);
    std::uniform_int_distribution<int> distY(0, img.height - 1);
    std::uniform_real_distribution<float> tDist(0.0f, 1.0f);
    std::vector<Positions> seededPoints;
    seededPoints.reserve(N);
    printf("Seeding %d points... \n", N);
    while(N != 0){
        int x = distX(gen);
        int y = distY(gen);
        int index = (img.width * y + x);
        float darkness = density[index];
        if (darkness > tDist(gen)){
            Positions pos = {
                x, 
                y
            };
            seededPoints.push_back(pos);
            N--;
        }
    }
    return seededPoints;
}

std::vector<jcv_point> packPoints(std::vector<Positions> &positions){
    std::vector<jcv_point> points;
    points.reserve(positions.size());
    printf("Packing points...\n");
    for(Positions &p : positions){
        points.push_back({static_cast<jcv_real>(p.x), static_cast<jcv_real>(p.y)});
    }
    return points;
}

double edge(jcv_point a, jcv_point b, jcv_point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void relaxPoints(std::vector<jcv_point>& points, std::vector<float>& darkness, int width, int height, int iterations){
    jcv_rect rect;
    rect.min = {0.0f, 0.0f};
    rect.max = {static_cast<jcv_real>(width - 1), static_cast<jcv_real>(height - 1)};
    for(int iter = 0; iter < iterations; iter++){
        printf("Performing Lloyd relaxation iteration %d...\n", iter);
        jcv_diagram diagram{};
        jcv_diagram_generate(static_cast<int>(points.size()), points.data(), &rect, nullptr, &diagram);

        size_t N = points.size();
        std::vector<double> sumW (N, 0.0);
        std::vector<double> sumWX (N, 0.0);   
        std::vector<double> sumWY (N, 0.0);
        
        const jcv_site *sites = jcv_diagram_get_sites(&diagram);
        for(int i = 0; i < diagram.numsites; i++){ // each cell
            const jcv_site *site = &sites[i];
            jcv_point C = site->p; // get center point
            int idx = site->index; // get location which maps back into packedPoints array

            const jcv_graphedge *e = site->edges;
            while (e != nullptr){ // each edge
                jcv_point A = e->pos[0];
                jcv_point B = e->pos[1];

                // discombobulate. (rasterise)

                // Adapted from - https://stackoverflow.com/a/9070812, Posted by templatetypedef
                // make bounding box from ABC
                double minX = std::min({C.x, A.x, B.x});
                double maxX = std::max({C.x, A.x, B.x});
                double minY = std::min({C.y, A.y, B.y});
                double maxY = std::max({C.y, A.y, B.y});
                // this gives float values - normalise to whole image so we can iterate over pixels
                int x0 = std::max(0, static_cast<int>(std::floor(minX)));
                int x1 = std::min(width - 1, static_cast<int>(std::floor(maxX)));
                int y0 = std::max(0, static_cast<int>(std::floor(minY)));
                int y1 = std::min(height - 1, static_cast<int>(std::floor(maxY)));
                
                // iterate over computed BB
                for(int y = y0; y <= y1; y++){
                    for(int x = x0; x <= x1; x++){
                        // check if pixel is inside triangle
                        // jvc_point is really just a {float, float}
                        double d0 = edge(A, B, {static_cast<jcv_real>(x), static_cast<jcv_real>(y)});
                        double d1 = edge(B, C, {static_cast<jcv_real>(x), static_cast<jcv_real>(y)});
                        double d2 = edge(C, A, {static_cast<jcv_real>(x), static_cast<jcv_real>(y)});
                        bool inside = (d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0);
                        if (inside){
                            float w = darkness[y * width + x];
                            sumW[idx] += w;
                            sumWX[idx] += w * x;
                            sumWY[idx] += w * y;
                        }
                    }
                }
                e = e->next;
            }
            if(sumW[idx] > 0.0){
                points[idx].x = static_cast<jcv_real>(sumWX[idx] / sumW[idx]);
                points[idx].y = static_cast<jcv_real>(sumWY[idx] / sumW[idx]);
            }
            
        }
        jcv_diagram_free(&diagram);
    }
}

void drawCircle(Image &canvas, int cx, int cy, int radius, RGBPixel color){
    // adapted from https://www.mathsisfun.com/algebra/circle-equations.html
    for(int dy  = -radius; dy <= radius; dy++){
        for(int dx = -radius; dx <= radius; dx++){
            if(dx*dx + dy*dy <= radius*radius){
                int px = cx + dx;
                int py = cy + dy;
                if(px >= 0 && px < canvas.width && py >= 0 && py < canvas.height){
                    canvas.pixels[py * canvas.width + px] = color;
                }
            }
        }
    }
}

int darknessToRadius(float darkness, int minR, int maxR){
    darkness = std::clamp(darkness, 0.0f, 1.0f);
    return minR + static_cast<int>(std::round(std::sqrt(darkness) * (maxR - minR))); // lerp
    // darkness scales with r^2 (because circle), so sqrt(darkness)
}

Image renderStipples(std::vector<jcv_point> &points, std::vector<float> &densityMap, int width, int height){
    Image canvas;
    canvas.width = width;
    canvas.height = height;
    canvas.pixels.assign(static_cast<size_t>(width * height), RGBPixel{255, 255, 255});
    printf("Rendering stippled image...\n");
    for(const jcv_point &p : points){
        int px = static_cast<int>(std::round(p.x));
        int py = static_cast<int>(std::round(p.y));
        if(px < 0 || px >= width || py < 0 || py >= height) continue;
        float d = densityMap[py * width + px];
        int r = darknessToRadius(d, 1, 10);
        drawCircle(canvas, px, py, r, RGBPixel{0, 0, 0});
    }
    return canvas;
}

int main(int argc, char *argv[])
{   
    printf("%d\n", argc);
    for(int i=0; i<argc; i++){
        printf("%s\n", argv[i]);
    }

    CLI::App app{"Weighted Voronoi Stippling by Marks Janis Maizitis as BUas programming homework (Y1Q0)"};
    std::string inputFN, outputFN;
    app.add_option("--i, -i", inputFN, "Input file")->required();
    app.add_option("--o, -o", outputFN, "Output file")->required();
    CLI11_PARSE(app, argc, argv);  

    // open file, close file and save as different one
    Image img = openImage(inputFN);
    if (img.height == 0) {
        return 1;
    }
    std::vector<float> densityMap = computeDarknessMap(img);
    std::vector<Positions> points = seedPoints(densityMap, img);
    std::vector<jcv_point> packedPoints = packPoints(points);
    relaxPoints(packedPoints, densityMap, img.width, img.height, 40);

    Image stipples = renderStipples(packedPoints, densityMap, img.width, img.height);
    writeImage(outputFN, stipples);

    return 0;
}