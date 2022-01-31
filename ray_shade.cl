struct transparency_calc_result {
  bool reflection;
  float3 ray_pos;
  float3 ray_dir;
};

transparency_calc_result calc_transparency(float3 ray_dir, float3 normal,
                                           float3 intersection,
                                           struct MaterialData *material,
                                           bool leaving,
                                           struct xorshift_state *rand_state) {

  float angle_in = leaving ? dot(normal, ray_dir) : dot(normal, ray_dir * -1);
  float n1 = leaving ? material.refractive_index : 1.f;
  float n2 = leaving ? 1.f : material.refractive_index;
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

    if (XorRandomFloat(&rand_state) < Fr) {
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

void shade(__global uint *new_ray_index, __global struct Ray *rays,
           const int rand) {

  uint i = get_global_id(0);
  Ray &r = active_rays ? rays2[i] : rays[i];
  if (r.hitptr == 4294967295)
    return; // if default value
  MaterialData material = materials[m_triangles[r.hitptr].m];
  float3 albedo = material.color;
  if (material.isLight) {
    r.E = r.T * albedo;
    return;
  };
  float3 N = m_triangles[r.hitptr].get_normal();
  float3 I = r.o + r.d * r.t;

  xorshift_state rand_state = {rand};
  XorRandomFloat(&rand_state);

  if (material.transparent < 1.f) {
    bool leaving = dot(r.d, N) > 0;

    transparency_calc_result result =
        calc_transparency(r.d, N, I, material, leaving, rand_state);
    if (result.reflection) {
      Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E,
                      r.T * albedo); // todo check E and T calculations
      if (active_rays)
        rays[new_ray_index++] = new_r;
      else
        rays2[new_ray_index++] = new_r;
      return;
    } else {
      if (leaving) {
        float3 color = albedo;
        float3 absorbtion = (-albedo * material.transparent * r.t);
        color.x *= exp(absorbtion.x);
        color.y *= exp(absorbtion.y);
        color.z *= exp(absorbtion.z);
        Ray new_r =
            Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T * color);
        if (active_rays)
          rays[new_ray_index++] = new_r;
        else
          rays2[new_ray_index++] = new_r;
        return;
      }
      Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T);
      if (active_rays)
        rays[new_ray_index++] = new_r;
      else
        rays2[new_ray_index++] = new_r;
      return;
    }
  }
  if (XorRandomFloat(&rand_state) < material.specularity) {
    float3 specular_dir = reflect(r.d, N);
    Ray new_r = Ray(I + specular_dir * epsilon, specular_dir, r.pixel_id, r.E,
                    r.T * albedo); // todo check E and T calculations
    if (active_rays)
      rays[new_ray_index++] = new_r;
    else
      rays2[new_ray_index++] = new_r;
    return;
  } else {
    float3 R = CosineWeightedDiffuseReflection(N, rand);
    float3 BRDF = albedo / PI;
    float PDF = dot(N, R) / PI;
    Ray new_r =
        Ray(I + R * epsilon, R, r.pixel_id, r.E,
            BRDF * (r.T * dot(N, R) / PDF)); // todo check E and T calculations
    if (active_rays)
      rays[new_ray_index++] = new_r;
    else
      rays2[new_ray_index++] = new_r;
    return;
  }
}