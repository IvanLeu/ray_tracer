#include "Renderer.h"
#include "VectorUtils.h"
#include "imgui.h"

Renderer::Renderer(Graphics& gfx)
	:
	m_Width(gfx.GetWidth()),
	m_Height(gfx.GetHeight())
{
	gfx.SetTextureClearColor(clearColor);
}

void Renderer::Render(Graphics& gfx)
{
	timer.Mark();
	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {
			DirectX::XMFLOAT2 coord = { (float)x / (float)m_Width, (float)y / (float)m_Height };
			coord.x = coord.x * 2.0f - 1.0f;
			coord.y = coord.y * 2.0f - 1.0f;
			gfx.PutPixel(x, y, PerPixel(coord));
		}
	}
	lastRenderTime = timer.Mark();
}

void Renderer::RenderUI()
{
	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat("Radius", &radius, 0.5f, 1.5f);

	ImGui::ColorEdit3("Sphere color", &sphereColor.x);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::End();
}

DirectX::XMFLOAT4 Renderer::PerPixel(DirectX::XMFLOAT2 coord) const
{
	coord.x *= (float)m_Width / (float)m_Height;

	DirectX::XMFLOAT3 dir = { coord.x, coord.y, 1.0f };

	float a = Utils::Dot(dir, dir);
	float b = 2.0f * Utils::Dot(cameraPos, dir);
	float c = Utils::Dot(cameraPos, cameraPos) - radius * radius;

	float D = b * b - 4.0f * a * c;

	if (D < 0.0f) {
		return clearColor;
	}

	const float closestHit = (-b - sqrt(D)) / (2.0f * a);
	const DirectX::XMFLOAT3 hitPos = Utils::Add(cameraPos, Utils::Scale(dir, closestHit));
	const DirectX::XMFLOAT3 normal = Utils::Normalize(hitPos);

	const float f = max(Utils::Dot(normal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
	const DirectX::XMFLOAT4 finalColor = { sphereColor.x * f, sphereColor.y * f, sphereColor.z * f, 1.0f };

	return finalColor;
}
