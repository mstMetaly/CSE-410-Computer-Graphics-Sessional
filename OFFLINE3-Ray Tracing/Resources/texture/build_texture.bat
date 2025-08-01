@echo off
echo Compiling texture generator...
g++ -o create_texture.exe create_texture.cpp
if %ERRORLEVEL% == 0 (
    echo Running texture generator...
    create_texture.exe
    if %ERRORLEVEL% == 0 (
        echo Texture created successfully!
        echo Compiling main ray tracer...
        g++ -o main.exe main.cpp -lopengl32 -lglu32 -lglut32
        if %ERRORLEVEL% == 0 (
            echo Build successful! Run main.exe to see textured floor.
        ) else (
            echo Failed to compile main program.
        )
    ) else (
        echo Failed to create texture.
    )
) else (
    echo Failed to compile texture generator.
)
pause 