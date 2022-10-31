// Plik nag³ówkowy
#include <d2d1_3.h>
#include <windows.h>

// Deklaracje u¿ycia pomocniczych funkcji
using D2D1::RenderTargetProperties;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;

// Interfejsy potrzebne do zainicjowania Direct2D
ID2D1Factory7* d2d_factory = nullptr;
ID2D1HwndRenderTarget* d2d_render_target = nullptr;

// Utworzenie fabryki Direct2D
HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
    &d2d_factory);

RECT rc;
GetClientRect(hwnd, &rc);

// Utworzenie celu renderowania w oknie Windows
d2d_factory->CreateHwndRenderTarget(
    RenderTargetProperties(),
    HwndRenderTargetProperties(hwnd,
        SizeU(static_cast<UINT32>(rc.right) -
            static_cast<UINT32>(rc.left),
            static_cast<UINT32>(rc.bottom) -
            static_cast<UINT32>(rc.top))),
    &d2d_render_target);

// gdzie rc jest typu RECT i zawiera aktualne rozmiary obszaru
// roboczego okna uzyskane funkcjê GetClientRect.
// ...
// Bezpieczne zwolnienie zasobów

if (d2d_render_target) d2d_render_target->Release();
if (d2d_factory) d2d_factory->Release();