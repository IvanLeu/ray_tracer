#include "Window.h"
#include "Input.h"
#include "Graphics.h"
#include "ImguiManager.h"
#include "Timer.h"
#include <memory>

class Application {
public:
	Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	int Run();
	~Application();
private:
	void OnUpdate();
	void OnRender();
	DirectX::XMFLOAT4 PerPixel(DirectX::XMFLOAT2 coord) const;
private:
	ImguiManager manager;
	float lastRenderTime = 0.0f;
	Timer timer;
	bool running = true;
	Window wnd;
	Graphics gfx{ wnd };
	std::shared_ptr<InputState> pInputState;
	// Scene
	DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 camera = { 0.0f, 0.0f, -1.0f };
	float radius = 0.5f;
};