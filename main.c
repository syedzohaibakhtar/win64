// main.c  — Win32 GUI (ANSI, works with classic MinGW/mingw32)

#include <windows.h>
#include <stdio.h>

// ----- Control IDs -----
#define IDC_EDIT_EXPR   1001
#define IDC_BTN_CALC    1002
#define IDC_STATIC_OUT  1003

// ----- Globals for child controls -----
static HWND g_hEdit = NULL;
static HWND g_hBtn  = NULL;
static HWND g_hOut  = NULL;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static void SetDefaultFont(HWND h)
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
}

static void LayoutControls(HWND hwnd)
{
    RECT rc; GetClientRect(hwnd, &rc);
    const int margin = 10;
    const int btnW = 100, btnH = 28;
    const int editH = 28;
    const int outH  = 24;

    int editW = (rc.right - rc.left) - (margin*3) - btnW;
    if (editW < 80) editW = 80;

    MoveWindow(g_hEdit, margin, margin, editW, editH, TRUE);
    MoveWindow(g_hBtn,  margin + editW + margin, margin, btnW, btnH, TRUE);
    MoveWindow(g_hOut,  margin, margin + editH + margin,
               (rc.right - rc.left) - margin*2, outH, TRUE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow)
{
    (void)hPrev; (void)pCmdLine;


    // Register window class (ANSI)
    const char CLASS_NAME[] = "CalcWindowClassA";

    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm       = wc.hIcon;

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "RegisterClassExA failed.", "Error", MB_ICONERROR);
        return 1;
    }

    // Create main window (ANSI)
    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Calculator (+ / -)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 460, 160,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxA(NULL, "CreateWindowExA failed.", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_hEdit = CreateWindowExA(
            0, "EDIT", "3 + 4",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            0,0,0,0, hwnd, (HMENU)IDC_EDIT_EXPR, GetModuleHandle(NULL), NULL);

        g_hBtn = CreateWindowExA(
            0, "BUTTON", "&Calculate",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            0,0,0,0, hwnd, (HMENU)IDC_BTN_CALC, GetModuleHandle(NULL), NULL);

        g_hOut = CreateWindowExA(
            0, "STATIC", "= result will appear here",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0,0,0,0, hwnd, (HMENU)IDC_STATIC_OUT, GetModuleHandle(NULL), NULL);

        if (!g_hEdit || !g_hBtn || !g_hOut) {
            MessageBoxA(hwnd, "Failed to create controls.", "Error", MB_ICONERROR);
            return -1;
        }

        SetDefaultFont(g_hEdit);
        SetDefaultFont(g_hBtn);
        SetDefaultFont(g_hOut);

        LayoutControls(hwnd);
        return 0;
    }

    case WM_SIZE:
        LayoutControls(hwnd);
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_BTN_CALC)
        {
            char buf[256];
            GetWindowTextA(g_hEdit, buf, (int)sizeof(buf));

            double a = 0.0, b = 0.0;
            char op = '\0';

            // Accepts: "3 + 4", "3+4", "  3   -   4  "
            if (sscanf(buf, " %lf %c %lf", &a, &op, &b) == 3) {
                double result = 0.0;
                if (op == '+') {
                    result = a + b;
                } else if (op == '-') {
                    result = a - b;
                } else {
                    SetWindowTextA(g_hOut, "Only '+' and '-' are supported.");
                    return 0;
                }

                char out[128];
                // Use snprintf if available; _snprintf is fine on old msvcrt
                int n = _snprintf(out, sizeof(out), "= %.12g", result);
                if (n < 0 || n >= (int)sizeof(out)) out[sizeof(out)-1] = '\0';
                SetWindowTextA(g_hOut, out);
            } else {
                SetWindowTextA(g_hOut, "Invalid input. Try: 3 + 4  (or 3-4)");
            }
            return 0;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
