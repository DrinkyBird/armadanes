#pragma once

#include "safewindows.h"

class System;
class D3D9Renderer;

class App {
public:
    App(HINSTANCE hinstance);
    ~App();

    void init();
    void loop();
    void shutdown();

    void setWindowTitle(const char *title = "");

    LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND getHwnd() const { return this->hwnd; }
    System *getSystem() const { return this->system; }

private:
    void openFile();

    HWND hwnd;
    HINSTANCE hinstance;
    HACCEL haccel;
    bool running;

    System *system;
    D3D9Renderer *renderer;
};

extern App *globalApp;
