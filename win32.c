#include <windows.h>
#include <dsound.h>

typedef struct {
    HWND Window;

    // Directsound
    IDirectSoundBuffer *SecondaryBuffer;
    DWORD BytesPerSample;
    DWORD BufferSize;
    DWORD SampleIndex;
} win32_setup;

typedef struct {
    short Left;
    short Right;
} audio_sample;

typedef struct {
    const char *Title;
    size_t Width;
    size_t Height;
} win32_setup_info;

win32_setup_info CallbackSetup();
LRESULT CallbackEvent(win32_setup *Setup, HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam);
void CallbackFrame(win32_setup *Setup, DWORD32 *FrameBuffer, int Width, int Height);
audio_sample CallbackGetSample(win32_setup *Setup);
void CallbackTeardown(win32_setup *Setup);

static win32_setup GLB_Setup;

static void __BlitToWindow(HWND Window)
{
    RECT Rect;
    GetWindowRect(Window, &Rect);

    int Width = Rect.right-Rect.left;
    int Height = Rect.bottom-Rect.top;

    DWORD32 *Buffer = VirtualAlloc(NULL, Width*Height*4, MEM_COMMIT, PAGE_READWRITE);

    CallbackFrame(&GLB_Setup, Buffer, Width, Height);

    HDC DC = GetDC(Window);
    BITMAPINFOHEADER BmpInfo = { 0 };
    BmpInfo.biSize = sizeof BmpInfo;
    BmpInfo.biPlanes = 1;
    BmpInfo.biBitCount = 32;
    BmpInfo.biWidth = Width;
    BmpInfo.biHeight = -Height;

    SetDIBitsToDevice(DC, 0, 0, Width, Height, 0, 0, 0, Height, Buffer, (BITMAPINFO*)&BmpInfo, DIB_RGB_COLORS);

    ReleaseDC(Window, DC);

    VirtualFree(Buffer, 0, MEM_RELEASE);
}

static void __UpdateAudio()
{
    win32_setup *Setup = &GLB_Setup;

    DWORD PlayCursor, WriteCursor;
    if (SUCCEEDED(Setup->SecondaryBuffer->lpVtbl->GetCurrentPosition(Setup->SecondaryBuffer, &PlayCursor, &WriteCursor)))
    {
        DWORD Cursor = Setup->SampleIndex * Setup->BytesPerSample % Setup->BufferSize;
        DWORD BytesToWrite;
        if (PlayCursor < WriteCursor)
        {
            BytesToWrite = Setup->BufferSize - Cursor;
            BytesToWrite += PlayCursor;
        }
        else
        {
            BytesToWrite = PlayCursor - Cursor;
        }

        void *Region1, *Region2;
        DWORD Bytes1, Bytes2;
        if (SUCCEEDED(Setup->SecondaryBuffer->lpVtbl->Lock(Setup->SecondaryBuffer, Cursor, BytesToWrite, &Region1, &Bytes1, &Region2, &Bytes2, 0)))
        {
            for (int i = 0; i < Bytes1/Setup->BytesPerSample; i++)
            {
                audio_sample Sample = CallbackGetSample(Setup);
                ((audio_sample*)Region1)[i] = Sample;
                Setup->SampleIndex++;
            }
            for (int i = 0; i < Bytes2/Setup->BytesPerSample; i++)
            {
                audio_sample Sample = CallbackGetSample(Setup);
                ((audio_sample*)Region2)[i] = Sample;
                Setup->SampleIndex++;
            }
            Setup->SecondaryBuffer->lpVtbl->Unlock(Setup->SecondaryBuffer, Region1, Bytes1, Region2, Bytes2);
        }
    }
}

static void __DoFrame(HWND hwnd)
{
    __UpdateAudio();
    __BlitToWindow(hwnd);
}

static UINT_PTR __FrameTimer = 1;

static LRESULT CALLBACK __WndProc(
    HWND   hWnd,
    UINT   Msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (Msg) {
    case WM_TIMER:
        if (wParam == __FrameTimer)
        {
            __DoFrame(hWnd);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY: 
        CallbackTeardown(&GLB_Setup);
        PostQuitMessage(0);
        break;
    }

    return CallbackEvent(&GLB_Setup, hWnd, Msg, wParam, lParam);
}

static void __FatalError(const char *Message)
{
    MessageBoxA(NULL, Message, "Fatal Error", MB_OK);
    ExitProcess(1);
}

static void __InitializeDirectsound(win32_setup *Setup)
{
    const int HzRate = 44100;
    const int BufferSize = HzRate*2*2*2;

    IDirectSound *DirectSound;
    if (FAILED(DirectSoundCreate(NULL, &DirectSound, NULL)))
    {
        __FatalError("DirectSoundCreate failed");
    }

    if (FAILED(DirectSound->lpVtbl->SetCooperativeLevel(DirectSound, Setup->Window, DSSCL_PRIORITY)))
    {
        __FatalError("SetCooperativeLevel failed");
    }
    DSBUFFERDESC BufferDesc = { 0 };
    BufferDesc.dwSize = sizeof BufferDesc;
    BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    IDirectSoundBuffer *PrimaryBuffer;
    if (FAILED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &BufferDesc, &PrimaryBuffer, NULL)))
    {
        __FatalError("CreateSoundBuffer failed");
    }

    WAVEFORMATEX Format = { 0 };
    Format.wFormatTag = WAVE_FORMAT_PCM;
    Format.nChannels = 2;
    Format.nSamplesPerSec = HzRate;
    Format.wBitsPerSample = 16;
    Format.nBlockAlign = Format.nChannels*Format.wBitsPerSample/8;
    Format.nAvgBytesPerSec = Format.nSamplesPerSec*Format.nBlockAlign;
    Format.cbSize = 0;

    if (FAILED(PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer, &Format)))
    {
        __FatalError("SetFormat failed");
    }

    DSBUFFERDESC PlaybackBufferDesc = { 0 };
    PlaybackBufferDesc.dwSize = sizeof PlaybackBufferDesc;
    PlaybackBufferDesc.dwBufferBytes = BufferSize;
    PlaybackBufferDesc.lpwfxFormat = &Format;
    PlaybackBufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS;

    IDirectSoundBuffer *PlaybackBuffer;
    if (FAILED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &PlaybackBufferDesc, &PlaybackBuffer, NULL)))
    {
        __FatalError("CreateSoundBuffer failed");
    }

    Setup->SecondaryBuffer = PlaybackBuffer;
    Setup->BytesPerSample = 2 * Format.nChannels;
    Setup->BufferSize = BufferSize;
}

int WinMainCRTStartup()
{
    win32_setup_info SetupInfo = CallbackSetup();

    WNDCLASSA WindowClass = { 0 };
    WindowClass.lpszClassName = "SmallWin32";
    WindowClass.lpfnWndProc = __WndProc;

    // NOTE(ske): No error handling, but not really necessary.
    RegisterClassA(&WindowClass);

    // NOTE(ske): No error handling, but not really necessary.
    HWND Window = CreateWindowA(
        WindowClass.lpszClassName,    
        SetupInfo.Title,
        WS_OVERLAPPEDWINDOW,          
        CW_USEDEFAULT, CW_USEDEFAULT,
        SetupInfo.Width, SetupInfo.Height,
        NULL,                        
        NULL,                       
        NULL,                      
        NULL                      
    );

    ShowWindow(Window, TRUE);

    SetTimer(Window, __FrameTimer, 16, NULL);

    win32_setup Setup = { 0 };
    Setup.Window = Window;
    __InitializeDirectsound(&Setup);
    Setup.SecondaryBuffer->lpVtbl->Play(Setup.SecondaryBuffer, 0, 0, DSBPLAY_LOOPING);

    GLB_Setup = Setup;

    MSG Msg = { 0 };
    while (GetMessageA(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg); 
        DispatchMessage(&Msg); 
    }

    ExitProcess(0);
}