#include "win32.c"

typedef struct
{
    int P;
    int X;
} game_state;

game_state GameState = { 0 };

win32_setup_info CallbackSetup()
{
    // You can set up game state here...
    win32_setup_info Setup = { 0 };
    Setup.Title = "Unnamed";
    Setup.Width = 800;
    Setup.Height = 600;

    GameState.P = 100;

    return Setup;
}

void CallbackFrame(win32_setup *Setup, DWORD32 *FrameBuffer, int Width, int Height)
{
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            FrameBuffer[i*Width+j] = i^j+GameState.X;
        }
    }
}

LRESULT CallbackEvent(win32_setup *Setup, HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_LEFT) GameState.X--;
        if (wParam == VK_RIGHT) GameState.X++;
        if (wParam == VK_DOWN) GameState.P++;
        if (wParam == VK_UP) GameState.P--;
        break;
    }

    if (GameState.P < 20) GameState.P = 20;
    if (GameState.P > 1000) GameState.P = 1000;

    return DefWindowProcA(Window, Msg, wParam, lParam);
}

// Samples served at a rate of 44100 Hz
// Warning: This runs on a different thread, expect race conditions!
audio_sample CallbackGetSample(win32_setup *Setup)
{
    // Plays a square wave
    static int Cycle = 0;
    static int Timer = 0;

    if (Timer > GameState.P)
    {
        Cycle = !Cycle;
        Timer = 0;
    }

    Timer++;

    int SampleValue = (Cycle == 0 ? 32767 : -32767)/16;
    return (audio_sample){ SampleValue, SampleValue };
}

void CallbackTeardown(win32_setup *Setup)
{
    // Free any resources you allocated, close any files you opened, save any data, etc.
}


