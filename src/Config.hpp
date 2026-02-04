#pragma once

#include <string>
#include <vector>
#include "Vec3.hpp"

struct ObjectConfig {
	std::string path;
	std::string texture;
	Vec3 position;
	Vec3 rotation;
	float scale;
};

struct AppConfig {
	int window_width = 1280;
	int window_height = 720;
	std::string window_title = "42 scop";

	float camera_speed = 2.5f;
	std::string shader_vertex_path = "shaders/normal.vert";
	std::string shader_fragment_path = "shaders/normal.frag";

	std::string skybox_path;

	std::vector<ObjectConfig> objects;
};
