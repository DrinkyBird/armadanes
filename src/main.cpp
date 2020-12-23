#include <cstdio>
#include "main.h"
#include "safewindows.h"
#include <commdlg.h>
#include "resource.h"
#include "system.h"
#include <cstdio>
#include "d3d9renderer.h"
#include "rom.h"
#include "cpubus.h"

#define WINDOWCLASS "ArmadaNesWindowClass"

App *globalApp = nullptr;

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK aboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

App::App(HINSTANCE hinstance) {
    setbuf(stdout, 0);

    ::globalApp = this;

    this->hwnd = nullptr;
    this->hinstance = hinstance;
    this->haccel = nullptr;
    this->running = false;
    this->system = nullptr;
    this->renderer = nullptr;
}

App::~App() {

}

void App::init() {
#ifndef NDEBUG
	AllocConsole();
	SetConsoleTitleA("vpg console");
	FILE *fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif

    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(WNDCLASSEX));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ::wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hinstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName = WINDOWCLASS;
    wcex.hIconSm = LoadIcon(hinstance, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    RECT r;
    r.top = 0;
    r.bottom = 480;
    r.left = 0;
    r.right = 512;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, TRUE);
    int width = r.right - r.left;
    int height = r.bottom - r.top;

    hwnd = CreateWindow(WINDOWCLASS, "ArmadaNES", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hinstance, nullptr);
    setWindowTitle();
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    haccel = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDR_MAINACCEL));
    if (!haccel) {
        MessageBox(hwnd, TEXT("Failed to load accelerator"), TEXT("Warning"), MB_OK|MB_ICONWARNING);
    }

    this->renderer = new D3D9Renderer(this);
    this->renderer->init();
}

void App::loop() {
    running = true;

    MSG msg;
    while (running) {
        if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
            if (!TranslateAccelerator(msg.hwnd, haccel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            if (system != nullptr) {
                system->tick();
            }

            renderer->render();
        }
    }

    running = false;
}

void App::shutdown() {
    renderer->shutdown();
    DestroyWindow(hwnd);
}

void App::setWindowTitle(const char *title) {
    const char *base = "ArmadaNES (" __DATE__ " " __TIME__ ")";

    char buf[512];
    if (title != nullptr && title[0] != '\0') {
        snprintf(buf, 512, "%s - %s", title, base);
    } else {
        snprintf(buf, 512, "%s", base);
    }

    SetWindowText(hwnd, buf);
}

LRESULT App::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);

            switch (wmId) {
                case IDM_OPEN: {
                    openFile();
                    break;
                }

                case IDM_ABOUT: {
                    DialogBox(hinstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, aboutProc);
                    break;
                }

                case IDM_DUMPROM: {
                    if (system && system->getRom()) {
                        system->getRom()->dump();
                    }
                    break;
                }

                case IDM_DUMPBUS: {
                    if (system && system->getBus()) {
                        system->getBus()->dump();
                    }
                    break;
                }

                case IDM_RESET: {
                    if (system) {
                        system->reset();
                    }
                    break;
                }

                default: {
                    break;
                }
            }

            break;
        }

        case WM_CLOSE:
            running = false;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void App::openFile() {
    TCHAR path[MAX_PATH];
    path[0] = '\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = TEXT("iNES ROMs\0*.nes\0All files\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = TEXT("Select a ROM");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn)) {
        if (system != nullptr) {
            delete system;
        }

        system = new System;
        if (!system->loadRom(path)) {
            MessageBox(hwnd, TEXT("Failed to load ROM"), TEXT("Error"), MB_OK|MB_ICONERROR);
        } else {
            setWindowTitle(path);
            system->start();
        }
    }
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return globalApp->wndProc(hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK aboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            LOGFONT logfont;
            HWND ctl = GetDlgItem(hDlg, IDC_BOLD);
            HFONT hfont = (HFONT)SendMessage(ctl, WM_GETFONT, 0, 0);

            GetObject(hfont, sizeof logfont, &logfont);
            logfont.lfWeight = FW_BOLD;
            hfont = CreateFontIndirect(&logfont);

            SendMessage(ctl, WM_SETFONT, (WPARAM)hfont, 0);

            char buf[512];
            snprintf(buf, 512, "Built %s %s", __DATE__, __TIME__);
            SetDlgItemText(hDlg, IDC_BUILDDATE, buf);

            return (INT_PTR)TRUE;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }

            break;
        }
    }
    return (INT_PTR)FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    App app(hInstance);

    app.init();
    app.loop();
    app.shutdown();

    return 0;
}