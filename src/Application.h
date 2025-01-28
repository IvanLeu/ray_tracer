#include "Window.h"
#include "Input.h"
#include "Graphics.h"
#include "ImguiManager.h"
#include "Renderer.h"
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
	void OnRenderUI();
private:
	ImguiManager manager;
	bool running = true;
	Window wnd;
	Graphics gfx{ wnd };
	Renderer renderer{ gfx };
	std::shared_ptr<InputState> pInputState;
};