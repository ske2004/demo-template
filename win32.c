#include <windows.h>
#include <dsound.h>

typedef struct {
    IDirectSoundBuffer *SecondaryBuffer;
    HANDLE Thread;

    DWORD BytesPerSample;

    DWORD BufferSize_Samples;
    DWORD Index_Samples;
    DWORD Ahead_Samples;
} win32_audio;

typedef struct {
    HWND Window;
    win32_audio Audio;
    UINT_PTR FrameTimer;
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

static void __AdvanceCursor(win32_audio *Audio)
{
    Audio->Index_Samples++;
    if (Audio->Index_Samples >= Audio->BufferSize_Samples)
    {
        Audio->Index_Samples = 0;
    }
}

static void __FillBuffer(win32_setup *Setup, DWORD SamplesToWrite)
{
    win32_audio *Audio = &Setup->Audio;

    DWORD Cursor_Bytes = Audio->Index_Samples * Audio->BytesPerSample;
    DWORD Write_Bytes = SamplesToWrite * Audio->BytesPerSample;

    void *Region1, *Region2;
    DWORD Bytes1, Bytes2;
    if (SUCCEEDED(Audio->SecondaryBuffer->lpVtbl->Lock(Audio->SecondaryBuffer, Cursor_Bytes, Write_Bytes, &Region1, &Bytes1, &Region2, &Bytes2, 0)))
    {
        for (int i = 0; i < Bytes1/Audio->BytesPerSample; i++)
        {
            audio_sample Sample = CallbackGetSample(Setup);
            ((audio_sample*)Region1)[i] = Sample;
            __AdvanceCursor(Audio);
        }
        for (int i = 0; i < Bytes2/Audio->BytesPerSample; i++)
        {
            audio_sample Sample = CallbackGetSample(Setup);
            ((audio_sample*)Region2)[i] = Sample;
            __AdvanceCursor(Audio);
        }
        Audio->SecondaryBuffer->lpVtbl->Unlock(Audio->SecondaryBuffer, Region1, Bytes1, Region2, Bytes2);
    }
}

static DWORD WINAPI __AudioThread(LPVOID Param)
{
    win32_setup *Setup = (win32_setup*)Param;
    win32_audio *Audio = &Setup->Audio;

    while (1)
    {
        DWORD _PlayCursor, _WriteCursor;
        if (SUCCEEDED(Audio->SecondaryBuffer->lpVtbl->GetCurrentPosition(Audio->SecondaryBuffer, &_PlayCursor, &_WriteCursor)))
        {
            DWORD PlayCursor = _PlayCursor/Audio->BytesPerSample;
            DWORD WriteCursor = (Audio->Index_Samples % Audio->BufferSize_Samples);
            DWORD TargetCursor = (PlayCursor + Audio->Ahead_Samples) % Audio->BufferSize_Samples;
            DWORD SamplesToWrite = 0;
            if (TargetCursor < WriteCursor)
            {
                SamplesToWrite = (Audio->BufferSize_Samples - WriteCursor) + TargetCursor;
            }
            else
            {
                SamplesToWrite = TargetCursor - WriteCursor;
            }

            __FillBuffer(Setup, SamplesToWrite);
        }

        Sleep(5);
    }

    return 0;
}

static void __DoFrame(HWND hwnd)
{
    __BlitToWindow(hwnd);
}

static LRESULT CALLBACK __WndProc(
    HWND   hWnd,
    UINT   Msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (Msg) {
    case WM_TIMER:
        if (wParam == GLB_Setup.FrameTimer)
        {
            __DoFrame(hWnd);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        TerminateThread(GLB_Setup.Audio.Thread, 0);
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
    const int BufferSize = HzRate*2*2;

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

    if (FAILED(PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer, &Format)))
    {
        __FatalError("SetFormat failed");
    }

    static DSBUFFERDESC PlaybackBufferDesc = { 0 };
    PlaybackBufferDesc.dwSize = sizeof PlaybackBufferDesc;
    PlaybackBufferDesc.dwBufferBytes = BufferSize;
    PlaybackBufferDesc.lpwfxFormat = &Format;
    PlaybackBufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_GETCURRENTPOSITION2;

    IDirectSoundBuffer *PlaybackBuffer;
    if (FAILED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &PlaybackBufferDesc, &PlaybackBuffer, NULL)))
    {
        __FatalError("CreateSoundBuffer failed");
    }

    IDirectSoundNotify *Notify;
    if (FAILED(PlaybackBuffer->lpVtbl->QueryInterface(PlaybackBuffer, &IID_IDirectSoundNotify, (void**)&Notify)))
    {
        __FatalError("QueryInterface failed");
    }

    Setup->Audio.SecondaryBuffer = PlaybackBuffer;
    Setup->Audio.BytesPerSample = 2 * Format.nChannels;
    Setup->Audio.BufferSize_Samples = BufferSize / Setup->Audio.BytesPerSample;
    Setup->Audio.Ahead_Samples = HzRate/10;
    Setup->Audio.Index_Samples = 0;
    __FillBuffer(Setup, Setup->Audio.Ahead_Samples*10);

    Setup->Audio.Thread = CreateThread(NULL, 0, __AudioThread, Setup, 0, NULL);
    Setup->Audio.SecondaryBuffer->lpVtbl->Play(Setup->Audio.SecondaryBuffer, 0, 0, DSBPLAY_LOOPING);
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
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        SetupInfo.Width, SetupInfo.Height,
        NULL,
        NULL,
        NULL,
        NULL
    );

    GLB_Setup.FrameTimer = 1;
    SetTimer(Window, GLB_Setup.FrameTimer, 16, NULL);

    GLB_Setup.Window = Window;
    __InitializeDirectsound(&GLB_Setup);

    static MSG Msg = { 0 };
    while (GetMessageA(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    ExitProcess(0);
}