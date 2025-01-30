#pragma once

#include <DirectXMath.h>
#include <vector>

class Camera {
public:
	Camera(int width, int height, float fovAngleYDegrees, float nearClip, float farClip);
	DirectX::XMMATRIX GetProjection() const noexcept;
	DirectX::XMMATRIX GetInverseProjection() const noexcept;
	DirectX::XMMATRIX GetView() const noexcept;
	DirectX::XMMATRIX GetInverseView() const noexcept;
	DirectX::XMVECTOR GetPosition() const noexcept;
	const std::vector<DirectX::XMFLOAT3>& GetRayDirections() const noexcept;
	void Move(float dt, DirectX::XMFLOAT3 v) noexcept;
	void Rotate(float dt, float deltaX, float deltaY) noexcept;
	void Update() noexcept;
	void SetMoveSpeed(float speed) noexcept;
	void SetRotationSpeed(float speed) noexcept;
private:
	void RecalculateProjection() noexcept;
	void RecalculateView() noexcept;
	void RecalculateRayDirections() noexcept;
private:
	bool moved = false;
	int m_Width = 0;
	int m_Height = 0;
	float fovAngleY = 45.0f;
	float aspectRatio = 1.0f;
	float nearClip = 0.01f;
	float farClip = 100.0f;
	float moveSpeed = 1.5f;
	float rotationSpeed = 0.2f;
	DirectX::XMVECTOR m_Position;
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_InverseProjection;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_InverseView;
	DirectX::XMVECTOR m_forwardDir;
	DirectX::XMVECTOR m_rightDir;
	std::vector<DirectX::XMFLOAT3> rayDirections;
};