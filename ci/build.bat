CALL setup.bat
cd Z:\prj
cl main.cpp /IZ:\opt\vc\include /GX /link /LIBPATH:Z:\opt\vc\LIB
move /Y main.exe winforth.exe
exit

