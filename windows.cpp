#include <windows.h>
#include <stdint.h>
#include "dx11_basic.h"
#include "app.h"
#include "clock.h"

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;
  switch (message) {
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
bool initWindow(HINSTANCE hInstance, int nCmdShow, HWND* hWnd) {

  // Register class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = nullptr;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = "DX11WindowClass";
  wcex.hIconSm = nullptr;
  if (!RegisterClassEx(&wcex))
    return false;

  // Create window
  RECT rc = { 0, 0, 1200, 720 };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  *hWnd = CreateWindow("DX11WindowClass", "Direct3D 11 Video Decoder Sample",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
    nullptr);
  if (!*hWnd)
    return false;

  ShowWindow(*hWnd, nCmdShow);

  return true;
}

// --------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  HWND hWnd;
  if (!initWindow(hInstance, nCmdShow, &hWnd))
    return 0;
  
  CApp app;
  if (!app.create(hWnd))
    return -1;

  Clock clock;

  // Main message loop
  MSG msg = { 0 };
  while (WM_QUIT != msg.message) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      float elapsed = clock.elapsed();
      app.update(elapsed);
      app.render();
    }
  }

  app.destroy();

  return (int)msg.wParam;
}
