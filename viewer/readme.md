# viewer

写一个web前端，需求如下

## 技术栈要求
后端使用python，具有搭建简易的优点
前端使用什么都行，最好使用主流的技术框架比如vue，但不要使用过于新的技术栈或者特性，一切从简。

## 页面展示需求及其跳转关系

页面：主页面
页面路径：/index
外观描述：
类似于bing.com的首页，中间一个大大的搜索栏；
在搜索栏的右方有搜索按钮。点击后跳转搜索结果页面。
搜索栏下方一行内有多个按钮，所有按钮在一起对于一行内是居中的，存在的按钮如下，
设置按钮，点击后跳转设置页面；
peer管理按钮，点击后跳转peer管理页面；
历史记录按钮，点击后跳转历史记录页面；


页面：搜索结果页面
页面路径：/search?rid=xxx
外观描述：
类似bing搜索的结果页面，顶部一个搜索栏，里面还有着刚刚搜索的内容。
搜索栏下面是搜索的结果，每个条目占一个矩形框的面积。
一个条目是左右结构，左边上下是标题和描述两个部分组成，右侧是一张正方形图片。
点击条目后跳转到视频播放页面


页面：设置页面
页面路径：/settings
外观描述：
类似浏览器的设置界面，但是更简洁。
只有若干行的设置条目。
条目分两种类型，bool型和数值型。
bool型，一行的左侧是条目名，右侧是一个经典的横向开关控件。
浮点型，左侧是条目名，右侧是一个数值设置框，可以输入小数，但不能输入除了数字之外的东西。
整数型，只能输入整数，其他和浮点型一致
我需要这样几条设置，我把类型和条目名写出来
bool：开启p2p服务
浮点：本地缓存上限
浮点：上传速度上限
整数：多线程数量


页面：视频播放页面
页面路径：/video?rid=xxx
外观描述：
类似哔哩哔哩的视频播放页面。
底部的控制栏更简洁，分上下结构，上面是一个支持拖拽的长进度条。
下面一行最左侧有个暂停/播放的二合一按钮，但并没有上一条下一条视频的按钮，靠右侧是几条实时更新的状态展示，类似vscode的界面右下角。
每一条状态信息是键值对的形式，类似“下行(Mb):3.2”这样，冒号左边是一个键，右边是值。两条之间用两个空格隔开。
我需要这样几条状态的值实时更新(0.5s一次)：“节点数:5  下行(MB/s):3.2  上行(KB/s):32.0”


页面：peer管理页面
页面路径：/peer
描述：
分为上下结构；
上面是一个固定的顶栏，键值对形式展示一些信息“自身NAT穿透类型：xxx  总节点数：5”
下面是一个列表，包含很多行，每一行都是一个条目，可以上下滚动。
在列表的顶部还有一行表头，里面有几项字段名：“pid 在线 下行速度(MB/s) 上行速度(KB/s) 总下行(MB) 总上行(KB) 启用”
表头之下，每个条目按照表头内容依次展示，除开特别项，每个项都是字符串形式展示，每秒更新。
对于“启用”项，展示为一个经典的开关，点击可以对其开关。


页面：历史记录页面
页面路径：/history
描述：
整个页面是一个列表，类似bing搜索结果页面，但是没有搜索框。
列表内每一个条目分为左右结构。
左侧分上下结构，上面是粗体标题，下面是键值对信息展示,一行至多两个键值对信息“时间：2025/6/1 19:32  大小(MB):3.2” “播放进度%：53  缓存存在：是” “平均下行(MB/s): 3.2”
点击条目可再次进入视频播放页面，并且从记录的播放进度继续播放。


## 数据处理需求

在页面中涉及到的数据操作，使用js实现。

所有的请求，在后端即python端的代码实现都要接口明确。

涉及到的数据操作，不要使用数据库，而是在python里先写死成变量={..}类似的形式，我后期再对数据来源和流向做单独的处理。

请求的url或者参数、表单名之类的，大体上符合我给的页面展示要求，细致的你自己设计。

## 项目录结构要求

前后端结构清晰，模块划分清晰。

## 运行方法

以下步骤基于本项目内置的模拟数据与接口，可直接启动联调。

### 后端（FastAPI）
1. 安装依赖（如缺少 `venv`，请先安装：`sudo apt install -y python3-venv`）
```bash
cd viewer/p2p-video-viewer/backend
[ -d venv ] || python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```
2. 启动服务（单进程，避免端口冲突）：
```bash
# 如端口被占用可先清理
fuser -k 8000/tcp 2>/dev/null || true
pkill -f "uvicorn app.main:app" 2>/dev/null || true

# 启动
python -m uvicorn app.main:app --host 127.0.0.1 --port 8000

# 后台启动（不加 --reload，避免热更新多进程导致端口占用）
nohup python -m uvicorn app.main:app --host 127.0.0.1 --port 8000 > uvicorn.log 2>&1 & echo $! > uvicorn.pid
sleep 2

```
3. 健康检查（应返回 {"status":"healthy"}：
```bash
curl -sS http://127.0.0.1:8000/health
```
- API 文档：`http://127.0.0.1:8000/docs`
- 主要接口：`/api/search`、`/api/videos/{rid}`、`/api/peers`、`/api/history`、`/api/settings`

### 前端（Vue 3）
1. 安装 Node.js（建议 Node 18）：
```bash
# 推荐使用 NodeSource（如需 sudo）
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs
```
2. 安装并启动前端：
```bash
cd viewer/p2p-video-viewer/frontend
npm install
npx vue-cli-service serve --port 8080
```
3. 访问前端：
- 前端开发服：`http://localhost:8080`
- 已配置代理：`/api` → `http://127.0.0.1:8000`

### 常见问题
- ESLint 报 “No ESLint configuration found”：已在前端添加 `.eslintrc.js` 与 `.eslintignore`，并设置 `lintOnSave: false`，若仍有冲突可执行：
```bash
rm -rf node_modules package-lock.json
npm install
```
- 8000 端口占用：
```bash
fuser -k 8000/tcp 2>/dev/null || true
pkill -f "uvicorn app.main:app" 2>/dev/null || true
```
- 播放页示例视频：为便于联调，后端返回可公开访问的示例视频流地址。
