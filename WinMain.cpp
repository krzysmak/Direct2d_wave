#include <windows.h>
#include <d2d1_3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <windowsx.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using D2D1::RenderTargetProperties;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;

using D2D1::ColorF;
using D2D1::LinearGradientBrushProperties;
using D2D1::Point2F;
using D2D1::RadialGradientBrushProperties;
using D2D1::BezierSegment;
using D2D1::QuadraticBezierSegment;
using D2D1::Matrix3x2F;
using D2D1::Ellipse;
using std::sin;


FLOAT angle = 0;

ID2D1Factory7* d2d_factory = nullptr;
ID2D1HwndRenderTarget* d2d_render_target = nullptr;

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

RECT rc;

FLOAT lEyeX = 0;
FLOAT lEyeY = 0;
FLOAT rEyeX = 0;
FLOAT rEyeY = 0;

FLOAT fTime = 0;

FLOAT currentScale = 0;
FLOAT firstWidth = 0;
FLOAT firstHeight = 0;
FLOAT width = 0;
FLOAT height = 0;

BOOL mouseMoved = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    switch (uMsg) {
        case WM_CREATE:
            GetClientRect(hwnd, &rc);

            if (firstWidth == 0)
                firstWidth = rc.right - rc.left;

            if (firstHeight == 0)
                firstHeight = rc.bottom - rc.top;

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
            return 0;
        case WM_DESTROY:
            if (d2d_render_target) d2d_render_target->Release();
            if (d2d_factory) d2d_factory->Release();
            PostQuitMessage(0);
            return 0;
        case WM_MOUSEMOVE:
        {
            mouseMoved = true;
            FLOAT newX = LOWORD(lParam);
            FLOAT newY = HIWORD(lParam);
            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
            FLOAT midHeight = height / 2;
            FLOAT midWidth = width / 2;
            FLOAT ldistance = sqrt(pow(newX - (midWidth - 90), 2) + pow(newY - (midHeight - 60), 2));
            FLOAT ldiv = ldistance / 47; // 47 = maksymalna odleg³oœæ œrodka Ÿrenicy od œrodka oka
            FLOAT rdistance = sqrt(pow(newX - (midWidth + 90), 2) + pow(newY - (midHeight - 60), 2));
            FLOAT rdiv = rdistance / 47; // 47 = maksymalna odleg³oœæ œrodka Ÿrenicy od œrodka oka
            // ldiv > 1 oznacza, ¿e Ÿrenica wychodzi poza oko i nale¿y przeskalowaæ jej wspó³rzêdne wzglêdem œrodka oka
            lEyeX = ldiv > 1 ? ((newX - midWidth + 90) / ldiv) + midWidth - 90 : newX;
            lEyeY = ldiv > 1 ? ((newY - midHeight + 60) / ldiv) + midHeight - 60 : newY;
            rEyeX = rdiv > 1 ? ((newX - midWidth - 90) / rdiv) + midWidth + 90 : newX;
            rEyeY = rdiv > 1 ? ((newY - midHeight + 60) / rdiv) + midHeight - 60 : newY;
            // chcemy aby wspó³rzêdne Ÿrenicy by³y zale¿ne od wielkoœci ekranu, tak aby przenosi³y siê razem ze zmian¹ okna
            lEyeX -= midWidth;
            rEyeX -= midWidth;
            lEyeY -= midHeight;
            rEyeY -= midHeight;
            // dlatego teraz odejmujemy wymiary, przy rysowaniu je dodamy
            return 0;
        }
        case WM_PAINT:
            GetClientRect(hwnd, &rc);
            ID2D1SolidColorBrush* brush1 = nullptr;
            ID2D1SolidColorBrush* brush2 = nullptr;
            // - Interfejsy do obs³ugi œcie¿ki
            ID2D1PathGeometry* path = nullptr;
            ID2D1GeometrySink* path_sink = nullptr;
            ID2D1PathGeometry* path2 = nullptr;
            ID2D1GeometrySink* path_sink2 = nullptr;
            ID2D1PathGeometry* path3 = nullptr;
            ID2D1GeometrySink* path_sink3 = nullptr;
            ID2D1PathGeometry* path4 = nullptr;
            ID2D1GeometrySink* path_sink4 = nullptr;
            ID2D1TransformedGeometry* transformed_geometry = nullptr;

            // Sta³e kolorów
            D2D1_COLOR_F const clear_color =
            { .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };
            D2D1_COLOR_F const brush1_color =
            { .r = 1.0f, .g = 0.5f, .b = 0.5f, .a = 1.0f };
            D2D1_COLOR_F const brush2_color =
            { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };

            // Sta³e ustawienia geometrii
            FLOAT const brush2_width = 3.0f;

            // Zmienne przydatne do rysowania, oraz wspó³rzêdne Ÿrenic, jeŸeli == 0 to trzeba ustawiæ na pocz¹tkowe wartoœci

            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
            FLOAT midHeight = height / 2;
            FLOAT midWidth = width / 2;
            lEyeX = lEyeX == 0 ? midWidth - 90 : lEyeX;
            lEyeY = lEyeY == 0 ? midHeight - 60 : lEyeY;
            rEyeX = rEyeX == 0 ? midWidth + 90 : rEyeX;
            rEyeY = rEyeY == 0 ? midHeight - 60 : rEyeY;

            // Utworzenie pêdzli

            d2d_render_target->CreateSolidColorBrush(brush1_color, &brush1);
            d2d_render_target->CreateSolidColorBrush(brush2_color, &brush2);
            // Utworzenie i zbudowanie geometrii œcie¿ki
            d2d_factory->CreatePathGeometry(&path);
            path->Open(&path_sink);
            path_sink->BeginFigure(Point2F(midWidth - 130, midHeight - 100), D2D1_FIGURE_BEGIN_FILLED);
            path_sink->AddBezier(BezierSegment(
                Point2F(midWidth - 300, midHeight + 205), Point2F(midWidth - 130, midHeight + 215), Point2F(midWidth, midHeight + 220)));
            path_sink->AddBezier(BezierSegment(
                Point2F(midWidth + 130, midHeight + 215), Point2F(midWidth + 300, midHeight + 205), Point2F(midWidth + 130, midHeight - 100)));
            path_sink->AddBezier(BezierSegment(
                Point2F(midWidth + 200, midHeight - 180), Point2F(midWidth + 150, midHeight - 250), Point2F(midWidth + 70, midHeight - 200)));
            path_sink->AddBezier(BezierSegment(
                Point2F(midWidth + 20, midHeight - 230), Point2F(midWidth - 20, midHeight - 230), Point2F(midWidth - 70, midHeight - 200)));
            path_sink->AddBezier(BezierSegment(
                Point2F(midWidth - 150, midHeight - 250), Point2F(midWidth - 200, midHeight - 180), Point2F(midWidth - 130, midHeight - 100)));

            path_sink->EndFigure(D2D1_FIGURE_END_OPEN);
            path_sink->Close();



            d2d_factory->CreatePathGeometry(&path2);
            path2->Open(&path_sink2);
            path_sink2->BeginFigure(Point2F(midWidth - 50, midHeight + 50), D2D1_FIGURE_BEGIN_FILLED);
            path_sink2->AddBezier(BezierSegment(
                Point2F(midWidth - 100, midHeight - 25), Point2F(midWidth + 100, midHeight - 25), Point2F(midWidth + 50, midHeight + 50)));
            path_sink2->AddQuadraticBezier(QuadraticBezierSegment(
                Point2F(midWidth, midHeight + 125), Point2F(midWidth - 50, midHeight + 50)));
            path_sink2->EndFigure(D2D1_FIGURE_END_OPEN);
            path_sink2->Close();

            d2d_factory->CreatePathGeometry(&path3);
            path3->Open(&path_sink3);
            path_sink3->BeginFigure(Point2F(midWidth - 100, midHeight + 150), D2D1_FIGURE_BEGIN_FILLED);
            path_sink3->AddQuadraticBezier(QuadraticBezierSegment(
                Point2F(midWidth, midHeight + 100), Point2F(midWidth + 100, midHeight + 150)));
            path_sink3->EndFigure(D2D1_FIGURE_END_OPEN);
            path_sink3->Close();

            d2d_factory->CreatePathGeometry(&path4);
            path4->Open(&path_sink4);
            path_sink4->BeginFigure(Point2F(midWidth - 100, midHeight + 150), D2D1_FIGURE_BEGIN_FILLED);
            path_sink4->AddQuadraticBezier(QuadraticBezierSegment(
                Point2F(midWidth, midHeight + 200), Point2F(midWidth + 100, midHeight + 150)));
            path_sink4->EndFigure(D2D1_FIGURE_END_OPEN);
            path_sink4->Close();

            ID2D1RadialGradientBrush* rad_brush = nullptr;
            ID2D1RadialGradientBrush* rad_brush2 = nullptr;
            ID2D1RadialGradientBrush* rad_brush3 = nullptr;
            ID2D1GradientStopCollection* rad_stops = nullptr;
            ID2D1GradientStopCollection* rad_stops2 = nullptr;
            UINT const NUM_RAD_STOPS = 2;
            UINT const NUM_RAD_STOPS2 = 3;
            D2D1_GRADIENT_STOP rad_stops_data[NUM_RAD_STOPS];
            D2D1_GRADIENT_STOP rad_stops_data2[NUM_RAD_STOPS2];
            D2D1_POINT_2F const ellipse_center = { .x = midWidth - 90, .y = midHeight - 60 };
            D2D1_POINT_2F const ellipse_center2 = { .x = midWidth + 90, .y = midHeight - 60 };
            D2D1_POINT_2F const ellipse_radii = { .x = 70, .y = 70 };
            FLOAT const brush_width = 2.0f;
            ID2D1SolidColorBrush* brush = nullptr;
            D2D1_COLOR_F const brush_color =
            { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
            d2d_render_target->CreateSolidColorBrush(brush_color, &brush);

            FLOAT widthModificator = mouseMoved ? midWidth : 0;
            FLOAT heightModificator = mouseMoved ? midHeight : 0;

            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                D2D1::Point2F(lEyeX + widthModificator, lEyeY + heightModificator),
                23.f,
                23.f
            );

            D2D1_ELLIPSE ellipse2 = D2D1::Ellipse(
                D2D1::Point2F(rEyeX + widthModificator, rEyeY + heightModificator),
                23.f,
                23.f
            );

            rad_stops_data[0] =
            { .position = 0.70f, .color = ColorF(1.0f, 1.0f, 1.0f, 1.0f) };
            rad_stops_data[1] =
            { .position = 1.0f, .color = ColorF(0.75f, 0.75f, 0.75f, 1.0f) };

            rad_stops_data2[0] =
            { .position = 0.2f, .color = ColorF(1.0f, 1.0f, 1.0f, 1.0f) };
            rad_stops_data2[1] =
            { .position = 0.6f, .color = ColorF(0.0f, 1.0f, 0.0f, 1.0f) };
            rad_stops_data2[2] =
            { .position = 1.0f, .color = ColorF(0.0f, 0.6f, 0.0f, 1.0f) };

            // Utworzenie gradientu promienistego
            d2d_render_target->CreateGradientStopCollection(
                rad_stops_data, NUM_RAD_STOPS, &rad_stops);
            if (rad_stops) {
                d2d_render_target->CreateRadialGradientBrush(
                    RadialGradientBrushProperties(ellipse_center,
                        Point2F(0, 0), 70, 70),
                    rad_stops, &rad_brush);
                d2d_render_target->CreateRadialGradientBrush(
                    RadialGradientBrushProperties(ellipse_center2,
                        Point2F(0, 0), 70, 70),
                    rad_stops, &rad_brush2);
            }

            d2d_render_target->CreateGradientStopCollection(
                rad_stops_data2, NUM_RAD_STOPS2, &rad_stops2);
            if (rad_stops2) {
                d2d_render_target->CreateRadialGradientBrush(
                    RadialGradientBrushProperties(Point2F(midWidth, midHeight + 30),
                        Point2F(0, 0), 200, 200),
                    rad_stops2, &rad_brush3);
            }
            

            d2d_render_target->BeginDraw();

            d2d_render_target->Clear(clear_color);

            // Skalowanie takie, ¿e zwierz¹tko ma ten sam rozmiar co na pocz¹tku, nie skaluje siê wraz z oknem

            Matrix3x2F scale = Matrix3x2F::Scale(firstWidth / width, firstHeight / height, Point2F(0, 0));
            d2d_render_target->SetTransform(scale);

            d2d_render_target->FillGeometry(path, rad_brush3);
            d2d_render_target->DrawGeometry(path, brush2, brush2_width);

            d2d_render_target->FillEllipse(
                Ellipse(ellipse_center, ellipse_radii.x, ellipse_radii.y),
                rad_brush);
            d2d_render_target->DrawEllipse(
                Ellipse(ellipse_center, ellipse_radii.x, ellipse_radii.y),
                brush, brush_width);

            d2d_render_target->FillEllipse(ellipse, brush);
            d2d_render_target->DrawEllipse(ellipse, brush, brush_width);

            d2d_render_target->FillEllipse(
                Ellipse(ellipse_center2, ellipse_radii.x, ellipse_radii.y),
                rad_brush2);
            d2d_render_target->DrawEllipse(
                Ellipse(ellipse_center2, ellipse_radii.x, ellipse_radii.y),
                brush, brush_width);

            d2d_render_target->FillEllipse(ellipse2, brush);
            d2d_render_target->DrawEllipse(ellipse2, brush, brush_width);

            angle = 5.0f * sin(fTime);
            fTime += 0.05f;
            
            Matrix3x2F transformation;
            d2d_render_target->GetTransform(&transformation);

            Matrix3x2F rotate = Matrix3x2F::Rotation(angle, Point2F(midWidth, midHeight));
            rotate.SetProduct(rotate, transformation);

            d2d_render_target->SetTransform(rotate);


            d2d_render_target->FillGeometry(path2, brush1);
            d2d_render_target->DrawGeometry(path2, brush2, brush2_width);

            //d2d_render_target->FillGeometry(path3, brush1);
            if (GetAsyncKeyState(VK_LBUTTON) < 0) {
                d2d_render_target->DrawGeometry(path4, brush2, brush2_width);
            }
            else {
                d2d_render_target->DrawGeometry(path3, brush2, brush2_width);
            }
            
            d2d_render_target->SetTransform(transformation);

            d2d_render_target->EndDraw();
            InvalidateRect(hwnd, &rc, 0);
            // Zwolnienie zasobów
            brush->Release();
            brush1->Release();
            brush2->Release();
            rad_brush->Release();
            rad_brush2->Release();
            rad_brush3->Release();
            rad_stops->Release();
            rad_stops2->Release();
            path->Release();
            path_sink->Release();
            path2->Release();
            path_sink2->Release();
            path3->Release();
            path_sink3->Release();
            path4->Release();
            path_sink4->Release();
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
