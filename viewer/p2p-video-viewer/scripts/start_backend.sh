#!/bin/bash

# 启动P2P视频加速器后端服务

echo "启动P2P视频加速器后端服务..."

# 进入后端目录
cd backend

# 检查Python环境
if ! command -v python3 &> /dev/null; then
    echo "错误: 未找到Python3，请先安装Python3"
    exit 1
fi

# 检查虚拟环境
if [ ! -d "venv" ]; then
    echo "创建虚拟环境..."
    python3 -m venv venv
fi

# 激活虚拟环境
source venv/bin/activate

# 安装依赖
echo "安装依赖..."
pip install -r requirements.txt

# 启动服务
echo "启动FastAPI服务..."
python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000





