#include "Renderer.h"
#include "VectorUtils.h"
#include "imgui.h"

#include <chrono>
#include <numeric>
#include <ranges>
#include <algorithm>
#include <execution>

Renderer::Renderer(Graphics& gfx)
	:
	m_Width(gfx.GetWidth()),
	m_Height(gfx.GetHeight()),
	m_AccumulationData(new DirectX::XMFLOAT4[m_Width * m_Height])
{
	gfx.SetTextureClearColor(clearColor);
	m_VerticalIter.resize(m_Height);
	m_HorizontalIter.resize(m_Width);
	std::ranges::iota(m_VerticalIter, 0);
	std::ranges::iota(m_HorizontalIter, 0);
}

void Renderer::Render(Graphics& gfx, const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1u) {
		memset(m_AccumulationData.get(), 0, sizeof(DirectX::XMFLOAT4) * m_Width * m_Height);
	}

	auto start = std::chrono::high_resolution_clock::now();

#define MT 1
#ifdef MT
	std::for_each(std::execution::par, m_VerticalIter.begin(), m_VerticalIter.end(),
		[this, &gfx](uint64_t y) {
			std::for_each(std::execution::par, m_HorizontalIter.begin(), m_HorizontalIter.end(),
				[this, y, &gfx](uint64_t x) {
					auto color = PerPixel(x, y);
					color.w = 1.0f;

					m_AccumulationData[x + y * m_Width] = Utils::Add(m_AccumulationData[x + y * m_Width], color);
					color = Utils::Scale(m_AccumulationData[x + y * m_Width], (1.0f / (float)m_FrameIndex));

					color = Utils::Clamp(color, 0.0f, 1.0f);
					gfx.PutPixel((int)x, (int)y, color);
				}
			);
		}
	);
#else
	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {

			auto color = PerPixel(x, y);
			color.w = 1.0f;

			m_AccumulationData[x + y * m_Width] = Utils::Add(m_AccumulationData[x + y * m_Width], color);
			color = Utils::Scale(m_AccumulationData[x + y * m_Width], (1.0f / (float)m_FrameIndex));

			color = Utils::Clamp(color, 0.0f, 1.0f);
			gfx.PutPixel(x, y, color);
		}
	}
#endif
	auto end = std::chrono::high_resolution_clock::now();

	lastRenderTime = std::chrono::duration<float, std::milli>(end - start).count();

	if (m_Accumulate)
		++m_FrameIndex;
	else
		m_FrameIndex = 1u;
}

void Renderer::RenderUI()
{
	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::Separator();

	ImGui::Checkbox("Accumulate", &m_Accumulate);
	if (ImGui::Button("Reset")) {
		ResetFrameIndex();
	}

	ImGui::End();
}

void Renderer::ResetFrameIndex()
{
	m_FrameIndex = 1u;
}

DirectX::XMFLOAT4 Renderer::PerPixel(uint64_t x, uint64_t y)
{
	Ray ray;
	DirectX::XMStoreFloat3(&ray.origin, m_ActiveCamera->GetPosition());
	ray.direction = m_ActiveCamera->GetRayDirections()[x + y * m_Width];

	DirectX::XMFLOAT3 color = { 0.0f, 0.0f, 0.0f };
	float multiplier = 1.0f;

	int nBounces = 5;
	for (int i = 0; i < nBounces; ++i) {
		HitPayload payload = TraceRay(ray);

		if (payload.hitDistance < 0.0f) {
			color = Utils::Add(color, Utils::Scale(Utils::ToFloat3(clearColor), multiplier));
			break;
		}

		const float f = std::max(Utils::Dot(payload.WorldNormal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
		
		const Sphere& sphere = m_ActiveScene->spheres[payload.objectIndex];
		const Material& material = m_ActiveScene->materials[sphere.materialIndex];
		auto sphereColor = Utils::Scale(Utils::ToFloat3(material.Albedo), f);
		color = Utils::Add(color, Utils::Scale(sphereColor, multiplier));

		multiplier *= 0.5f;

		ray.origin = Utils::Add(payload.WorldPosition, Utils::Scale(payload.WorldNormal, 0.0001f));
		ray.direction = Utils::Reflect(ray.direction, 
			Utils::Add(payload.WorldNormal, Utils::Scale(Utils::RandomFloat3(-0.5f, 0.5f), material.Roughness)));
	}

	return Utils::ToFloat4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->spheres.size(); ++i) {
		const Sphere& sphere = m_ActiveScene->spheres[i];

		DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, sphere.position);

		float a = Utils::Dot(ray.direction, ray.direction);
		float b = 2.0f * Utils::Dot(origin, ray.direction);
		float c = Utils::Dot(origin, origin) - sphere.radius * sphere.radius;

		float D = b * b - 4.0f * a * c;

		if (D < 0.0f) {
			continue;
		}

		const float closestHit = (-b - sqrt(D)) / (2.0f * a);
		if (closestHit >= 0.0f && closestHit < hitDistance) {
			hitDistance = closestHit;
			closestSphere = (int)i;
		}
	}

	if (closestSphere == -1) {
		return Miss();
	}
	
	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) const
{
	HitPayload payload;
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	const Sphere& sphere = m_ActiveScene->spheres[objectIndex];
	DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, sphere.position);

	payload.WorldPosition = Utils::Add(origin, Utils::Scale(ray.direction, hitDistance));
	payload.WorldNormal = Utils::Normalize(payload.WorldPosition);

	payload.WorldPosition = Utils::Add(payload.WorldPosition, sphere.position);

	return payload;
}

Renderer::HitPayload Renderer::Miss() const
{
	HitPayload payload;
	payload.hitDistance = -1.0f;
	return payload;
}
