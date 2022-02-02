#include "common.cl"

struct transparency_calc_result {
  bool reflection;
  float3 ray_pos;
  float3 ray_dir;
};

float3 reflect(float3 incidentVec, float3 normal) {
  // https://asawicki.info/news_1301_reflect_and_refract_functions.html
  return incidentVec - 2.f * dot(incidentVec, normal) * normal;
}

float3 CosineWeightedDiffuseReflection(float3 normal,
                                       struct xorshift_state *rand_state) {
  // XorRandomFloat(rand_state); // TODO find out why these two initialising
  // randoms are needed for proper distribution
  XorRandomFloat(rand_state);
  // XorRandomFloat(rand_state);
  // XorRandomFloat(rand_state);
  float r0 = XorRandomFloat(rand_state), r1 = XorRandomFloat(rand_state);
  float r = sqrt(r0);
  float theta = 2 * M_PI_F * r1;
  float x = r * cos(theta);
  float y = r * sin(theta);
  float3 dir = {x, y, sqrt(1 - r0)};
  dir = normalize(dir);
  return (dot(dir, normal) > 0.f) ? dir : dir * -1;
}

struct transparency_calc_result
calc_transparency(float3 ray_dir, float3 normal, float3 intersection,
                  struct MaterialData *material, bool leaving,
                  struct xorshift_state *rand_state) {

  float angle_in = leaving ? dot(normal, ray_dir) : dot(normal, ray_dir * -1);
  float n1 = leaving ? material->refractive_index : 1.f;
  float n2 = leaving ? 1.f : material->refractive_index;
  float refractive_ratio = n1 / n2;
  float k = 1.f - pow(refractive_ratio, 2.f) * (1 - pow(angle_in, 2.f));
  if (k < 0.f) {
    // total internal reflection
    float3 specular_dir =
        leaving ? reflect(ray_dir, normal * -1) : reflect(ray_dir, normal);
    struct transparency_calc_result r = {
        true, intersection + specular_dir * epsilon, specular_dir};
    return r;
  } else {
    float3 new_dir =
        normalize(refractive_ratio * ray_dir +
                  normal * (refractive_ratio * angle_in - sqrt(k)));

    float angle_out =
        leaving ? dot(normal, new_dir) : dot(normal * -1, new_dir);

    float Fr_par =
        pow((n1 * angle_in - n2 * angle_out) / (n1 * angle_in + n2 * angle_out),
            2.f);
    float Fr_per =
        pow((n1 * angle_out - n2 * angle_in) / (n1 * angle_out + n2 * angle_in),
            2.f);
    float Fr = (Fr_par + Fr_per) / 2.f;

    if (XorRandomFloat(rand_state) < Fr) {
      float3 specular_dir =
          leaving ? reflect(ray_dir, normal * -1) : reflect(ray_dir, normal);
      struct transparency_calc_result r = {
          true, intersection + specular_dir * epsilon, specular_dir};
      return r;
    } else {
      struct transparency_calc_result r = {
          false, intersection + new_dir * epsilon, new_dir};
      return r;
    }
  }
}

struct Ray build_ray(float3 ray_pos, float3 ray_dir, uint pixel_id, float4 E,
                     float4 T) {
  float4 dir = {ray_dir.x, ray_dir.y, ray_dir.z, 0};
  struct Ray r = {
      .o = {ray_pos.x, ray_pos.y, ray_pos.z, 0},
      .d = dir,
      .invDir = 1.f / dir,
      .t = 999999.f,
      .pixel_id = pixel_id,
      .E = E,
      .T = T,
      .hitptr = 4294967295,
  };
  return r;
}

__kernel void shade(__global uint *new_ray_index, __global struct Ray *rays,
                    __global struct Ray *rays2,
                    __global struct Triangle *m_triangles, int active_rays,
                    int rand, int max_i) {

  uint i = get_global_id(0);
  struct Ray r = active_rays ? rays2[i] : rays[i];
  if (i > max_i)
    return;

  if (r.hitptr == 4294967295)
    return; // if default value

  struct MaterialData material = materials[m_triangles[r.hitptr].m];
  float4 albedo = material.color;
  if (material.isLight) {
    r.E = r.T * albedo;
    if (active_rays)
      rays2[i] = r;
    else
      rays[i] = r;
    return;
  };
  float4 N = m_triangles[r.hitptr].normal;
  float4 I = r.o + r.d * r.t;

  struct xorshift_state rand_state = {rand + i};
  XorRandomFloat(&rand_state);

  if (material.transparent < 1.f) {
    bool leaving = dot(r.d.xyz, N.xyz) > 0;

    struct transparency_calc_result result = calc_transparency(
        r.d.xyz, N.xyz, I.xyz, &material, leaving, &rand_state);
    if (result.reflection) {
      struct Ray new_r =
          build_ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E,
                    r.T * albedo); // todo check E and T calculations
      if (active_rays)
        rays[atomic_add(new_ray_index, 1)] = new_r;
      else
        rays2[atomic_add(new_ray_index, 1)] = new_r;
      return;
    } else {
      if (leaving) {
        float4 color = albedo;
        float4 absorbtion = (-albedo * material.transparent * r.t);
        color.x *= exp(absorbtion.x);
        color.y *= exp(absorbtion.y);
        color.z *= exp(absorbtion.z);
        struct Ray new_r = build_ray(result.ray_pos, result.ray_dir, r.pixel_id,
                                     r.E, r.T * color);

        if (active_rays)
          rays[atomic_add(new_ray_index, 1)] = new_r;
        else
          rays2[atomic_add(new_ray_index, 1)] = new_r;

        return;
      }
      struct Ray new_r =
          build_ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T);
      if (active_rays)
        rays[atomic_add(new_ray_index, 1)] = new_r;
      else
        rays2[atomic_add(new_ray_index, 1)] = new_r;

      return;
    }
  }
  if (XorRandomFloat(&rand_state) < material.specularity) {
    float3 specular_dir = reflect(r.d.xyz, N.xyz);
    struct Ray new_r =
        build_ray(I.xyz + specular_dir * epsilon, specular_dir, r.pixel_id, r.E,
                  r.T * albedo); // todo check E and T calculations

    if (active_rays)
      rays[atomic_add(new_ray_index, 1)] = new_r;
    else
      rays2[atomic_add(new_ray_index, 1)] = new_r;
    return;
  } else {
    float3 R = CosineWeightedDiffuseReflection(N.xyz, &rand_state);
    float4 BRDF = albedo / M_PI_F;
    float PDF = dot(N.xyz, R) / M_PI_F;
    struct Ray new_r = build_ray(
        I.xyz + R * epsilon, R, r.pixel_id, r.E,
        BRDF * (r.T * dot(N.xyz, R) / PDF)); // todo check E and T calculations
    if (active_rays)
      rays[atomic_add(new_ray_index, 1)] = new_r;
    else
      rays2[atomic_add(new_ray_index, 1)] = new_r;
    return;
  }
}