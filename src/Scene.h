#pragma once

#include <DirectXMath.h>
#include <vector>

struct Sphere {
	float radius = 0.5f;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 albedo;
};

struct Scene {
	std::vector<Sphere> spheres;
};