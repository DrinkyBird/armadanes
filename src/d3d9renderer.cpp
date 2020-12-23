#include <cstdio>
#include "d3d9renderer.h"
#include "main.h"

#define SAFE_RELEASE(obj)           \
    (obj)->Release();               \
    obj = nullptr;

D3D9Renderer::D3D9Renderer(App *app) {
    this->app = app;
    this->d3d9 = nullptr;
    this->device = nullptr;
}

D3D9Renderer::~D3D9Renderer() {
}

bool D3D9Renderer::init() {
    HRESULT hresult;

    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d9) {
        return false;
    }

    D3DADAPTER_IDENTIFIER9 identifier;
    d3d9->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &identifier);
    printf("Driver: %s\n", identifier.Driver);
    printf("Description: %s\n", identifier.Description);
    printf("Device Name: %s\n", identifier.DeviceName);
    printf("Vendor ID: %04lx\n", identifier.VendorId);
    printf("Device ID: %04lx\n", identifier.DeviceId);
    printf("Subsys ID: %04lx\n", identifier.SubSysId);
    printf("Revision: %04lx\n", identifier.Revision);

    RECT rect;
    GetClientRect(app->getHwnd(), &rect);

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(D3DPRESENT_PARAMETERS));
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = app->getHwnd();
    pp.Windowed = true;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    hresult = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, app->getHwnd(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device);
    if (hresult != D3D_OK) {
        return false;
    }

    return true;
}

void D3D9Renderer::shutdown() {
    SAFE_RELEASE(device);
    SAFE_RELEASE(d3d9);
}

void D3D9Renderer::render() {
    device->Clear(0, nullptr, D3DCLEAR_TARGET, 0xFFFF0000, 1.0f, 0);
    device->BeginScene();

    device->EndScene();
    device->Present(nullptr, nullptr, nullptr, nullptr);
}