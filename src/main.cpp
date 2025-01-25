#include "Application.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd) 
{
	try {
		return Application{}.Run();
	}
	catch (std::exception& e) {
		MessageBox(nullptr, e.what(), "An exception occured", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
	}
	catch (...) {
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
	}

	return -1;
}