//include the base windows header file
#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <d2d1.h>

ID2D1Factory *pFactory = nullptr;
ID2D1HwndRenderTarget *pRenderTarget = nullptr;
ID2D1SolidColorBrush *pLightSlateGrayBrush = nullptr;
ID2D1SolidColorBrush *pCornflowerBlueBrush = nullptr;

template<class T>
inline void SafeRelease(T **ppInterfaceToRelease){
    if(*ppInterfaceToRelease != nullptr){
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = nullptr;
    }
}

HRESULT CreateGraphicsResources(HWND hWnd){
    HRESULT hr = S_OK;
    if(pRenderTarget == nullptr){
        RECT rc;
        GetClientRect(hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &pRenderTarget);

        if(SUCCEEDED(hr)){
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray), &pLightSlateGrayBrush);
        }

        if(SUCCEEDED(hr)){
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &pCornflowerBlueBrush);
        }
    }

    return hr;
}

void DiscardGraphisResources(){
    SafeRelease(&pRenderTarget);
    SafeRelease(&pLightSlateGrayBrush);
    SafeRelease(&pCornflowerBlueBrush);
}

//the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//the entry point for any Windows program
int  WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow){
    //the handle for the window, filled by a function
    HWND hWnd;
    //this struct holds information for window class
    WNDCLASSEX wc;

    //initialize COM
    if(FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) return -1;

    //clear out the window class for use
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    //fill in the struct with the needed information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = _T("WindowClass");

    //register the window class
    RegisterClassEx(&wc);

    //create the window and use the result as the handle
    hWnd = CreateWindowEx(0, _T("WindowClass"), _T("GameEngine D2D"), WS_OVERLAPPEDWINDOW, 300, 300, 480, 320, NULL, NULL, hInstance, NULL);

    //display the window on the screen
    ShowWindow(hWnd, nCmdShow);

    //enter the main loop:

    //this struct holds windows event message
    MSG msg;

    //wait for the next message in the queue, store the result in 'msg'
    while(GetMessage(&msg, NULL, 0, 0)){

        //translate keystroke messages into the right format
        TranslateMessage(&msg);

        //send the message to the WindowProc function
        DispatchMessage(&msg);
    }

    // uninitialize COM
    CoUninitialize();

    //return this part of the WM_QUIT message to Windows
    return msg.wParam;
}

//this is the main message handles for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    bool wasHandled = false;

    //sort through and find what code to run for the message given
    switch(message){
        case WM_CREATE:{
            if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))){
                result = -1;    // Fail CreateWindowEx.
                return result;
            }
            wasHandled = true;
            result = 0;
        } break;
        case WM_PAINT:{
            HRESULT hr = CreateGraphicsResources(hWnd);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rec = {10 ,10, 100, 100};
            HBRUSH brush = (HBRUSH) GetStockObject(BLACK_BRUSH);

            FillRect(hdc, &rec, brush);

            EndPaint(hWnd, &ps);
        } break;
        case WM_DESTROY:{
            //close the application entirely
            PostQuitMessage(0);
            return 0;
        } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}