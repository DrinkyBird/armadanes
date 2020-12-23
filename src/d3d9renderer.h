#pragma once
#include <d3d9.h>

class App;

class D3D9Renderer {
public:
    D3D9Renderer(App *app);
    ~D3D9Renderer();

    bool init();
    void shutdown();

    void render();

private:
    App *app;

    LPDIRECT3D9 d3d9;
    LPDIRECT3DDEVICE9 device;
};


