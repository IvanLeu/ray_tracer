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
		return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v) {
		float length = Magnitude(v);
		return { v.x / length, v.y / length, v.z / length };
	}

	inline DirectX::XMFLOAT3 Subtract(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) {
		return {
			v1.x - v2.x,
			v1.y - v2.y,
			v1.z - v2.z,
		};
	}

	inline DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) {
		return {
			v1.x + v2.x,
			v1.y + v2.y,
			v1.z + v2.z,
		};
	}

	inline DirectX::XMFLOAT3 Scale(const DirectX::XMFLOAT3& v1, float scalar) {
		return {
			v1.x * scalar,
			v1.y * scalar,
			v1.z * scalar,
		};
	}

	inline DirectX::XMFLOAT3 Negate(const DirectX::XMFLOAT3& v1) {
		return {
			-v1.x,
			-v1.y,
			-v1.z,
		};
	}

	inline bool IsZero(const DirectX::XMFLOAT3 v) {
		return v.x == 0 && v.y == 0 && v.z == 0;
	}
}