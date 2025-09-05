#!/bin/bash

# 启动P2P视频加速器前端服务

echo "启动P2P视频加速器前端服务..."

# 进入前端目录
cd frontend

# 检查Node.js环境
if ! command -v node &> /dev/null; then
    echo "错误: 未找到Node.js，请先安装Node.js"
    exit 1
fi

# 检查npm
if ! command -v npm &> /dev/null; then
    echo "错误: 未找到npm，请先安装npm"
    exit 1
fi

# 安装依赖
echo "安装依赖..."
npm install

# 启动开发服务器
echo "启动Vue开发服务器..."
npm run serve





