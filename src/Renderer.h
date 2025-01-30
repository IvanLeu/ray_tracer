#pragma once

#include "Graphics.h"
#include "Timer.h"
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"
#include <DirectXMath.h>

class Renderer {
public:
	Renderer(Graphics& gfx);
	void Render(Graphics& gfx, const Scene& scene, const Camera& camera);
	void RenderUI();
private:
	DirectX::XMFLOAT4 TraceRay(const Scene& scene, const Ray& ray) const;
private:
	int m_Width = 0;
	int m_Height = 0;
	Timer timer;
	float lastRenderTime = 0.0f;
	// Scene
	DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 lightDir = { -1.0f, 1.0f, 1.0f };
};