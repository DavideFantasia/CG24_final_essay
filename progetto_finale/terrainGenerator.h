#pragma once

#include "..\common\simple_shapes.h"

void TerrainGenerator(shape *plane) {
	(*plane) = shape_maker::terrain(1000, 1000);
}