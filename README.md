# Demo Starter for Win32

Sets up a basic Win32 application, with a framebuffer and audio.

Poke around in main.c to get started. The demo in the repository takes up `1496 bytes` of space.

![Demo screenshot](https://ske.land/r/boxesese.3ay.png)

## Building

You need to have:
 - [msvc](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
 - [crinkler](https://github.com/runestubbe/Crinkler)

Run `build.bat` in Visual Studio Developer Command Prompt for ***32 BIT***.

Yes, it means you can't use 64-bit, but if you're aiming for a small demo 64 bit isn't a very good choice anyway.

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
            break;
    }
    return DefWindowProcA(Window, Msg, wParam, lParam);
}
``` 
> [!TIP]
> Look [here](https://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm) for list of messages. 

`CallbackGetSample(win32_setup *Setup)` to get a sample for the audio callback. It's called at a rate of 44100 Hz. For example:
```c
audio_sample CallbackGetSample(win32_setup *Setup)
{
    // Plays a square wave
    static int LastPosition = 0;
    int SampleValue = ((LastPosition/100%2) == 0 ? 32767 : -32767)/2;
    LastPosition++;
    // You can specify left and right sample values independently
    return (audio_sample){ SampleValue, SampleValue };
}
```

`CallbackTeardown(win32_setup *Setup)` to tear down your game state. For example:
```c
void CallbackTeardown(win32_setup *Setup)
{
    // Free any resources you allocated, close any files you opened, etc.
}
```

## Tips

Feel free to edit `win32.c`, it's a reference implementation. Add DirectX or OpenGL or whatever you want.

Maybe you want to put sources into the `src/` directory, you can change `build.bat` to handle that.

`win32_setup` is passed to all callbacks, you can access the guts of the setup there.

