call C:\sdk\emsdk\emsdk_env.bat
cd pfm\emscripten\build
c:\sdk\CMake\bin\cmake -DCMAKE_TOOLCHAIN_FILE=c:/sdk/emsdk/emscripten/1.37.36/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ../../..
c:\sdk\MinGW\bin\mingw32-make -j 8 VERBOSE=1
cd c:\dev\npw\techno-globe
