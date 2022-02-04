#include "precomp.h"
#include "BVHNode.h"

float3 BVHNode::get_center()
{
	return float3(minx + (maxx - minx) / 2, miny + (maxy - miny) / 2, minz + (maxz - minz) / 2);
}