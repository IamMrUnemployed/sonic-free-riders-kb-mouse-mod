@echo off
REM Builds xinput1_4.dll and xinput1_3.dll (x64) with MinGW-w64.
REM Requires x86_64-w64-mingw32 g++ on PATH (e.g. MSYS2 "mingw-w64-x86_64-gcc").
cd /d "%~dp0"
if not exist build mkdir build
g++ -shared -O2 -static -static-libgcc -static-libstdc++ -o build\xinput1_4.dll src\xinput_kb_mouse.cpp src\xinput.def -luser32
if errorlevel 1 (
  echo Build failed.
  exit /b 1
)
copy /y build\xinput1_4.dll build\xinput1_3.dll >nul
copy /y config\sonic_kb_mouse.ini build\sonic_kb_mouse.ini >nul
echo Done: build\xinput1_4.dll, build\xinput1_3.dll, build\sonic_kb_mouse.ini
