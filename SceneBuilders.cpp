#include "precomp.h"
#include "SceneBuilders.h"

Scene SceneBuilders::original()
{
    Scene s = Scene();
	s.spheres = {
		Sphere(float3(0, 2, 0), 1.5, Material::glass),
		Sphere(float3(3, 1, 2.5), 1, Material::red_glass),
		Sphere(float3(-3, 1, -1), 1, Material::cyan),
		Sphere(float3(-4, 2, -4), 1, Material::mirror),
		Sphere(float3(6, 4, -6), 4, Material::red_glass),
		Sphere(float3(0, 2, -6), 2, Material::red_glass),
		Sphere(float3(-4, 1, -6), 1, Material::red_glass),
		Sphere(float3(-6, 0.5, -6), 0.5, Material::red_glass)
	};
	s.planes = {
		Plane(float3(0, 1, 0), 0, Material::checkerboard),
		Plane(float3(-1, 0, 0), 20, Material::white),
		Plane(float3(0, 0, -1), 20, Material::white)
	};
	s.triangles.push_back(get_mesh_from_file("./assets/sheep.obj", 0.1f, Material::glass));
	s.bvhs.emplace_back(s.triangles[0], true);
	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,2,4) });
	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.point_lights.push_back(PointLight(float3(19, 10, 19), float3(1, 1, 1), 50000.0));
	s.spot_lights.push_back(SpotLight(float3(15, 10, 0), float3(0, -1, 0), 0.5f, float3(0.1, 0.5, 0.99), 30000.f));
	s.directional_lights.push_back(DirectionalLight(float3(1, -1, 0.5), float3(0.9, 0.9, 0.9), 0.7));

    return s;
}

Scene SceneBuilders::bunch_of_objects()
{
	Scene s = Scene();

	s.planes.push_back(Plane(float3(0, 1, 0), 0, Material::checkerboard));

	s.triangles.push_back(get_mesh_from_file("./assets/cube.obj", 1.f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/sheep.obj", 0.1f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.0f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/isMountainA.obj", 0.0005f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 1.f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/isHibiscus.obj", 0.01f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/dragon.obj", 4.f, Material::normal));
	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 3.f, Material::normal));

	for (int i = 0; i < s.triangles.size(); i++) s.bvhs.emplace_back(s.triangles[i], true);

	std::vector<TopBVHNode> bvh_nodes;

	for (int i = 0; i < s.triangles.size(); i++) bvh_nodes.push_back(TopBVHNode{ &s.bvhs[i], float3(-((float)i / 3) * 5 + 7.5,1,-(i % 3) * 5 + 7.5) });

	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.point_lights.push_back(PointLight(float3(0, 100, 0), float3(1, 1, 1), 500000000.0));
	s.directional_lights.push_back(DirectionalLight(float3(1, -1, 0.5), float3(0.9, 0.9, 0.9), 0.7));


	return s;
}

Scene SceneBuilders::billion_triangles_bunnies()
{
	Scene s = Scene();

	s.planes.push_back(Plane(float3(0, 1, 0), 0, Material::checkerboard));

	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 1.f, Material::normal));
	s.bvhs.emplace_back(s.triangles[0], true);
	
	std::vector<TopBVHNode> bvh_nodes;
	for (int i = 0; i < 10000; i++) { //each bunny has 100k+ triangles
		bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(-((float)i / 100) * 3,0,-(i % 100) * 3) });
	}
	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.point_lights.push_back(PointLight(float3(0, 100, 0), float3(1, 1, 1), 50000000000.0));

	return s;
}

Scene SceneBuilders::billion_triangles_buddhas() {
	Scene s = Scene();

	s.planes.push_back(Plane(float3(0, 1, 0), 0, Material::checkerboard));

	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 3.f, Material::normal));
	s.bvhs.emplace_back(s.triangles[0], true);

	std::vector<TopBVHNode> bvh_nodes;
	for (int i = 0; i < 1000; i++) { //each buddha has 1m+ triangles
		bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(-((float)i / 32) * 3,1.5,-(i % 32) * 3) });
	}
	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.point_lights.push_back(PointLight(float3(0, 100, 0), float3(1, 1, 1), 500000000.0));

	return s;
}

Scene SceneBuilders::hibiscus_tree()
{
	Scene s = Scene();

	s.planes.push_back(Plane(float3(0, 1, 0), 0, Material::reflective_blue));

	s.triangles.push_back(get_mesh_from_file("./assets/isHibiscus.obj", 0.04f, Material::normal)); //of course we trace a literal tree
	s.bvhs.emplace_back(s.triangles[0], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,-8) });
	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.point_lights.push_back(PointLight(float3(0, 100, 0), float3(1, 1, 1), 500000000.0));
	s.skycolor = float3(0.5294, 0.8078, 0.9215);

	return s;
}
