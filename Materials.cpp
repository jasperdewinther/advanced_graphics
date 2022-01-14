#include "precomp.h"
#include "Materials.h"

MaterialData::MaterialData(float3 color, float specularity, float transparent, float refractive_index, bool isLight):
    color(color),
    specularity(specularity),
    transparent(transparent),
    refractive_index(refractive_index),
    isLight(isLight)
{
}
