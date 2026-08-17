@echo off
setlocal EnableDelayedExpansion

echo =====================================
echo   Volatile Setup Script (Windows)
echo =====================================

:: ==========================================
:: Volatile Setup Configuration
:: ==========================================
set VENV_DIR=.venv
set REQUIRED_TOOLS=cmake
set OPTIONAL_TOOLS=fastchess cutechess-cli

:: 1. Resolve Python (python3 vs python)
where python >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set PYTHON_CMD=python
) else (
    set PYTHON_CMD=python3
)
set REQUIRED_TOOLS=%REQUIRED_TOOLS% %PYTHON_CMD%

:: 2. Resolve C++ Compiler (cl vs g++)
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set REQUIRED_TOOLS=%REQUIRED_TOOLS% cl
) else (
    where g++ >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        set REQUIRED_TOOLS=%REQUIRED_TOOLS% g++
    ) else (
        :: Fallback so it fails gracefully in check loop
        set REQUIRED_TOOLS=%REQUIRED_TOOLS% cl
    )
)

:: ==========================================
:: Main Execution
:: ==========================================

:: Check Required Tools
for %%T in (%REQUIRED_TOOLS%) do (
    call :CheckTool %%T true
)

:: Check Optional Tools
for %%T in (%OPTIONAL_TOOLS%) do (
    call :CheckTool %%T false
)

:: Setup Virtual Environment
call :SetupVenv

echo =====================================
echo Setup complete!
echo To activate the virtual environment manually, run: %VENV_DIR%\Scripts\activate
echo To build the engine, run:
echo   mkdir build ^&^& cd build ^&^& cmake .. -DUSE_NNUE=OFF ^&^& cmake --build . --config Release
exit /b 0

:: ==========================================
:: Subroutines
:: ==========================================

:CheckTool
set TOOL=%1
set IS_REQUIRED=%2

where %TOOL% >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    :: Try to print version if possible
    for /f "tokens=*" %%i in ('%TOOL% --version 2^>nul ^| findstr /R "."') do (
        echo [+] Found %TOOL%: %%i
        exit /b 0
    )
    echo [+] Found %TOOL%
) else (
    if "%IS_REQUIRED%"=="true" (
        echo [-] Error: Required tool '%TOOL%' is missing.
        exit /b 1
    ) else (
        echo [-] Warning: Optional tool '%TOOL%' is missing.
    )
)
exit /b 0

:SetupVenv
echo -------------------------------------
    if exist %VENV_DIR% (
        echo Removing existing virtual environment to ensure a clean build...
        rmdir /s /q %VENV_DIR%
    )
    
    echo Creating Python virtual environment in %VENV_DIR%...
    %PYTHON_CMD% -m venv %VENV_DIR%

echo Activating virtual environment...
call %VENV_DIR%\Scripts\activate.bat

echo Installing Python dependencies...
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
exit /b 0
