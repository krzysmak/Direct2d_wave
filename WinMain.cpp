#include <windows.h>
#include <d2d1_3.h>
#include <iostream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using D2D1::RenderTargetProperties;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;

using D2D1::Point2F;
using D2D1::RectF;
using D2D1::RoundedRect;
using D2D1::Ellipse;

const INT spaceSize = 51;
const INT scale = 200;

const FLOAT Alfa = 0.005;
const FLOAT sinusAlfa = sin(Alfa);
const FLOAT cosinusAlfa = cos(Alfa);

ID2D1Factory7* d2d_factory = nullptr;
ID2D1HwndRenderTarget* d2d_render_target = nullptr;

struct Point {
    FLOAT x;
    FLOAT y;
    FLOAT z;

    Point(FLOAT x, FLOAT y) {
        this->x = x;
        this->y = y;
        this->z = cos(10 * sqrt(pow(x, 2) + pow(y, 2))) / 4;
    }

    Point() = default;

    void rotateYZ(FLOAT sinus, FLOAT cosinus) {
        FLOAT newZ = (cosinus * z) - (sinus * y);
        FLOAT newY = (sinus * z) + (cosinus * y);
        this->z = newZ;
        this->y = newY;
    }

    void rotateXY(FLOAT sinus, FLOAT cosinus) {
        FLOAT newX = (cosinus * x) - (sinus * y);
        FLOAT newY = (sinus * x) + (cosinus * y);
        this->x = newX;
        this->y = newY;
    }
};

struct Space {
    Point points[spaceSize][spaceSize];

    void rotateSpace(FLOAT sinus, FLOAT cosinus) {
        for (int x = 0; x < spaceSize; x++) {
            for (int y = 0; y < spaceSize; y++) {
                points[x][y].rotateYZ(sinus, cosinus);
            }
        }
    }

    void fillSpace() {
        for (int x = -25; x <= 25; x++) {
            for (int y = -25; y <= 25; y++) {
                points[x + 25][y + 25] = Point((FLOAT)x / 25.0f, (FLOAT)y / 25.0f);
                points[x + 25][y + 25].rotateXY(sqrt(2) / 2, sqrt(2) / 2);
            }
        }
    }

    void paintSpace(ID2D1Brush* brush, FLOAT xModifier, FLOAT yModifier) {
        for (int x = 1; x < spaceSize; x++) {
            for (int y = 0; y < spaceSize; y++) {
                d2d_render_target->DrawLine(
                    Point2F(scale * (this->points[x - 1][y].x) + xModifier, scale * (this->points[x - 1][y].y) + yModifier), 
                    Point2F(scale * (this->points[x][y].x) + xModifier, scale * (this->points[x][y].y) + yModifier),
                    brush, 1.0f);
            }
        }
        for (int x = 0; x < spaceSize; x++) {
            for (int y = 1; y < spaceSize; y++) {
                d2d_render_target->DrawLine(
                    Point2F(scale * (this->points[x][y - 1].x) + xModifier, scale * (this->points[x][y - 1].y) + yModifier),
                    Point2F(scale * (this->points[x][y].x) + xModifier, scale * (this->points[x][y].y) + yModifier),
                    brush, 1.0f);
            }
        }
    }
};

INT WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ PWSTR cmd_line, _In_ INT cmd_show) {
            

    // Register the window class.
    const wchar_t CLASS_NAME[] = TEXT("Sample Window Class");

    WNDCLASSEX wc = { };

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_GLOBALCLASS;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = nullptr;


    RegisterClassEx(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        TEXT("Learn to Program Windows"),    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        instance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, cmd_show);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

Space space;
RECT rc;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    switch (uMsg) {
        case WM_CREATE:
            GetClientRect(hwnd, &rc);

            if (!d2d_factory) {
                HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                    &d2d_factory);
            }

            if (!d2d_render_target) {
                d2d_factory->CreateHwndRenderTarget(
                    RenderTargetProperties(),
                    HwndRenderTargetProperties(hwnd,
                        SizeU(static_cast<UINT32>(rc.right) -
                            static_cast<UINT32>(rc.left),
                            static_cast<UINT32>(rc.bottom) -
                            static_cast<UINT32>(rc.top))),
                    &d2d_render_target);
            }

            space.fillSpace();
            return 0;
        case WM_DESTROY:
            if (d2d_render_target) d2d_render_target->Release();
            if (d2d_factory) d2d_factory->Release();
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            // Interfejsy potrzebne do rysowania
            ID2D1SolidColorBrush* brush = nullptr;

            // Sta³e z kolorami
            D2D1_COLOR_F const clear_color =
            { .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };
            D2D1_COLOR_F const brush_color_1 =
            { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
       
            // Utworzenie pêdzla
            d2d_render_target->CreateSolidColorBrush(brush_color_1, &brush);

            // Rysowanie
            d2d_render_target->BeginDraw();
            d2d_render_target->Clear(clear_color);

            space.paintSpace(brush, (FLOAT)(rc.right - rc.left) / 2.0f, (FLOAT)(rc.bottom - rc.top) / 2.0f);

            space.rotateSpace(sinusAlfa, cosinusAlfa);

            d2d_render_target->EndDraw();

            // Zwolnienie zasobów
            if (brush) brush->Release();
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
