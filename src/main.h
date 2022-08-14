#ifndef MAIN_H

#define MAIN_H

#include <windows.h>
#include <stdint.h>

#define PROGRAM_NAME "Emunes"
#define WINDOW_CLASS_NAME "emunesWindowClass"

#define NES_PX_WIDTH 256
#define NES_PX_HEIGHT 240

 // Bits per pixel
#define NES_BPP 32
#define DRAW_AREA_MEMORY_SIZE (NES_PX_WIDTH * NES_PX_HEIGHT * NES_BPP / 8)

// How many frames pass between each calulation of the average framerate
#define AVG_FRAMERATE_FRAME_SAMPLES 120
// This results in 60 FPS
#define TARGET_MICROSECONDS_PER_FRAME 16667

typedef struct NES_BITMAP
{
    BITMAPINFO BitmapInfo;
    void* Memory;
} NES_BITMAP;

typedef union PIXEL32{
	struct BGRA
	{
		uint8_t Blue;
		uint8_t Green;
		uint8_t Red;
		uint8_t Alpha;
	} BGRA;

	DWORD Bytes;
} PIXEL32;

typedef struct PERFDATA
{
	uint64_t TotalFramesRendered;	

	float RawFPSAverage;

	float CookedFPSAverage;	

	int64_t PerfFrequency;	

	MONITORINFO MonitorInfo;

	BOOL DisplayDebugInfo;	

	ULONG MinimumTimerResolution;

	ULONG MaximumTimerResolution;

	ULONG CurrentTimerResolution;

	DWORD HandleCount;

	SYSTEM_INFO SystemInfo;

	int64_t CurrentSystemTime;

	int64_t PreviousSystemTime;

	double CPUPercent;

	uint8_t MaxScaleFactor;

	uint8_t CurrentScaleFactor;

} PERFDATA;

//////////// DECLARATIONS /////////////
// Variables

extern HWND window;
extern NES_BITMAP backBuffer;
extern PERFDATA perfData;
extern BOOL running;

// Functions

DWORD CreateMainWindow(void);
void RenderFrame(HDC hdc);
DWORD SetWindowToMatchScale(uint8_t scale);
void ProcessInput(void);

#endif