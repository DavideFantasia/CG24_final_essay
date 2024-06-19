#pragma once

#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"

void TerrainGenerator(shape *plane) {
	(*plane) = shape_maker::terrain(500, 500);
}