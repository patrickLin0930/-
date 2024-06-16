#include <windows.h>
#include <string>
#include <map>
#include <iostream>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <gdiplus.h>
#include <codecvt>  // For std::wstring_convert
#include <locale>   // For std::wstring_convert

using namespace std;
using namespace Gdiplus;

// 轉換 UTF-8 字串為 wstring
std::wstring utf8_to_wstring(const std::string& utf8str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8str);
}

// 全域變數
HINSTANCE hInst;
HWND hwndMain;
HBITMAP hbmBackground;
map<string, RECT> keyPositions;

// 函數宣告
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LoadBackgroundImage(HWND hWnd);
void LoadKeyPositions();
void CreateKeyboardButtons(HWND hWnd);

// 主程序入口點
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    OutputDebugString(L"進入 WinMain\n");
    hInst = hInstance;

    // 初始化 GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 載入背景圖片
    OutputDebugString(L"開始載入背景圖片\n");
    LoadBackgroundImage(NULL); // 將 NULL 傳遞給 LoadBackgroundImage，因為 hwndMain 尚未創建

    // 介面類別
    OutputDebugString(L"註冊視窗類別\n");
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"KeyboardMouseApp";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"註冊視窗類別失敗!", L"錯誤", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 建立主視窗
    OutputDebugString(L"建立主視窗\n");
    hwndMain = CreateWindow(
        L"KeyboardMouseApp",
        L"鍵盤與滑鼠視覺化器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwndMain) {
        MessageBox(NULL, L"建立視窗失敗!", L"錯誤", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 載入鍵盤按鈕位置
    OutputDebugString(L"載入鍵盤按鈕位置\n");
    LoadKeyPositions();

    // 建立鍵盤按鈕
    OutputDebugString(L"建立鍵盤按鈕\n");
    CreateKeyboardButtons(hwndMain);

    // 顯示並更新視窗
    OutputDebugString(L"顯示並更新視窗\n");
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    // 訊息迴圈
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 關閉 GDI+
    OutputDebugString(L"關閉 GDI+\n");
    GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}

// 圖形化介面
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT:
    {
        OutputDebugString(L"處理 WM_PAINT\n");
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 在 WM_PAINT 消息處理中跑背景圖片
        if (hbmBackground != NULL) {
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmBackground);

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            StretchBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, 1200, 500, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteDC(hdcMem);
        }

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        OutputDebugString(L"收到 WM_DESTROY\n");
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 載入背景圖片
void LoadBackgroundImage(HWND hWnd) {

    WCHAR szFileName[MAX_PATH] = L"C:\\Users\\bigdi\\source\\repos\\sec\\background.jpg";
    OutputDebugString(L"載入背景圖片函式\n");

    // 使用 GDI+ 載入圖片(格式?沒顯示?)
    Bitmap* bitmap = new Bitmap(szFileName);
    if (bitmap->GetLastStatus() == Gdiplus::Ok) {
        hbmBackground = (HBITMAP)bitmap->GetHBITMAP(NULL, NULL);

        // 如果有更新，要求視窗更新
        if (hWnd != NULL) {
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            OutputDebugString(L"視窗重新load!!\n");
        }

        OutputDebugString(L"圖片載入成功\n");
    }
    else {
        MessageBox(hWnd, L"無法載入背景圖片!", L"錯誤", MB_OK | MB_ICONERROR);

        OutputDebugString(L"圖片載入失敗\n");
    }

    delete bitmap;
}

// 載入鍵盤按鈕位置
void LoadKeyPositions() {
    // 每個按鈕在介面上的位置

    keyPositions["A"] = RECT{ 100, 100, 150, 150 };
    keyPositions["B"] = RECT{ 200, 100, 250, 150 };
    
}

// 建立鍵盤按鈕
void CreateKeyboardButtons(HWND hWnd) {
    for (auto& pair : keyPositions) {
        string key = pair.first;
        RECT& rc = pair.second;

        CreateWindow(
            L"BUTTON",
            utf8_to_wstring(key).c_str(),  // 將按鈕名稱從 std::string 轉換為 std::wstring
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top,
            hWnd,
            NULL,
            hInst,
            NULL
        );

        // 輸出按鈕建立訊息
        wstring buttonName = utf8_to_wstring(key);
        wstring outputMsg = L"建立按鈕: " + buttonName + L"\n";
        OutputDebugString(outputMsg.c_str());
    }
}
