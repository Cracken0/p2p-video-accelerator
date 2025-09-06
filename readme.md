# p2p-video-accelerator


运行手册

一、总体说明
本项目提供“前端 Viewer（Vue3）+ 后端 API（FastAPI）+ C++ 服务层（P2P/Store/NAT/TCP-Like）+ 进程间中转器（Repeater UDS）”的端到端原型。推荐在本机开发环境直接联调：后端默认监听 127.0.0.1:8000，前端开发服默认 8080，Repeater 的 Unix 套接字为 /tmp/p2p_repeater.sock。

二、环境依赖
1) Linux（WSL2 可）与 CMake ≥ 3.16，g++/clang++，OpenSSL（Crypto）
2) Python 3.10+，pip，python3-venv
3) Node.js 18+ 与 npm

三、构建与启动
1) 构建 C++ 服务
```bash
cmake -S /home/cracken/project/p2p-video-accelerator/service -B /home/cracken/project/p2p-video-accelerator/service/build -DCMAKE_BUILD_TYPE=Release
cmake --build /home/cracken/project/p2p-video-accelerator/service/build -j
```
生成的可执行文件：/home/cracken/project/p2p-video-accelerator/service/build/bin/service_main

2) 启动 Repeater（C++ 服务）
```bash
/home/cracken/project/p2p-video-accelerator/service/build/bin/service_main >/tmp/repeater.log 2>&1 & echo $! > /tmp/repeater.pid
```
默认在 /tmp/p2p_repeater.sock 创建 UDS 套接字。

3) 启动后端（FastAPI）
```bash
cd /home/cracken/project/p2p-video-accelerator/viewer/p2p-video-viewer/backend
[ -d venv ] || python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python -m uvicorn app.main:app --host 127.0.0.1 --port 8000
```
健康检查：http://127.0.0.1:8000/health 返回 {"status":"healthy"}

4) 启动前端（Vue3）
```bash
cd /home/cracken/project/p2p-video-accelerator/viewer/p2p-video-viewer/frontend
npm install
npx vue-cli-service serve --port 8080
```
访问前端：http://localhost:8080 （已配置 /api 代理至 127.0.0.1:8000）

四、模块协作关系
前端通过 REST 访问后端 API；后端根据需要通过 Repeater 客户端访问 /tmp/p2p_repeater.sock，从 C++ 内核读取运行指标或下发控制；C++ 内核由 P2P、NAT-Traversal、TCP-Like 与 Store 组成，Store 负责本地切片缓存。

五、常见问题
1) 8000 端口被占用：
```bash
fuser -k 8000/tcp 2>/dev/null || true
pkill -f "uvicorn app.main:app" 2>/dev/null || true
```
2) Node 依赖冲突：
```bash
rm -rf node_modules package-lock.json && npm install
```
3) Repeater 未启动：确认 /tmp/p2p_repeater.sock 是否存在，或重新启动 service_main。

