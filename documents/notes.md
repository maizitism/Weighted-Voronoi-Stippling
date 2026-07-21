# Weighted Voronoi Stippling

Stippling is an artistic technique where small dots of ink are placed onto paper such that
their density gives the illusion of tone.

---
Notes on the topic from this youtube video: https://www.youtube.com/watch?v=Bxdt6T_1qgc

* linked and related to Delauney triangulation

for a set of seed points on a 2d plane, the Delauney triangulation algorithm will build a mesh of triangles out of that set of seed points, where their circumcircles (a circle where all 3 vertices of the triangle pass through the circle). The Delauney triangulation is such that none of the seed points live inside any of the circumcircles of any of the triangles. Example program here: https://editor.p5js.org/codingtrain/sketches/fuQz_-FnA

Tutorial uses d3 delaunay for this triangulation

While the result of the Delauney triangulation is the result of triangles, the result of The Voronoi diagram are polygons. For each pixel, color the areas where pixel is closest to one of the seed points. The intersection points of these polygons are centers of the circumcirlces of the associated Delauney triangulation

For each point in the seed points, which point is the closest to it? Then, draw the area where the point is the closest

To now get to the Weighted Voronoi stippling, you need to first look at the Lloyd algorithm (relaxation algorithm)

It could be possible that the seed points are close to each other but in other cells. The idea is to "relax", move these points to the centroid of the polygons.

For this, we need to compute the area and centroid (centre of mass) of each polygon. Refer to this material https://paulbourke.net/geometry/polygonmesh/

