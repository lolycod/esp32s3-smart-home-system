#!/bin/bash

echo "正在启动智能终端管理系统..."
echo ""

# 检查Node.js是否安装
if ! command -v node &> /dev/null; then
    echo "❌ 错误: Node.js未安装或未配置环境变量"
    echo "请先安装Node.js (版本 >= 14.0.0)"
    echo "下载地址: https://nodejs.org/"
    exit 1
fi

echo "✅ Node.js 已安装: $(node --version)"
echo ""

# 检查依赖是否安装
if [ ! -d "server/node_modules" ]; then
    echo "📦 正在安装依赖..."
    cd server
    npm install
    if [ $? -ne 0 ]; then
        echo "❌ 依赖安装失败"
        exit 1
    fi
    cd ..
fi

echo "✅ 依赖已安装"
echo ""
echo "🚀 正在启动服务器..."
echo "📖 启动后请访问: http://localhost:8080"
echo "⚠️  按 Ctrl+C 停止服务器"
echo ""

cd server
npm start

if [ $? -ne 0 ]; then
    echo "❌ 服务器启动失败"
    exit 1
fi