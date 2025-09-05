# P2P视频加速器 - Viewer

一个基于P2P技术的视频加速器Web前端应用，提供视频搜索、播放、Peer管理等功能。

## 项目结构

```
p2p-video-viewer/
├── backend/          # Python FastAPI后端
├── frontend/          # Vue.js前端
├── docs/             # 项目文档
├── scripts/          # 启动脚本
└── docker/           # Docker配置
```

## 技术栈

### 后端
- **Python 3.8+**
- **FastAPI** - 现代、快速的Web框架
- **Pydantic** - 数据验证
- **Uvicorn** - ASGI服务器

### 前端
- **Vue.js 3.x** - 渐进式JavaScript框架
- **Vue Router 4.x** - 官方路由管理器
- **Vuex 4.x** - 状态管理模式
- **Axios** - HTTP客户端

## 功能特性

### 🎬 视频管理
- 视频搜索和播放
- 播放进度管理
- 实时状态监控

### 🌐 Peer管理
- Peer节点列表
- 在线状态监控
- 速度统计
- 启用/禁用控制

### 📚 历史记录
- 播放历史记录
- 进度恢复
- 缓存状态显示

### ⚙️ 设置管理
- P2P服务开关
- 缓存上限设置
- 上传速度限制
- 多线程配置

## 快速开始

### 环境要求

- Python 3.8+
- Node.js 16+
- npm 或 yarn

### 安装和运行

1. **克隆项目**
```bash
git clone <repository-url>
cd p2p-video-viewer
```

2. **启动后端**
```bash
# 使用脚本启动
./scripts/start_backend.sh

# 或手动启动
cd backend
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

3. **启动前端**
```bash
# 使用脚本启动
./scripts/start_frontend.sh

# 或手动启动
cd frontend
npm install
npm run serve
```

4. **访问应用**
- 前端: http://localhost:8080
- 后端API: http://localhost:8000
- API文档: http://localhost:8000/docs

## 页面说明

### 主页面 (/)
- 类似Bing首页的搜索界面
- 中央搜索栏
- 功能按钮：设置、Peer管理、历史记录

### 搜索结果页面 (/search)
- 显示搜索结果列表
- 每个结果包含标题、描述、缩略图
- 点击跳转到视频播放页面

### 视频播放页面 (/video)
- 类似B站视频播放界面
- 简洁的控制栏
- 实时状态显示：节点数、下行速度、上行速度

### 设置页面 (/settings)
- 简洁的设置界面
- 支持布尔型和数值型设置
- 实时保存设置

### Peer管理页面 (/peer)
- 显示Peer节点列表
- 实时更新节点状态
- 支持启用/禁用控制

### 历史记录页面 (/history)
- 播放历史列表
- 显示播放进度和缓存状态
- 支持从记录位置继续播放

## API接口

### 视频相关
- `GET /api/videos/search` - 搜索视频
- `GET /api/videos/{rid}` - 获取视频详情
- `GET /api/videos/{rid}/stream` - 视频流

### Peer管理
- `GET /api/peers` - 获取Peer列表
- `PUT /api/peers/{pid}/toggle` - 切换Peer状态
- `GET /api/peers/stats` - 获取Peer统计

### 历史记录
- `GET /api/history` - 获取历史记录
- `POST /api/history` - 添加历史记录
- `DELETE /api/history/{id}` - 删除历史记录

### 设置
- `GET /api/settings` - 获取设置
- `PUT /api/settings` - 更新设置

## 开发指南

### 后端开发
```bash
cd backend
# 安装开发依赖
pip install -r requirements.txt
# 启动开发服务器
python -m uvicorn app.main:app --reload
```

### 前端开发
```bash
cd frontend
# 安装依赖
npm install
# 启动开发服务器
npm run serve
# 构建生产版本
npm run build
```

## 部署

### Docker部署
```bash
# 构建并启动所有服务
docker-compose up -d

# 查看服务状态
docker-compose ps

# 查看日志
docker-compose logs -f
```

### 手动部署
1. 构建前端
```bash
cd frontend
npm run build
```

2. 配置后端
```bash
cd backend
# 修改配置文件
# 启动生产服务器
python -m uvicorn app.main:app --host 0.0.0.0 --port 8000
```

## 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开 Pull Request

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 Issue
- 发送邮件
- 项目讨论区





