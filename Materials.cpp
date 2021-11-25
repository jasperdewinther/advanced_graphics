#include "precomp.h"
#include "Materials.h"

float3 MaterialData::get_color(const float3& pos, const float3& norm)
{
    if (const float3* val = std::get_if<float3>(&color))
        return *val;
    else {
        const std::function<float3(float3, float3)>* func = std::get_if<std::function<float3(float3, float3)>>(&color);
        return func->operator()(pos, norm);
    }
}
