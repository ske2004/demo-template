# Demo Starter for Win32

Sets up a basic Win32 application, with a framebuffer and audio.

Poke around in main.c to get started. The demo in the repository takes up `1657 bytes` of space.

![Demo screenshot](https://ske.land/r/boxesese.3ay.png)

## Building

You need to have:
 - [msvc](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
 - [crinkler](https://github.com/runestubbe/Crinkler)

Set up the environment by running `init_msvc.bat`, and then run `build.bat` :)

The project is limited to 32 bits.

> [!WARNING]
> There is no C standard library, so you can't use `printf` or `malloc` or anything like that. 

> [!TIP]
> Use `VirtualAlloc` and `VirtualFree` to allocate and free memory.

## Usage

`CallbackSetup()` to set up your game state. For example:
```c
win32_setup_info CallbackSetup()
{
    win32_setup_info Setup = { 0 };
    Setup.Title = "Unnamed";
    Setup.Width = 800;
    Setup.Height = 600;
    return Setup;
}
```

`CallbackFrame(win32_setup *Setup, DWORD32 *FrameBuffer, int Width, int Height)` to draw to the framebuffer (32-bit ARGB). For example:
```c
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
```

`CallbackEvent(win32_setup *Setup, HWND Window, UINT Msg, WPARAM wParam, LPARAM lParam)` to handle events. For example:
```c
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
``` 
> [!TIP]
> Look [here](https://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm) for list of messages. 

`CallbackGetSample(win32_setup *Setup)` to get a sample for the audio callback. It's called at a rate of 44100 Hz. For example:
```c
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
```
> [!WARNING]
> This callback runs on a different thread, you need to use mutexes, semaphores, critical sections e.t.c. to avoid race conditions. 

`CallbackTeardown(win32_setup *Setup)` to destroy your game state. For example:
```c
void CallbackTeardown(win32_setup *Setup)
{
    // Free any resources you allocated, close any files you opened, save any data, etc.
}
```

## Tips

Feel free to edit `win32.c`, it's a reference implementation. Add DirectX or OpenGL or whatever you want.

Maybe you want to put sources into the `src/` directory, you can change `build.bat` to handle that.

`win32_setup` is passed to all callbacks, you can access the guts of the setup there.

