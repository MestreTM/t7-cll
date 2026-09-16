# CLL DXGI proxy

Tiny `dxgi.dll` for Black Ops III / boiii.exe. It loads first from the game
folder, forwards real DXGI to `C:\Windows\System32\dxgi.dll`, hooks the 1920x1080
swap chain Present, and calls `CLL_OnPresent` exported by `boiii.exe`.

## Build

```bat
cd cll-dxgi
build.bat
```

Needs VS x64 Developer tools (`cl`).

## Install

1. Uninstall ReShade (delete the game-folder `dxgi.dll` first).
2. Copy the built `dxgi.dll` to `X:\games\t7_full_game\dxgi.dll`.
3. Rebuild boiii with the updated `download_overlay.cpp` (exports `CLL_OnPresent`).
4. Launch `boiii.exe -launch`.

Do not keep ReShade `dxgi.dll` and this file at the same time.

## Layout

- `dxgi_proxy.cpp` — forwards + CreateSwapChain/Present hook
- `dxgi.def` — exported names
- `build.bat` — cl /LD
