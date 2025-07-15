@echo off
echo Building PhysicsStudio...

cd Build
call cmake_vs2022_cl.bat

if %ERRORLEVEL% EQU 0 (
    echo CMake configuration completed successfully.
    echo.
    echo Building PhysicsStudio...
    cmake --build ../build --config Release --target PhysicsStudio
    
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo PhysicsStudio built successfully!
        echo Executable location: build\Release\PhysicsStudio.exe
        echo.
        echo To run PhysicsStudio:
        echo   cd build\Release
        echo   PhysicsStudio.exe
    ) else (
        echo Build failed with error %ERRORLEVEL%
    )
) else (
    echo CMake configuration failed with error %ERRORLEVEL%
)

cd ..
pause