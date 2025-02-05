#pragma once

#include <DirectXMath.h>
#include <math.h>
#include <algorithm>
#include <random>

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

	inline bool IsZero(const DirectX::XMFLOAT3& v) {
		return v.x == 0 && v.y == 0 && v.z == 0;
	}

	inline DirectX::XMFLOAT4 Clamp(const DirectX::XMFLOAT4& v, const DirectX::XMFLOAT4& min, const DirectX::XMFLOAT4& max) {
		return { std::clamp(v.x, min.x, max.x),
				 std::clamp(v.y, min.y, max.y),
				 std::clamp(v.z, min.z, max.z),
				 std::clamp(v.w, min.w, max.w) };
	}

	inline DirectX::XMFLOAT4 Clamp(const DirectX::XMFLOAT4& v, float min, float max) {
		return { std::clamp(v.x, min, max),
				 std::clamp(v.y, min, max),
				 std::clamp(v.z, min, max),
				 std::clamp(v.w, min, max) };
	}

	inline DirectX::XMFLOAT3 Reflect(const DirectX::XMFLOAT3& v, const DirectX::XMFLOAT3& n) {
		auto normal = Normalize(n);
		return Subtract(v, Scale(normal, 2.0f * Dot(v, normal)));
	}

	inline DirectX::XMFLOAT3 ToFloat3(const DirectX::XMFLOAT4& v) {
		return { v.x, v.y, v.z };
	}

	inline DirectX::XMFLOAT4 ToFloat4(const DirectX::XMFLOAT3& v, float w) {
		return { v.x, v.y, v.z, w };
	}

	inline DirectX::XMFLOAT3 RandomFloat3(float min, float max) {
		static thread_local std::random_device rd;
		static thread_local std::mt19937 generator(rd());
		static thread_local std::uniform_real_distribution<float> dist(min, max);

		return { dist(generator) , dist(generator), dist(generator) };
	}
}