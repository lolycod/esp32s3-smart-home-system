@echo off
echo 正在启动智能终端管理系统...
echo.

REM 检查Node.js是否安装
node --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误: Node.js未安装或未配置环境变量
    echo 请先安装Node.js (版本 >= 14.0.0)
    echo 下载地址: https://nodejs.org/
    pause
    exit /b 1
)

echo ✅ Node.js 已安装
echo.

REM 检查依赖是否安装
if not exist "server\node_modules" (
    echo 📦 正在安装依赖...
    cd server
    npm install
    if %errorlevel% neq 0 (
        echo ❌ 依赖安装失败
        pause
        exit /b 1
    )
    cd ..
)

echo ✅ 依赖已安装
echo.
echo 🚀 正在启动服务器...
echo 📖 启动后请访问: http://localhost:8080
echo ⚠️  按 Ctrl+C 停止服务器
echo.

cd server
npm start

if %errorlevel% neq 0 (
    echo ❌ 服务器启动失败
    pause
    exit /b 1
)