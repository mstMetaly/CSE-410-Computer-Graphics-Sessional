@echo off
echo ===============================================
echo          TEXTURE GENERATOR AND SWITCHER
echo ===============================================
echo.

:menu
echo Choose an option:
echo 1. Generate ALL texture patterns
echo 2. Use Brick texture
echo 3. Use Wood texture  
echo 4. Use Marble texture
echo 5. Use Tile texture
echo 6. Use your own image file
echo 7. Exit
echo.
set /p choice=Enter your choice (1-7): 

if "%choice%"=="1" goto generate_all
if "%choice%"=="2" goto use_brick
if "%choice%"=="3" goto use_wood
if "%choice%"=="4" goto use_marble
if "%choice%"=="5" goto use_tile
if "%choice%"=="6" goto use_custom
if "%choice%"=="7" goto exit
goto menu

:generate_all
echo Generating all texture patterns...
g++ -o create_multiple_textures.exe create_multiple_textures.cpp
if %ERRORLEVEL% == 0 (
    create_multiple_textures.exe
    echo.
    echo All textures generated successfully!
) else (
    echo Failed to compile texture generator.
)
pause
goto menu

:use_brick
if exist texture_brick.bmp (
    copy texture_brick.bmp texture.bmp > nul
    echo ✓ Switched to BRICK texture
) else (
    echo ❌ texture_brick.bmp not found. Please generate textures first (option 1).
)
pause
goto menu

:use_wood
if exist texture_wood.bmp (
    copy texture_wood.bmp texture.bmp > nul
    echo ✓ Switched to WOOD texture
) else (
    echo ❌ texture_wood.bmp not found. Please generate textures first (option 1).
)
pause
goto menu

:use_marble
if exist texture_marble.bmp (
    copy texture_marble.bmp texture.bmp > nul
    echo ✓ Switched to MARBLE texture
) else (
    echo ❌ texture_marble.bmp not found. Please generate textures first (option 1).
)
pause
goto menu

:use_tile
if exist texture_tile.bmp (
    copy texture_tile.bmp texture.bmp > nul
    echo ✓ Switched to TILE texture
) else (
    echo ❌ texture_tile.bmp not found. Please generate textures first (option 1).
)
pause
goto menu

:use_custom
echo.
echo Place your image file in this directory and enter its name:
echo (Supported formats: .jpg, .png, .bmp, .tga, .gif)
set /p filename=Enter filename with extension: 
if exist "%filename%" (
    copy "%filename%" texture.bmp > nul
    echo ✓ Switched to custom texture: %filename%
) else (
    echo ❌ File "%filename%" not found.
)
pause
goto menu

:exit
echo Goodbye!
exit 