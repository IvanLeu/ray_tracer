#pragma once

#include <DirectXMath.h>
#include <vector>

struct Material {
	DirectX::XMFLOAT4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
};

struct Sphere {
	float radius = 0.5f;
	DirectX::XMFLOAT3 position;
	int materialIndex = 0;
};

struct Scene {
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};