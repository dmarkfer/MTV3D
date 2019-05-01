#pragma once

#include "Window.h"
#include <vector>
#include <utility>
#include <map>

class App {
private:
	std::map<std::string, WNDCLASSEXW> wndClassTypeStruct;
	std::unique_ptr<Window> hSplashWnd;
	std::unique_ptr<Window> hMainWnd;
	std::vector<std::pair<std::unique_ptr<Window>, std::unique_ptr<Window>>> hVisWnd;
public:
	HINSTANCE hCurrentInst;

	App() = default;
	~App() = default;

	int run(HINSTANCE hInstance, int nCmdShow);
private:
	void createWndClasses();
};
