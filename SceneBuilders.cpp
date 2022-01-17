#include "precomp.h"
#include "SceneBuilders.h"

Scene SceneBuilders::bunch_of_objects()
{
	Scene s = Scene();

	s.triangles.push_back(get_mesh_from_file("./assets/cube.obj", 1.f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/sheep.obj", 0.1f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.0f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/isMountainA.obj", 0.0005f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 1.f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/isHibiscus.obj", 0.01f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/dragon.obj", 4.f, Material::red));
	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 3.f, Material::red));

	for (int i = 0; i < s.triangles.size(); i++) s.bvhs.emplace_back(s.triangles[i], true);

	std::vector<TopBVHNode> bvh_nodes;

	for (int i = 0; i < s.triangles.size(); i++) bvh_nodes.push_back(TopBVHNode{ &s.bvhs[i], float3(-((float)i / 3) * 5 + 7.5,1,-(i % 3) * 5 + 7.5) });

	s.bvh = TopLevelBVH(bvh_nodes, true);

	return s;
}

Scene SceneBuilders::billion_triangles_bunnies()
{
	Scene s = Scene();


	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 1.f, Material::red));
	s.bvhs.emplace_back(s.triangles[0], true);
	
	std::vector<TopBVHNode> bvh_nodes;
	for (int i = 0; i < 10000; i++) { //each bunny has 100k+ triangles
		bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(-((float)i / 100) * 3,0,-(i % 100) * 3)});
	}
	s.bvh = TopLevelBVH(bvh_nodes, true);


	return s;
}

Scene SceneBuilders::billion_triangles_buddhas() {
	Scene s = Scene();

	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 3.f, Material::red));
	s.bvhs.emplace_back(s.triangles[0], true);

	std::vector<TopBVHNode> bvh_nodes;
	for (int i = 0; i < 1000; i++) { //each buddha has 1m+ triangles
		bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(-((float)i / 32) * 3,1.5,-(i % 32) * 3) });
	}
	s.bvh = TopLevelBVH(bvh_nodes, true);

	return s;
}

Scene SceneBuilders::moana_hibiscus_tree()
{
	Scene s = Scene();


	s.triangles.push_back(get_mesh_from_file("./assets/isHibiscus.obj", 0.04f, Material::red)); //of course we trace a literal tree
	s.bvhs.emplace_back(s.triangles[0], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,-8) });
	s.bvh = TopLevelBVH(bvh_nodes, true);

	s.skycolor = float3(0.5294, 0.8078, 0.9215);

	return s;
}

Scene SceneBuilders::teapot()
{
	Scene s = Scene();


	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::white_light));
	s.bvhs.emplace_back(s.triangles[0], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,1,0) });
	s.bvh = TopLevelBVH(bvh_nodes, true);


	return s;
}
Scene SceneBuilders::glass_dragon()
{
	Scene s = Scene();

	s.triangles.push_back({
		Triangle(float3(-20, 0, -20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white), //y sides
		Triangle(float3(20, 0, 20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white),
		Triangle(float3(-20, 20, -20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),
		Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),

		Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white), //z sides
		Triangle(float3(20, 20, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white),
		Triangle(float3(-20, 0, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),
		Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),

		Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white), //x sides
		Triangle(float3(-20, 20, 20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white),
		Triangle(float3(20, 0, -20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white),
		Triangle(float3(20, 20, 20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white)
		});

	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::red_glass));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::cyan));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::mirror));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::white_light));

	s.bvhs.emplace_back(s.triangles[0], true);
	s.bvhs.emplace_back(s.triangles[1], true);
	s.bvhs.emplace_back(s.triangles[2], true);
	s.bvhs.emplace_back(s.triangles[3], true);
	s.bvhs.emplace_back(s.triangles[4], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,0) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[1], float3(-3.5,0,2.5) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[2], float3(3.5,0,-2.5) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[3], float3(3.5,0,2.5) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[4], float3(-3.5,0,-2.5) });
	s.bvh = TopLevelBVH(bvh_nodes, true);


	return s;
}

Scene SceneBuilders::path_tracing_test()
{
	Scene s = Scene();

	s.triangles.push_back({
		Triangle(float3(-20, 0, -20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white), //y sides
		Triangle(float3(20, 0, 20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white),
		//Triangle(float3(-20, 20, -20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),
		//Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),

		//Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white), //z sides
		//Triangle(float3(20, 20, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white),
		//Triangle(float3(-20, 0, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),
		//Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),
		//
		//Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white), //x sides
		//Triangle(float3(-20, 20, 20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white),
		//Triangle(float3(20, 0, -20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white),
		//Triangle(float3(20, 20, 20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white)
		});

	for (int i = 0; i < 8 * 8; i++) s.triangles.push_back(get_mesh_from_file("./assets/sheep.obj", 0.1f, get_random_material()));

	s.bvhs.emplace_back(s.triangles[0], true);
	for (int i = 0; i < 8 * 8; i++) s.bvhs.emplace_back(s.triangles[i+1], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,0) });
	for (int i = 0; i < 8 * 8; i++) bvh_nodes.push_back(TopBVHNode{ &s.bvhs[i+1], float3((i%8)*4-16,1,(i / 8) * 4 - 16) });
	s.bvh = TopLevelBVH(bvh_nodes, true);

	return s;
}