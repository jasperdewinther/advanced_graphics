#include "precomp.h"
#include "Materials.h"

MaterialData::MaterialData(std::variant<float3, std::function<float3(float3, float3)>> color, float specularity, float transparent, float refractive_index, bool isLight):
    color(color),
    specularity(specularity),
    transparent(transparent),
    refractive_index(refractive_index),
    isLight(isLight)
{
}

float3 MaterialData::get_color(const float3& pos, const float3& norm) const
{
    if (const float3* val = std::get_if<float3>(&color))
        return *val;
    else {
        const std::function<float3(float3, float3)>* func = std::get_if<std::function<float3(float3, float3)>>(&color);
        return func->operator()(pos, norm);
    }
}

