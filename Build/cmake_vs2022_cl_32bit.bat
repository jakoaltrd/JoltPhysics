@echo off
cmake -S . -B VS2022_CL_32BIT -G "Visual Studio 17 2022" -A Win32 -DUSE_SSE4_1=OFF -DUSE_SSE4_2=OFF -DUSE_AVX=OFF -DUSE_AVX2=OFF -DUSE_LZCNT=OFF -DUSE_TZCNT=OFF -DUSE_F16C=OFF -DUSE_FMADD=OFF
echo Open VS2022_CL_32BIT\JoltPhysics.sln to build the project.