// build: g++ AutoKye.cpp -o AutoKye.exe -static -mwindows

#include <windows.h>

volatile bool programRunning = true;
WORD inputKey = VK_SPACE;

HWND hEdit, hStatus;

// 🔥 키 입력 (ScanCode + 확장키 대응)
void PressKey(WORD vk) {
    INPUT input = {0};

    WORD scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);

    DWORD flags = KEYEVENTF_SCANCODE;

    // 확장 키 처리 (게임 중요)
    switch (vk) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_CONTROL:
        case VK_MENU: // ALT
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR: // PageUp
        case VK_NEXT:  // PageDown
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
    }

    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scan;
    input.ki.dwFlags = flags;

    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = flags | KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// 🔥 자동 입력 스레드
DWORD WINAPI AutoThread(LPVOID lpParam) {
    while (programRunning) {

        // 키 눌림 감지
        if (GetAsyncKeyState(inputKey) & 0x8000) {
            PressKey(inputKey);
        }

        Sleep(1); // 안정성 유지
    }
    return 0;
}

// 🔥 상태 표시
void UpdateStatus() {
    char text[64];
    wsprintf(text, "VK Code: %d", inputKey);
    SetWindowText(hStatus, text);
}

// 🔥 키 갱신 (안정 처리)
void UpdateKey() {
    char buf[32] = {0};
    GetWindowText(hEdit, buf, 31);

    if (buf[0] != 0) {
        SHORT vk = VkKeyScan(buf[0]);
        if (vk != -1) {
            inputKey = vk & 0xFF;
        }
    }
}

// 🔥 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_CREATE:
        CreateWindow("STATIC", "Input Key:",
            WS_VISIBLE | WS_CHILD,
            20, 20, 80, 20,
            hwnd, NULL, NULL, NULL);

        hEdit = CreateWindow("EDIT", "A",
            WS_VISIBLE | WS_CHILD | WS_BORDER,
            100, 20, 100, 20,
            hwnd, NULL, NULL, NULL);

        CreateWindow("BUTTON", "Apply",
            WS_VISIBLE | WS_CHILD,
            20, 60, 180, 30,
            hwnd, (HMENU)1, NULL, NULL);

        hStatus = CreateWindow("STATIC", "VK Code: 65",
            WS_VISIBLE | WS_CHILD,
            20, 100, 180, 20,
            hwnd, NULL, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            UpdateKey();
            UpdateStatus();
        }
        break;

    case WM_DESTROY:
        programRunning = false;
        Sleep(10); // 스레드 종료 여유
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 🔥 GUI 진입점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "AutoKey";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "AutoKey",
        "Hold Key Auto Input",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 180,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    CreateThread(NULL, 0, AutoThread, NULL, 0, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}