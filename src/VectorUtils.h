#pragma once

#include <DirectXMath.h>
#include <math.h>

namespace Utils 
{
	inline float Dot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}

	inline float Magnitude(const DirectX::XMFLOAT3& v) {
		return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v) {
		float length = Magnitude(v);
		return { v.x / length, v.y / length, v.z / length };
	}
}