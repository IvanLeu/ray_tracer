#include "Window.h"
#include "Input.h"
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
private:
	bool running = true;
	Window wnd;
	std::shared_ptr<InputState> pInputState;
};