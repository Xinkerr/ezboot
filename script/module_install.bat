@echo off
:: 检查 Python 是否已安装
python --version >nul 2>nul
if %errorlevel% neq 0 (
    echo Python 没有安装，请先安装 Python。
    exit /b 1
)

:: 检查 pip 是否已安装
python -m ensurepip --upgrade >nul 2>nul
if %errorlevel% neq 0 (
    echo pip 没有安装，请确保安装了 pip。
    exit /b 1
)

:: 安装所需的库
echo installing intelhex...
python -m pip install intelhex

echo installing pycryptodome...
python -m pip install pycryptodome

echo installing chardet...
python -m pip install chardet

:: 安装完成
echo compelet
pause
