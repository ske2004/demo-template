#include "win32.c"

typedef struct
{
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
    return Setup;
}

void CallbackFrame(DWORD32 *FrameBuffer, int Width, int Height)
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
        break;
    }

    return DefWindowProcA(Window, Msg, wParam, lParam);
}

// Samples served at a rate of 44100 Hz
audio_sample CallbackGetSample()
{
    // Plays a square wave
    static int LastPosition = 0;
    int SampleValue = ((LastPosition/100%2) == 0 ? 32767 : -32767)/2;
    LastPosition++;
    return (audio_sample){ SampleValue, SampleValue };
}

void CallbackTeardown()
{
    // TODO!
}


