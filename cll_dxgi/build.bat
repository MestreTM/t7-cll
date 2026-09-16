@echo off
setlocal
cd /d "%~dp0"

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
  for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    call "%%i\VC\Auxiliary\Build\vcvars64.bat" >nul
  )
)

if exist dxgi.dll del /q dxgi.dll
if exist dxgi_proxy.obj del /q dxgi_proxy.obj
if exist dxgi.lib del /q dxgi.lib
if exist dxgi.exp del /q dxgi.exp

cl /nologo /LD /O2 /MD /W3 dxgi_proxy.cpp /link /DEF:dxgi.def /OUT:dxgi.dll
if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo Built dxgi.dll
echo Copy to the game folder AFTER removing ReShade dxgi.dll:
echo   X:\games\t7_full_game\dxgi.dll
