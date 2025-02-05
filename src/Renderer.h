#pragma once

#include "Graphics.h"
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"
#include <DirectXMath.h>

class Renderer {
private:
	struct HitPayload {
		float hitDistance;
		DirectX::XMFLOAT3 WorldPosition;
		DirectX::XMFLOAT3 WorldNormal;
		int objectIndex;
	};
public:
	Renderer(Graphics& gfx);
	void Render(Graphics& gfx, const Scene& scene, const Camera& camera);
	void RenderUI();
private:
	DirectX::XMFLOAT4 PerPixel(int x, int y) const; // RayGen
	HitPayload TraceRay(const Ray& ray) const;
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex) const;
	HitPayload Miss() const;
private:
	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
	int m_Width = 0;
	int m_Height = 0;
	float lastRenderTime = 0.0f;
	// Scene
	DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 lightDir = { -1.0f, 1.0f, 1.0f };
};