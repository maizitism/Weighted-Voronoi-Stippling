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

double edge(jcv_point a, jcv_point b, jcv_point c) {return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);}
int main(int argc, char *argv[]){
    CLI::App app{"Weighted Voronoi Stippling by Marks Janis Maizitis as BUas programming homework (Y1Q0)"};
    std::string inputFN, outputFN;
    int iterations, N;
    app.add_option("--i", inputFN, "Input file")->required();
    app.add_option("--o", outputFN, "Output file")->required();
    app.add_option("--iter", iterations, "Number of Lloyd relaxation iterations to perform")->required();
    app.add_option("--n", N, "Number of points to seed")->required();
    CLI11_PARSE(app, argc, argv);
    int W, H; uint8_t* imageData = stbi_load(inputFN.c_str(), &W, &H, nullptr, 3); // force RGB output
    if(!imageData){printf("Image loading failed.\n");return 1;}
    std::vector<float>densityMap(W*H);
    for (int i = 0; i< W*H; i++){ densityMap[i] = 1.f - (0.299f*imageData[i*3]+0.587f*imageData[i*3+1]+0.114f*imageData[i*3+2])/255.f; }
    stbi_image_free(imageData);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, W - 1), distY(0, H - 1);
    std::uniform_real_distribution<float> tDistrib(0.0f, 1.0f);
    std::vector<jcv_point> points; points.reserve(N);
    while (static_cast<int>(points.size()) < N){double x = distX(gen), y = distY(gen);  if (densityMap[W * y + x] > tDistrib(gen)) {points.push_back({x,y});}}
    jcv_rect rect {0.0f, 0.0f, static_cast<jcv_real>(W)-1, static_cast<jcv_real>(H)-1};
    for(int it = 0; it < iterations; it++) {
        printf("Performing Lloyd relaxation iteration %d...\n", it); jcv_diagram diagram{}; jcv_diagram_generate(static_cast<int>(points.size()), points.data(), &rect, nullptr, &diagram);
        std::vector<double> sumW (points.size(), 0.0), sumWX (points.size(), 0.0), sumWY (points.size(), 0.0);
        const jcv_site *sites = jcv_diagram_get_sites(&diagram);
        for(int i = 0; i < diagram.numsites; i++){ // each cell
            const jcv_site *site = &sites[i]; jcv_point C = site->p; int idx = site->index;
            for (const jcv_graphedge *e = site->edges; e; e = e->next) {
                jcv_point A = e->pos[0], B = e->pos[1];
                int x0 = std::max(0, static_cast<int>(std::floor(std::min({C.x, A.x, B.x})))), x1 = std::min(W - 1, static_cast<int>(std::floor(std::max({C.x, A.x, B.x}))));
                int y0 = std::max(0, static_cast<int>(std::floor(std::min({C.y, A.y, B.y})))), y1 = std::min(H - 1, static_cast<int>(std::floor(std::max({C.y, A.y, B.y}))));
                for (double y = y0; y <= y1; y++) for (double x = x0; x <= x1; x++) {
                    double d0 = edge(A, B, {(x),(y)}), d1 = edge(B, C, {(x), (y)}), d2 = edge(C, A, {(x), (y)});
                    if ((d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0)){ sumW[idx] += densityMap[y * W + x]; sumWX[idx] += densityMap[y * W + x] * x; sumWY[idx] += densityMap[y * W + x] * y; }
                }
            }
            if(sumW[idx] > 0.0){points[idx].x = (sumWX[idx] / sumW[idx]); points[idx].y = (sumWY[idx] / sumW[idx]);}
        }jcv_diagram_free(&diagram);
    }
    std::vector<uint8_t> img(W*H*3, 255);
    for(const jcv_point &p : points){int cx = static_cast<int>(std::round(p.x)); int cy = static_cast<int>(std::round(p.y));
        if(cx < 0 || cx >= W || cy < 0 || cy >= H) continue;
        int r = 1 + static_cast<int>(std::round(std::sqrt(std::clamp(densityMap[cy * W + cx], 0.f, 1.f)) * 9));
        for(int dy = -r; dy <= r; dy++) for(int dx = -r; dx <= r; dx++) if(dx*dx + dy*dy <= r*r){
                    int px = cx + dx, py = cy + dy;
                    if(px >= 0 && px < W && py >= 0 && py < H){ int o = ((py*W+px)*3); img[o] = img[o+1] = img[o+2] = 0;}
        }
    }
    printf(stbi_write_bmp(outputFN.c_str(),W,H,3,img.data()) ? "Image written.\n" : "Image failed to save.\n");
}