cls

powershell -Command "if (-not(Get-Process | Where-Object { $_.MainWindowTitle -eq 'Developer Command Prompt for VS 2019' })) { exit 0 } else { exit 1 }"

:: If not open, run vcvars64.bat to set up environment
if %errorlevel% equ 0 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)


del *.obj
del /Q logs\*.log
if exist OGL.exe (
    del *.exe
    del *.res
)

cl.exe /c /EHsc /I C:\OpenGL\glew\include /I C:\OpenGL\assimp\include /I C:\OpenGL  OGL.cpp Imgui\imgui_impl_opengl3.cpp Imgui\imgui_impl_win32.cpp Imgui\imgui*.cpp

rc.exe OGL.rc

link.exe OGL.obj imgui*.obj  OGL.res /LIBPATH:C:\OpenGL\glew\lib\Release\x64 user32.lib gdi32.lib  /LIBPATH:C:\OpenGL\assimp\lib assimp-vc143-mtd.lib /subsystem:windows


if exist OGL.exe (
    del *.obj
    del *.res
    OGL.exe
)
