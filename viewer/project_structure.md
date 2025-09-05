# P2P视频加速器 - Viewer项目结构

```
p2p-video-viewer/
├── backend/                          # Python后端
│   ├── app/
│   │   ├── __init__.py
│   │   ├── main.py                   # FastAPI应用入口
│   │   ├── config.py                 # 配置文件
│   │   ├── models/                   # 数据模型
│   │   │   ├── __init__.py
│   │   │   ├── video.py              # 视频相关模型
│   │   │   ├── peer.py               # Peer相关模型
│   │   │   ├── history.py            # 历史记录模型
│   │   │   └── settings.py           # 设置模型
│   │   ├── api/                      # API路由
│   │   │   ├── __init__.py
│   │   │   ├── videos.py             # 视频相关API
│   │   │   ├── search.py             # 搜索API
│   │   │   ├── peers.py              # Peer管理API
│   │   │   ├── history.py            # 历史记录API
│   │   │   └── settings.py           # 设置API
│   │   ├── services/                 # 业务逻辑层
│   │   │   ├── __init__.py
│   │   │   ├── video_service.py      # 视频服务
│   │   │   ├── search_service.py     # 搜索服务
│   │   │   ├── peer_service.py       # Peer服务
│   │   │   ├── history_service.py    # 历史记录服务
│   │   │   └── settings_service.py   # 设置服务
│   │   └── data/                     # 模拟数据存储
│   │       ├── __init__.py
│   │       ├── mock_data.py          # 模拟数据定义
│   │       ├── video_data.py         # 视频数据
│   │       ├── peer_data.py          # Peer数据
│   │       ├── history_data.py       # 历史记录数据
│   │       └── settings_data.py      # 设置数据
│   ├── requirements.txt              # Python依赖
│   ├── main.py                       # 启动脚本
│   └── README.md                     # 后端说明文档
│
├── frontend/                         # Vue.js前端
│   ├── public/
│   │   ├── index.html
│   │   └── favicon.ico
│   ├── src/
│   │   ├── main.js                   # Vue应用入口
│   │   ├── App.vue                   # 根组件
│   │   ├── router/                   # 路由配置
│   │   │   ├── index.js
│   │   │   └── routes.js
│   │   ├── views/                    # 页面组件
│   │   │   ├── HomePage.vue          # 主页面 (/index)
│   │   │   ├── SearchPage.vue        # 搜索结果页面 (/search)
│   │   │   ├── VideoPage.vue         # 视频播放页面 (/video)
│   │   │   ├── SettingsPage.vue      # 设置页面 (/settings)
│   │   │   ├── PeerPage.vue          # Peer管理页面 (/peer)
│   │   │   └── HistoryPage.vue       # 历史记录页面 (/history)
│   │   ├── components/               # 通用组件
│   │   │   ├── SearchBar.vue         # 搜索栏组件
│   │   │   ├── VideoPlayer.vue       # 视频播放器组件
│   │   │   ├── PeerTable.vue         # Peer表格组件
│   │   │   ├── HistoryList.vue       # 历史记录列表组件
│   │   │   ├── SettingsForm.vue      # 设置表单组件
│   │   │   ├── ToggleSwitch.vue      # 开关组件
│   │   │   ├── NumberInput.vue       # 数字输入组件
│   │   │   └── StatusBar.vue         # 状态栏组件
│   │   ├── services/                 # API服务
│   │   │   ├── api.js                # API基础配置
│   │   │   ├── videoApi.js           # 视频相关API
│   │   │   ├── searchApi.js          # 搜索API
│   │   │   ├── peerApi.js            # Peer管理API
│   │   │   ├── historyApi.js         # 历史记录API
│   │   │   └── settingsApi.js        # 设置API
│   │   ├── utils/                    # 工具函数
│   │   │   ├── constants.js          # 常量定义
│   │   │   ├── formatters.js         # 格式化工具
│   │   │   └── validators.js         # 验证工具
│   │   ├── assets/                   # 静态资源
│   │   │   ├── css/
│   │   │   │   ├── main.css          # 主样式
│   │   │   │   ├── components.css    # 组件样式
│   │   │   │   └── variables.css     # CSS变量
│   │   │   └── images/
│   │   │       ├── logo.png
│   │   │       └── placeholder.jpg   # 占位图片
│   │   └── store/                    # Vuex状态管理
│   │       ├── index.js
│   │       ├── modules/
│   │       │   ├── video.js          # 视频状态
│   │       │   ├── search.js         # 搜索状态
│   │       │   ├── peer.js           # Peer状态
│   │       │   ├── history.js        # 历史记录状态
│   │       │   └── settings.js       # 设置状态
│   ├── package.json                  # 前端依赖
│   ├── vue.config.js                 # Vue配置
│   └── README.md                     # 前端说明文档
│
├── docs/                             # 项目文档
│   ├── api.md                        # API文档
│   ├── deployment.md                 # 部署说明
│   └── development.md                # 开发指南
│
├── scripts/                          # 脚本文件
│   ├── start_backend.sh             # 启动后端脚本
│   ├── start_frontend.sh             # 启动前端脚本
│   └── build.sh                      # 构建脚本
│
├── docker/                           # Docker配置
│   ├── Dockerfile.backend            # 后端Dockerfile
│   ├── Dockerfile.frontend           # 前端Dockerfile
│   └── docker-compose.yml            # Docker Compose配置
│
├── .gitignore                        # Git忽略文件
├── README.md                         # 项目总说明
└── requirements.txt                  # 项目依赖总览
```

## 技术栈说明

### 后端技术栈
- **框架**: FastAPI (Python)
- **数据存储**: 内存变量 (模拟数据)
- **API文档**: 自动生成 (FastAPI自带)
- **CORS**: 支持跨域请求

### 前端技术栈
- **框架**: Vue.js 3.x
- **路由**: Vue Router 4.x
- **状态管理**: Vuex 4.x
- **HTTP客户端**: Axios
- **UI组件**: 自定义组件
- **构建工具**: Vue CLI

## 主要功能模块

### 1. 视频管理模块
- 视频搜索和播放
- 播放进度管理
- 实时状态监控

### 2. Peer管理模块
- Peer节点列表
- 在线状态监控
- 速度统计
- 启用/禁用控制

### 3. 历史记录模块
- 播放历史记录
- 进度恢复
- 缓存状态显示

### 4. 设置管理模块
- P2P服务开关
- 缓存上限设置
- 上传速度限制
- 多线程配置

## API接口设计

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

## 开发环境要求

### 后端环境
- Python 3.8+
- FastAPI
- uvicorn
- pydantic

### 前端环境
- Node.js 16+
- npm 或 yarn
- Vue CLI

## 启动方式

### 开发环境
```bash
# 启动后端
cd backend
python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000

# 启动前端
cd frontend
npm install
npm run serve
```

### 生产环境
```bash
# 使用Docker Compose
docker-compose up -d
```





