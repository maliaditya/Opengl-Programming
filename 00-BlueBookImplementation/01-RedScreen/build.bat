cls
del OGL.exe
del OGL.obj
del OGL.res

cl.exe /c /EHsc /I C:\OpenGL\glew\include OGL.cpp

rc.exe OGL.rc

link.exe OGL.obj OGL.res /LIBPATH:C:\OpenGL\glew\lib\Release\x64 user32.lib gdi32.lib /subsystem:windows
