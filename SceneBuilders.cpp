#include "precomp.h"
#include "SceneBuilders.h"

Scene SceneBuilders::bunch_of_objects()
{
	Scene s = Scene();

	s.triangles.push_back({
		Triangle(float3(-20, 0, -20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white),
		Triangle(float3(20, 0, 20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white),
	});

	s.triangles.push_back(get_mesh_from_file("./assets/cube.obj", 1.f, Material::white_light));
	s.triangles.push_back(get_mesh_from_file("./assets/dragon.obj", 5.f, Material::emerald));
	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 6.0f, Material::glass));
	s.triangles.push_back(get_mesh_from_file("./assets/sheep.obj", 0.1f, Material::magenta));
	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 3.f, Material::cyan));

	s.bvhs.emplace_back(s.triangles[0], true);
	s.bvhs.emplace_back(s.triangles[1], true);
	s.bvhs.emplace_back(s.triangles[2], true);
	s.bvhs.emplace_back(s.triangles[3], true);
	s.bvhs.emplace_back(s.triangles[4], true);
	s.bvhs.emplace_back(s.triangles[5], true);

	std::vector<TopBVHNode> bvh_nodes;

	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,0) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[1], float3(0,1.001,0) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[2], float3(6,1.414,0) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[3], float3(0,2.676,6) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[4], float3(0,1.258,-6) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[5], float3(-6,-0.062,0) });

	s.bvh = TopLevelBVH(bvh_nodes, true);

	return s;
}

Scene SceneBuilders::billion_triangles_bunnies()
{
	Scene s = Scene();

	s.triangles.push_back({
	Triangle(float3(-200, 0, -200),float3(-200, 0, 200), float3(200, 0, -200), float3(0,1,0), Material::white_light),
	Triangle(float3(200, 0, 200),float3(-200, 0, 200), float3(200, 0, -200), float3(0,1,0), Material::white_light),
		});


	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 1.f, Material::red));
	s.bvhs.emplace_back(s.triangles[0], true);
	s.bvhs.emplace_back(s.triangles[1], true);
	
	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,0)});
	for (int i = 0; i < 10000; i++) { //each bunny has 100k+ triangles
		bvh_nodes.push_back(TopBVHNode{ &s.bvhs[1], float3(-((float)i / 100) * 3,0,-(i % 100) * 3)});
	}
	s.bvh = TopLevelBVH(bvh_nodes, true);


	return s;
}


Scene SceneBuilders::glass_dragon()
{
	Scene s = Scene();

	s.triangles.push_back({
		Triangle(float3(-20, 0, -20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white), //y sides
		Triangle(float3(20, 0, 20),float3(-20, 0, 20), float3(20, 0, -20), float3(0,1,0), Material::white),
		/*Triangle(float3(-20, 20, -20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),
		Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 20, -20), float3(0,-1,0), Material::white),

		Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white), //z sides
		Triangle(float3(20, 20, -20),float3(-20, 20, -20), float3(20, 0, -20), float3(0,0,1), Material::white),
		Triangle(float3(-20, 0, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),
		Triangle(float3(20, 20, 20),float3(-20, 20, 20), float3(20, 0, 20), float3(0,0,-1), Material::white),

		Triangle(float3(-20, 0, -20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white), //x sides
		Triangle(float3(-20, 20, 20),float3(-20, 20, -20), float3(-20, 0, 20), float3(1,0,0), Material::white),
		Triangle(float3(20, 0, -20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white),
		Triangle(float3(20, 20, 20),float3(20, 20, -20), float3(20, 0, 20), float3(-1,0,0), Material::white)*/
		});

	s.triangles.push_back(get_mesh_from_file("./assets/dragon.obj", 5.f, Material::red_glass));
	s.triangles.push_back(get_mesh_from_file("./assets/buddha.obj", 6.f, Material::yellow));
	s.triangles.push_back(get_mesh_from_file("./assets/teapot.obj", 1.f, Material::cyan));
	s.triangles.push_back(get_mesh_from_file("./assets/bunny.obj", 3.f, Material::white_light));

	s.bvhs.emplace_back(s.triangles[0], true);
	s.bvhs.emplace_back(s.triangles[1], true);
	s.bvhs.emplace_back(s.triangles[2], true);
	s.bvhs.emplace_back(s.triangles[3], true);
	s.bvhs.emplace_back(s.triangles[4], true);

	std::vector<TopBVHNode> bvh_nodes;
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[0], float3(0,0,0) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[1], float3(-3.5,5,2.5) });
	bvh_nodes.push_back(TopBVHNode{ &s.bvhs[2], float3(3.5,6.001,-2.5) });
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
	for (int i = 0; i < 8 * 8; i++) bvh_nodes.push_back(TopBVHNode{ &s.bvhs[i+1], float3((i%8)*4-16,1.26,(i / 8) * 4 - 16) });
	s.bvh = TopLevelBVH(bvh_nodes, true);

	return s;
}