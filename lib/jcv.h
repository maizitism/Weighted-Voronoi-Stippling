#pragma once
// Central configuration for jc_voronoi. ALWAYS include this, never lib/jc_voronoi.h
// directly. Forces double precision everywhere: at 1440p, float coordinate spacing
// (~3e-4) exceeds the library's duplicate-pruning epsilon, so Lloyd relaxation drove
// points sub-pixel close without pruning and hung the sweep. Doubles fix it — but
// only if every translation unit agrees on the real type, which is why this lives
// in one place.
#define JCV_REAL_TYPE double
#define JCV_ATAN2 atan2
#define JCV_SQRT  sqrt
#define JCV_FLT_MAX 1.7976931348623157E+308
#include "jc_voronoi.h"