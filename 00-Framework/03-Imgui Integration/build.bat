cls

del /Q logs\*.log

del *.obj

if exist OGL.exe (
    del *.exe
    del *.res
)

cl.exe /c /EHsc /I C:\OpenGL\ /I C:\OpenGL\glew\include OGL.cpp

rc.exe OGL.rc

link.exe OGL.obj OGL.res /LIBPATH:C:\OpenGL\glew\lib\Release\x64 user32.lib gdi32.lib /subsystem:windows

if exist OGL.exe (
    del *.obj
    del *.res
    OGL.exe
)
