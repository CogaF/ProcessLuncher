# Dependencies

Organised like the other projects made from the wxAppTemplate kit (<https://github.com/CogaF/template>),
but Process Launcher needs **only wxWidgets**: no SQLite and no serial library.

| Dependency | Version | Found through |
|---|---|---|
| Visual Studio | 2022 (v143) or 2026 (v145), workload "Desktop development with C++" | the default platform toolset is used |
| wxWidgets | 3.3.x (3.3.3 recommended; dark mode needs 3.3+) | environment variable `WXWIN` |

```bat
setx WXWIN C:\libs\wxWidgets-3.3.3
```

Restart Visual Studio afterwards. Build wxWidgets (`%WXWIN%\build\msw\wx_vc17.sln`, Batch Build) for
the configurations you need:

| Project configuration | wxWidgets configuration | Platform | Library folder |
|---|---|---|---|
| Debug / Release | Debug / Release | x64 | `lib\vc_x64_lib` |
| Debug_DLL / Release_DLL | DLL Debug / DLL Release | x64 | `lib\vc_x64_dll` |
| Debug / Release | Debug / Release | Win32 | `lib\vc_lib` |
| Debug_DLL / Release_DLL | DLL Debug / DLL Release | Win32 | `lib\vc_dll` |

The project does not list the wxWidgets `.lib` files: `wx/msw/setup.h` (through `%WXWIN%\include\msvc`)
links the right ones. To use another layout override `WxLibDir` in a `Directory.Build.props` next to the
`.sln`. The DLL configurations copy the wxWidgets DLLs next to the exe after each build.

The manifest (common controls v6, per-monitor DPI awareness) is written by the linker, so
`wxUSE_NO_MANIFEST=1` is defined for the resource compiler; without it the dark appearance does not work.
