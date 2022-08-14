#include <Windows.h>
#include <commdlg.h>
#include <Wingdi.h>
#include <Winerror.h>
#include <stdio.h>
#include "resource.h"
#include "main.h"
#include "logger.h"
#include "./nes/loader.h"
#include "./nes/cpu.h"
#include "./nes/ppu.h"
#include "./nes/controller.h"

HWND window;
NES_BITMAP backBuffer;
PERFDATA perfData;
BOOL running;

// If null is passed as the hdc, a device context is created and released by the renderer
// Any hdc passed to the function, will not be released
void RenderFrame(HDC hdc)
{
    HDC locHdc;
    if (hdc == NULL)
    {
        locHdc = GetDC(window);
    }
    else
    {
        locHdc = hdc;
    }

    RECT windowRect;
    GetClientRect(window, &windowRect);

    LONG windowWidth = windowRect.right - windowRect.left;
    LONG windowHeight = windowRect.bottom - windowRect.top;

    // This assumes that integer division truncates towards zero
    uint8_t maxWscale = windowWidth / NES_PX_WIDTH;
    uint8_t maxHscale = windowHeight / NES_PX_HEIGHT;

    // Select the smallest of the scales
    perfData.CurrentScaleFactor = maxWscale;
    if (maxWscale > maxHscale)
    {
        perfData.CurrentScaleFactor = maxHscale;
    }

    StretchDIBits(locHdc,
                  (windowWidth - NES_PX_WIDTH * perfData.CurrentScaleFactor) / 2,
                  (windowHeight - NES_PX_HEIGHT * perfData.CurrentScaleFactor) / 2,
                  NES_PX_WIDTH * perfData.CurrentScaleFactor,
                  NES_PX_HEIGHT * perfData.CurrentScaleFactor,
                  0,
                  0,
                  NES_PX_WIDTH,
                  NES_PX_HEIGHT,
                  backBuffer.Memory,
                  &backBuffer.BitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    //Display debug info

    if (perfData.DisplayDebugInfo)
    {
        RECT drawArea;
        drawArea.left = (windowWidth - NES_PX_WIDTH * perfData.CurrentScaleFactor) / 2;
        drawArea.right = (windowWidth - NES_PX_WIDTH * perfData.CurrentScaleFactor) / 2 + NES_PX_WIDTH * perfData.CurrentScaleFactor;
        drawArea.top = (windowHeight - NES_PX_HEIGHT * perfData.CurrentScaleFactor) / 2;
        drawArea.bottom = (windowHeight - NES_PX_HEIGHT * perfData.CurrentScaleFactor) / 2 + NES_PX_HEIGHT * perfData.CurrentScaleFactor;

        char strbuf[1024];
        sprintf(strbuf, "W: %4d H: %4d\nAvg. cooked FPS: %.2f\nAvg. raw FPS: %.2f\nPC %.4x\tOP: %s\nCYC: %d, PPU_CYC: %d",
                windowWidth, windowHeight, perfData.CookedFPSAverage, perfData.RawFPSAverage, cpu.registers.pc, opcode_to_string[cpu.current_instruction.operation], cpu.cycle, ppu_state.cycle);

        if (cpu.current_instruction.bytes > 1)
        {
            char strbuf2[100];
            sprintf(strbuf2, " %.2x", cpu_memory[cpu.registers.pc + 1]);
            strcat(strbuf, strbuf2);
        }

        if (cpu.current_instruction.bytes > 2)
        {
            char strbuf2[100];
            sprintf(strbuf2, " %.2x", cpu_memory[cpu.registers.pc + 2]);
            strcat(strbuf, strbuf2);
        }

        DrawTextA(locHdc, strbuf, strlen(strbuf), &drawArea, TA_TOP);
    }

    if (hdc == NULL)
    {
        ReleaseDC(window, locHdc);
    }
}

void Blit32BppBitmapToBuffer(NES_BITMAP bitmap, uint16_t x, uint16_t y, uint16_t brightness)
{
    // Start in the upper left pixel
    // Without the transposition, the bottom left is the first index
    uint16_t bufferStartPx = NES_PX_WIDTH * NES_PX_HEIGHT - NES_PX_WIDTH - (NES_PX_WIDTH * y) + x;
    uint16_t bmStartPx = bitmap.BitmapInfo.bmiHeader.biWidth * bitmap.BitmapInfo.bmiHeader.biHeight - bitmap.BitmapInfo.bmiHeader.biWidth;

    PIXEL32 px = {0};

    // TODO Implement SIMD
    // For now only one by one pixel

    for (uint16_t y = 0; y < bitmap.BitmapInfo.bmiHeader.biWidth; y++)
    {
        for (uint16_t x = 0; x < bitmap.BitmapInfo.bmiHeader.biHeight; x++)
        {
            // TODO Implement brightness
            *((PIXEL32 *)backBuffer.Memory + bufferStartPx + x - y * NES_PX_WIDTH) = *((PIXEL32 *)bitmap.Memory + bmStartPx + x - bitmap.BitmapInfo.bmiHeader.biWidth * y);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_EXIT:
            PostMessage(window, WM_CLOSE, 0, 0);
            break;
        case ID_FILE_OPEN:
        {
            char filenameBuf[256] = {0};
            OPENFILENAMEA filename;
            filename.lStructSize = sizeof(filename);
            filename.hwndOwner = window;
            filename.hInstance = NULL;
            filename.lpstrFilter = "NES Roms\0*.nes\0\0";
            filename.lpstrCustomFilter = NULL;
            filename.nMaxCustFilter = 0;
            filename.nFilterIndex = 1;
            filename.lpstrFile = filenameBuf;
            filename.nMaxFile = 256;
            filename.lpstrFileTitle = NULL;
            filename.nMaxFileTitle = 0;
            filename.lpstrInitialDir = NULL;
            filename.lpstrTitle = "Open NES rom";
            filename.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
            BOOL open = GetOpenFileNameA(&filename);

            if (!open)
            {
                DWORD error = CommDlgExtendedError();
                if (error)
                {
                    Logf("Error: %d", LL_ERROR, error);
                    MessageBoxA(window, "Unable to open the file", "File error", MB_ICONEXCLAMATION | MB_OK);
                }
                Logf("Open: %d", LL_DEBUG, open);
                break;
            }

            Logf("Opening: %s", LL_INFO, filename.lpstrFile);

            HANDLE nesFileHandle = CreateFileA(
                filename.lpstrFile,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (nesFileHandle == INVALID_HANDLE_VALUE)
            {
                Logf("Unable to open file handle", LL_ERROR);
                break;
            }

            LOAD_STATUS status = loadNESFile(nesFileHandle);

            if (status == SUCCESS)
            {
                cpu_power_up();
                ppu_power_up();
            }

            CloseHandle(nesFileHandle);
        }
        break;
        case ID_OPTIONS_TOGGLE_DEBUG:
            perfData.DisplayDebugInfo = !perfData.DisplayDebugInfo;
            break;
        case ID_WINDOW_SET_MAX_SCALE:
            SetWindowToMatchScale(perfData.MaxScaleFactor);
            break;
        case ID_WINDOW_SET_MIN_SCALE:
            SetWindowToMatchScale(1);
            break;
        case ID_WINDOW_SET_MATCH:
            SetWindowToMatchScale(perfData.CurrentScaleFactor);
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
    {
        HDC hdc;
        PAINTSTRUCT ps;

        hdc = BeginPaint(window, &ps);
        RenderFrame(hdc);
        EndPaint(window, &ps);
    }
    break;
    case WM_CLOSE:
        running = FALSE;
        Log("CPU:", LL_DEBUG);
        log_cpu_mem();
        Log("PPU:", LL_DEBUG);
        log_ppu_memory();
        Log("Terminating", LL_INFO);
        VirtualFree(cartrage, 0, MEM_RELEASE);
        VirtualFree(backBuffer.Memory, 0, MEM_RELEASE);
        CloseLogFile();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CreateLogFile();
    Log("Created Log file", LL_INFO);

    MSG msg;

    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        Log("failed to set global time resolution", LL_ERROR);
    }

    // Finds the performance frequency, number of "performance ticks" per second
    QueryPerformanceFrequency((LARGE_INTEGER *)&perfData.PerfFrequency);

    if (CreateMainWindow() != ERROR_SUCCESS)
    {
        Log("Failed to create main window", LL_ERROR);
        return 1;
    }

    // Initialize the backbuffer
    backBuffer.BitmapInfo.bmiHeader.biSize = sizeof(backBuffer.BitmapInfo.bmiHeader);
    backBuffer.BitmapInfo.bmiHeader.biWidth = NES_PX_WIDTH;
    backBuffer.BitmapInfo.bmiHeader.biHeight = NES_PX_HEIGHT;
    backBuffer.BitmapInfo.bmiHeader.biBitCount = NES_BPP;
    backBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
    backBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
    backBuffer.Memory = VirtualAlloc(NULL, DRAW_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (backBuffer.Memory == NULL)
    {
        Logf("Unable to allocate backbuffer memory of %d bytes", LL_ERROR, DRAW_AREA_MEMORY_SIZE);
    }

    running = TRUE;
    cpu.powered = FALSE;
    perfData.DisplayDebugInfo = FALSE;

    int64_t frameStart, frameEnd, elapsedTime;
    int64_t cookedAccumulatedMicroseconds = 0;
    int64_t rawAccumulatedMicroseconds = 0;
    while (running)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&frameStart);

        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&msg);
        }

        ProcessInput();
        RenderFrame(NULL);
        perfData.TotalFramesRendered += 1;

        uint64_t prev_cpu_cycles = cpu.cycle;
        //for (uint64_t i = 0; i < CYCLES_PER_SEC * elapsedTime / 1000000; i++)
        while (cpu.cycle < prev_cpu_cycles + CYCLES_PER_SEC / 60 && cpu.powered)
        {
            perform_next_instruction();

            while (ppu_state.cycle < cpu.cycle * 3)
            {
                perform_next_ppu_cycle();
            }
        }

        // Calculate the raw frame time in microseconds
        QueryPerformanceCounter((LARGE_INTEGER *)&frameEnd);
        // This results in the time passed in microsecond
        elapsedTime = (frameEnd - frameStart) * 1000000 / perfData.PerfFrequency;
        rawAccumulatedMicroseconds += elapsedTime;

        // Sleep for the rest of the frame time
        //Logf("Time elapsed %dms", LL_DEBUG, elapsedTime / 1000);
        if ((TARGET_MICROSECONDS_PER_FRAME - elapsedTime) > 0)
        {
            //Logf("Sleeping for %dms", LL_DEBUG, (TARGET_MICROSECONDS_PER_FRAME - elapsedTime) / 1000);
            Sleep((TARGET_MICROSECONDS_PER_FRAME - elapsedTime) / 1000);
        }

        // Calulate the cooked frame time in microseconds
        QueryPerformanceCounter((LARGE_INTEGER *)&frameEnd);
        // This results in the time passed in microsecond
        elapsedTime = (frameEnd - frameStart) * 1000000 / perfData.PerfFrequency;
        cookedAccumulatedMicroseconds += elapsedTime;

        if (perfData.TotalFramesRendered % AVG_FRAMERATE_FRAME_SAMPLES == 0)
        {
            perfData.RawFPSAverage = 1000000.0f * AVG_FRAMERATE_FRAME_SAMPLES / rawAccumulatedMicroseconds;
            perfData.CookedFPSAverage = 1000000.0f * AVG_FRAMERATE_FRAME_SAMPLES / cookedAccumulatedMicroseconds;

            //Logf("Avg raw FPS:%f", LL_DEBUG, perfData.RawFPSAverage);
            //Logf("Avg cooked FPS:%f", LL_DEBUG, perfData.CookedFPSAverage);

            rawAccumulatedMicroseconds = 0;
            cookedAccumulatedMicroseconds = 0;
        }
    }

    return msg.wParam;
}

DWORD CreateMainWindow(void)
{
    WNDCLASSEX windowClass;

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleA(NULL);
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    windowClass.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassEx(&windowClass))
    {
        Log("Failed to register the window class", LL_ERROR);
        return GetLastError();
    }

    window = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        "Emunes",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NES_PX_WIDTH,
        NES_PX_HEIGHT,
        NULL,
        NULL,
        GetModuleHandleA(NULL),
        NULL);

    if (!window)
    {
        Log("Failed to create the window", LL_ERROR);
        return GetLastError();
    }

    Log("Created window", LL_INFO);

    perfData.MonitorInfo.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &perfData.MonitorInfo))
    {
        Log("Unable to get monitor info", LL_ERROR);
        return 0x00261001L; //ERROR_MONITOR_NO_DESCRIPTOR;
    }

    // Find the maximum scale factor window
    LONG monitorWidth = perfData.MonitorInfo.rcMonitor.right - perfData.MonitorInfo.rcMonitor.left;
    LONG monitorHeight = perfData.MonitorInfo.rcMonitor.bottom - perfData.MonitorInfo.rcMonitor.top;

    // This assumes that integer division truncates towards zero
    uint8_t maxWscale = (monitorWidth / NES_PX_WIDTH) - 1;
    uint8_t maxHscale = (monitorHeight / NES_PX_HEIGHT) - 1;

    // Select the smallest of the scales
    perfData.MaxScaleFactor = maxWscale;
    if (maxWscale > maxHscale)
    {
        perfData.MaxScaleFactor = maxHscale;
    }
    perfData.CurrentScaleFactor = perfData.MaxScaleFactor;

    if (!SetWindowLongPtrA(window, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW))
    {
        Log("SetWindowLongPtrA failed", LL_ERROR);
        return GetLastError();
    }

    DWORD result = SetWindowToMatchScale(perfData.MaxScaleFactor);

    if (result)
    {
        return result;
    }

    return ERROR_SUCCESS;
}

void ProcessInput(void)
{
    // The GetAsyncKeyState function may return different numbers based on when the button was pressed
    // 0 is the only instance where the button is not clicked. This is used to create a 'truth' value (0 or 1) of the button press
    controller.a = GetAsyncKeyState(A_KEYCODE) != 0;
    controller.b = GetAsyncKeyState(B_KEYCODE) != 0;
    controller.DPAD_up = GetAsyncKeyState(DPAD_UP) != 0;
    controller.DPAD_down = GetAsyncKeyState(DPAD_DOWN) != 0;
    controller.DPAD_left = GetAsyncKeyState(DPAD_LEFT) != 0;
    controller.DPAD_right = GetAsyncKeyState(DPAD_RIGHT) != 0;
    controller.start = GetAsyncKeyState(START_KEYCODE) != 0;
    controller.select = GetAsyncKeyState(SELECT_KEYCODE) != 0;
}

DWORD SetWindowToMatchScale(uint8_t scale)
{
    // This is used to find the window size given the required client area
    RECT adjustedWindowRect = {0};
    adjustedWindowRect.bottom = NES_PX_HEIGHT * scale;
    adjustedWindowRect.right = NES_PX_WIDTH * scale;
    if (!AdjustWindowRect(&adjustedWindowRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, TRUE))
    {
        Log("Unable to adjust window", LL_WARNING);
    }

    LONG windowW = adjustedWindowRect.right - adjustedWindowRect.left;
    LONG windowH = adjustedWindowRect.bottom - adjustedWindowRect.top;

    if (!SetWindowPos(window, HWND_TOP, perfData.MonitorInfo.rcMonitor.left, perfData.MonitorInfo.rcMonitor.top, windowW, windowH, SWP_FRAMECHANGED | SWP_NOOWNERZORDER))
    {
        Log("SetWindowPos failed", LL_ERROR);
        return GetLastError();
    }

    return 0;
}