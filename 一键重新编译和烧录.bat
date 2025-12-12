@echo off
title ESP32-S3 一键重新编译和烧录
color 0B

echo ================================================
echo ESP32-S3 一键重新编译和烧录
echo ================================================
echo.

REM 设置ESP-IDF路径
set "IDF_PATH=D:\esp32-idf-ahy\5.1.2\frameworks\esp-idf-v5.1.2"
set "IDF_TOOLS_PATH=D:\esp32-idf-ahy\5.1.2"

REM 切换到项目目录
cd /d D:\esp_test_code\project\sample_project

echo [1/7] 设置ESP-IDF环境...
call "%IDF_PATH%\export.bat" >nul 2>&1
echo      完成！
echo.

echo [2/7] 关闭占用COM8的进程...
taskkill /F /IM python.exe >nul 2>&1
timeout /t 1 >nul
echo      完成！
echo.

echo [3/7] 完全清理旧的编译文件...
idf.py fullclean
if errorlevel 1 (
    echo      [错误] 清理失败！
    pause
    exit /b 1
)
echo      完成！
echo.

echo [4/7] 重新编译项目（可能需要1-2分钟）...
idf.py build
if errorlevel 1 (
    echo      [错误] 编译失败！
    pause
    exit /b 1
)
echo      完成！
echo.

echo [5/7] 擦除Flash（删除所有旧数据）...
idf.py -p COM8 erase-flash
if errorlevel 1 (
    echo      [错误] 擦除失败！
    pause
    exit /b 1
)
echo      完成！
echo.

echo [6/7] 烧录新固件...
idf.py -p COM8 flash
if errorlevel 1 (
    echo      [错误] 烧录失败！
    pause
    exit /b 1
)
echo      完成！
echo.

echo ================================================
echo 烧录完成！
echo ================================================
echo.
echo 关键验证点：
echo 1. 编译时间应该是新的（不是 Nov 7 2025 10:31:36）
echo 2. 必须看到: [TEST] Starting GPIO4 basic test...
echo 3. 必须看到: GPIO_TEST: ========== 开始GPIO4基础测试 ==========
echo.
echo [7/7] 按任意键启动串口监视器...
pause >nul

echo.
echo 正在打开串口监视器（按 Ctrl+] 退出）...
echo.
idf.py -p COM8 monitor
