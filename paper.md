3. 系统设计

本系统旨在构建一套面向视频分发场景的端到端 P2P 加速体系，提供从前端播放、后端编排到本地节点传输及资源管理的完整闭环。系统总体采用“浏览器前端 + Python Web 后端 + C++ P2P 服务 + 进程间中转器”的分层解耦架构，通过 Unix Domain Socket 的轻量 IPC 统一内外部数据面，结合 NAT 穿透与基于 UDP 的可靠传输子系统，形成低时延、可扩展的混合内容分发路径。

系统总体结构如下所示：

```mermaid
graph LR
  A[前端 Viewer<br/>Vue3] -- REST/JSON --> B[后端 API<br/>FastAPI]
  B -- Repeater 客户端 --> C[IPC 中转器<br/>Unix Socket]
  C -- Getter/Setter 协议 --> D[P2P 服务<br/>C++]
  D -- 片段缓存/读写 --> E[资源文件管理<br/>FileStore]
  D -- NAT 穿透/连接管理 --> F[NAT Traversal]
  D -- 可靠传输/拥塞控制 --> G[UDP Reliable]
```

数据与控制流程（简化）如下：

```mermaid
sequenceDiagram
  participant UI as Viewer 前端
  participant API as FastAPI 后端
  participant REP as Repeater(IPC)
  participant P2P as C++ P2P 服务
  participant FS as FileStore

  UI->>API: /api/search?q=...
  API-->>UI: 搜索结果(JSON)
  UI->>API: /api/videos/{rid}/stream
  API->>REP: get("stream_url:{rid}")
  REP->>P2P: Getter("stream_url:{rid}")
  P2P-->>REP: url/调度信息
  REP-->>API: url
  API-->>UI: 播放地址/元信息
  UI->>API: /api/peers /api/settings ...
  API->>REP: get/set(状态/配置)
  REP->>P2P: 同步至内核
  P2P->>FS: append/read 切片
  P2P-->>UI: 通过拉流/本地缓存实现播放加速
```

3.2 功能模块设计
	自顶向下；

在功能模块设计方面，本文采用自顶向下的方法论以确保系统复杂性在分层与分域的过程中得到可控化管理。自顶向下的设计首先从整体目标与性能指标出发，明确系统在端到端时延、卡顿率、吞吐量与可运维性等维度的约束，然后逐层细分为前端交互、后端编排、进程间中转、对等通信、可靠传输与资源管理等子域，以职责单一、接口清晰为原则进行边界划定。这种方法的直接收益在于：其一，便于在需求演进过程中对具体子域进行局部替换或增强，从而降低全局级的耦合与重构成本；其二，能够以接口为契约对上层屏蔽底层实现细节，通过稳定的 API 与字段化的中转协议实现跨语言与跨技术栈的协作；其三，适配“先可用、后增强”的工程节奏，系统在初期可由后端内置模拟数据保障演示与测试，在中期逐步切换到由中转器连接的 C++ P2P 内核，在后期再引入更复杂的调度、穿透与可靠传输策略，从而实现能力渐进式提升与风险可控的迭代路径。

从控制视角看，自顶向下的分解使得策略与机制得以分离：后端负责策略编排与对外接口语义，P2P 内核则聚焦机制层的连接管理、片段调度与链路控制，而二者之间通过中转器以字段命名的形式达成松耦合的契约。这种契约化的接口设计不仅强化了模块之间的可替换性，还为观测与调试提供了天然的插桩点，任何新的观测或控制需求都可以在不破坏既有路径的前提下通过新增字段予以承载。从数据视角看，系统遵循“控制面轻量、数据面高效”的分层原则：控制面在 REST 与 IPC 之间传递结构化的少量元数据，而数据面则在 P2P 与本地缓存之间以流与块的形式高效交换。下文将分别围绕用户界面服务、进程间数据中转、点对点通信、资源文件管理、NAT 穿透以及基于 UDP 的可靠通信展开，重点阐述其设计动机、结构组织与与其他模块的协作关系。

为便于读者从宏观上把握模块间的依赖与作用域划分，本文给出功能域关系图。图中以层次化方式展现了“交互呈现—服务编排—协议中转—网络传输—存储持久化”的五级结构，各层通过稳定接口衔接，并以观测点贯穿全局以支撑运行时可见性与故障定位。

```mermaid
graph TD
  UI[用户界面层] --> API[服务编排层]
  API --> IPC[协议中转层]
  IPC --> P2P[对等网络层]
  P2P --> UDP[可靠传输子层]
  P2P --> FS[缓存与持久化层]
  classDef a fill:#eef,stroke:#88a
  class UI,API,IPC,P2P,UDP,FS a
```

3.1.1 用户界面服务设计
	
用户界面服务的设计目标在于以一致的交互范式承载复杂的后端能力，同时对网络状态的瞬态变化提供可理解的反馈，从而在不增加用户心智负担的前提下达成对系统“可感知—可调节—可追溯”的整体体验。基于此目标，界面以单页应用的形式组织，采用路由驱动的页面切换与组件化的视图复用，将“入口检索—结果筛选—视频播放—运行观测—历史追溯—参数配置”的主流程进行结构化分解。在该流程中，首页承担用户意图采集与导航汇聚的职责，搜索页面以信息密度与可读性之间的折中实现高效的结果对比，播放页面在传统控制功能之上强调运行态指标的周期性呈现，Peer 管理页面将局部拓扑与节点状态进行表格式归纳，历史页面把观测数据与使用行为关联以支持恢复性播放，设置页面则以“显式参数—即时生效”的模式形成用户与系统之间的契约。上述页面通过统一的服务层访问后端 API，并在异常情况下采用提示与降级策略保证交互连续性。

从数据流角度，用户界面服务作为控制面流量的起点与终点，主要负责元数据的采集、展示与变更提交。请求在前端聚合并经由代理转发至后端，响应数据在前端进行轻量格式化后渲染，并在播放场景下以固定周期请求运行态指标；当用户修改设置或对节点进行启停时，前端生成带有最小必要字段的变更请求，以避免对后端造成过度耦合或状态漂移。为了保证不同页面与组件间的一致性，前端在状态管理层投入最小限度的共享状态，仅存储页面级缓存与关键运行指标，其他数据均采用即取即用的懒加载策略，降低了在网络抖动场景下的状态不一致风险。

为了明确用户界面层与后端的交互逻辑，本文提供检索与播放主流程的高层交互示意。图中展现了请求的阶段性分解与对异常的容错路径，读者可据此理解前端如何在时变网络条件下维持交互体验的一致性。

```mermaid
sequenceDiagram
  participant U as 用户
  participant V as Viewer(前端)
  participant A as API(后端)
  participant R as Repeater(IPC)
  participant N as P2P 内核

  U->>V: 输入检索关键词
  V->>A: GET /api/search?q=...
  A-->>V: 返回结果列表
  U->>V: 选择条目进入播放
  V->>A: GET /api/videos/{rid}/stream
  A->>R: get("stream_url:{rid}") 或使用模拟数据兜底
  R->>N: Getter("stream_url:{rid}")
  N-->>R: 返回调度/地址信息
  R-->>A: 统一封装返回
  A-->>V: 前端获得播放地址/元信息
  loop 周期性状态刷新
    V->>A: GET /api/peers /api/settings /api/videos/{rid}/status
    A->>R: 聚合 get 字段
    R->>N: 取运行时指标
    N-->>R: 返回指标
    R-->>A: 返回聚合数据
    A-->>V: 渲染状态栏
  end
```

3.1.2 进程间数据中转传输
	
进程间数据中转器的设计立足于跨语言、跨进程的弱耦合协作需求，通过“字段化—协议最小化—契约稳定化”的方法确立接口边界与演进路径。首先，字段化意味着系统将观测与控制能力统一抽象为键值对的读写操作，避免业务特定语义渗入到传输层，从而降低前后端同时变更的概率。其次，协议最小化体现在报文头仅包含操作类型、字段名与负载长度，消息体则保持为原始字节流以留出最大自由度给上层编码策略。这一“薄协议”策略使得中转器的实现与调试维持在较低复杂度，同时也方便了字段的增删与兼容兼容性策略的实施。最后，契约稳定化通过字段命名域与版本演进约定得以保障，诸如 peer、net、cfg、video 等前缀划分了观测与控制的语义空间，使得新增能力以增量方式接入而不破坏既有消费者。

在交互路径上，Python 侧使用统一的 `_send(func, field, data)` 发起 getter 或 setter 请求，C++ 侧通过字段注册中心将具体的读取与写入处理函数与字段名绑定，从而形成“调用—分发—处理—回送”的闭环。为进一步阐明字段层的组织，本文给出字段关系与生命周期图：

```mermaid
graph LR
  subgraph 命名域
    A[cfg.* 配置项]-->B[影响内核调度/传输参数]
    C[peer.* 节点统计]-->D[驱动 UI 状态可视化]
    E[net.* 链路指标]-->F[提供拥塞/切换决策证据]
    G[video.* 内容信息]-->H[关联播放与缓存语义]
  end
  subgraph 流程
    UI[FastAPI 调用]-->IPC[Repeater 客户端]
    IPC-->SRV[Repeater 服务器]
    SRV-->REG[FieldRegistry 分发]
    REG-->CORE[P2P 内核字段处理]
    CORE-->|响应|SRV
    SRV-->|封装|IPC
    IPC-->|返回|UI
  end
```

该设计的核心在于将系统内通用的观测与控制语义沉淀为字段层接口，使服务编排与网络内核的迭代相互独立。随着系统发展，字段可通过版本前缀、可选值与回退值等机制实现平滑兼容；在灰度阶段，后端能够按字段粒度进行开关控制与日志追踪，显著降低线上风险与定位成本。

3.1.3 点对点通信模块
	
点对点通信模块面向“高吞吐—低时延—强鲁棒”的综合目标构建，其设计强调调度策略与传输机制的松耦合以及与观测体系的紧密结合。节点接入网络后，首先依据信令与历史观测获取候选邻居集，并以带宽估计、往返时延与丢包率等指标构建链路质量画像，在此基础上确定初始并发连接数与请求窗口大小。内容被划分为细粒度的分片并以位图表示拥有情况，调度器综合稀缺度、热点程度与播放时序对请求进行排序，优先保障近端播放时点所需的即时片段，同时利用后台带宽回填冷片段以提升整体命中率。

当链路质量发生劣化，模块通过快速重试与备用路径切换控制不确定性传播的范围，并在必要时退避并回源拉流以保障用户体验。在此过程中，模块持续收集并输出包含下行吞吐、上行速率、连接成功率、平均 RTT、瞬时丢包等在内的关键指标，经由中转器上报后端，以支持状态展示与策略调参。下图展示了调度循环的抽象流程，强调了从观测到决策再到执行的闭环关系：

```mermaid
graph TD
  OBS[观测收集<br/>RTT/丢包/带宽]-->EST[链路估计与分级]
  EST-->PLAN[分片优先级规划<br/>稀缺度/热点/时序]
  PLAN-->ACT[请求下发与并发控制]
  ACT-->XFER[数据交换与校验]
  XFER-->FEED[即时反馈<br/>成功率/速率]
  FEED-->OBS
```

通过这种闭环机制，点对点通信模块在复杂网络条件下能够维持相对稳定的服务质量，并以观测驱动的方式实现对策略参数的渐进式自适应，从而在实践中兼顾了启动时延、卡顿率与平均画质等体验指标。

3.1.4 资源文件管理模块
	
资源文件管理模块承载切片缓存与持久化的职责，其设计遵循“元信息独立、顺序友好、淘汰可解释”的原则。首先，系统为每个缓存对象维护独立的元信息条目，记录其最近修改时间与占用空间，以便在崩溃恢复与离线统计场景下快速重建状态。其次，写入路径按照顺序追加优化磁盘访问模式，并在到达容量上限前通过预测性检查预留空间，避免在高并发场景下频繁触发淘汰。再次，淘汰策略采用以“最近修改时间”为主的近似 LRU，并允许在未来引入多维度权重（如命中频率、播放时序）以更好地贴合业务目标。并发控制方面，模块通过互斥保护元信息与打开句柄表，并以后台线程周期性回收空闲文件描述符，从而降低系统资源占用与句柄泄漏风险。

为直观呈现缓存行为与淘汰策略的互动，本文给出缓存写入与空间管理流程图。图中可以看到，模块在写入前进行容量检查，不足时触发基于元信息的逐文件淘汰，写入完成后再以原子方式更新元信息并刷新已用空间统计。

```mermaid
graph TD;
  WREQ([写入请求 data-sid]);
  CHK{容量检查};
  APP[[顺序追加写入]];
  EVICT[[基于最近修改时间淘汰]];
  META([更新元信息 时间戳 大小]);
  USED([刷新已用空间]);
  IDLE([后台线程回收空闲fd]);

  WREQ --> CHK;
  CHK --> APP;
  CHK --> EVICT;
  EVICT --> CHK;
  APP --> META;
  META --> USED;
  USED --> IDLE;
```

上述设计保证了缓存层在长时间运行与高并发场景下的可预测行为与稳定吞吐，并以可解释的策略为上层播放与回源逻辑提供可靠支撑。随着系统演进，还可以在不改变接口的前提下，将淘汰策略扩展为基于命中统计与播放时序的多目标优化模型。

3.1.5 NAT穿透模块
	
NAT 穿透模块针对家庭宽带与移动网络环境中普遍存在的内网地址转换问题，通过类型识别、候选地址收集与打洞/保活等手段提升端到端可达性与路径质量。模块首先对自身 NAT 类型进行分类，并结合可选的信令或中继组件执行外网可达性测试，借此构建可选传输路径集合。随后，模块依据时延、抖动与丢包等指标对候选路径进行排序，并在 P2P 调度层发起连接时优先选择预估质量更优的直连路径；当直连失败或质量退化时，系统回退至次优路径并维持连接的活性，以降低切换对上层业务的冲击。与此同时，模块将 NAT 类型、打洞进度与当前可用路径等信息以字段形式上报，为后端的状态可视化与策略调整提供依据。

穿透流程的高层时序见下图。该图展示了从类型识别到候选收集，再到路径探测与选择、保活与回退的整体链路。

```mermaid
sequenceDiagram
  participant C as Client(Node)
  participant S as 信令/中继(可选)
  participant P as 目标对等端

  C->>C: 自身 NAT 类型识别
  C->>S: 请求可达性测试/候选地址
  S-->>C: 返回候选信息
  par 路径探测
    C->>P: 探测包(多端口/多地址)
    P-->>C: 响应包(选择最佳)
  end
  C->>C: 选择直连/回退策略
  loop 保活
    C->>P: keepalive
    P-->>C: ack/统计
  end
```

该设计以最小假设达成最大兼容，在缺乏公网 Peer 的场景下仍然能够通过回退路径提供基础可达性；在公网 Peer 充分的场景下，则优先选择时延与抖动优势明显的直连路径，以提升整体体验指标。

3.1.6 基于UDP的可靠通信
	
基于 UDP 的可靠通信子系统在协议层为数据面提供序号、有序重组、重传与拥塞控制能力，使上层得以在不牺牲低延迟特性的前提下获得稳定吞吐。设计上，子系统以滑动窗口作为核心控制结构，通过对往返时延的在线估计来设置超时阈值，并在重传触发条件下采用快速重传与超时重传的组合策略，以兼顾恢复速度与带宽效率。为避免小包过多造成协议头与系统调用开销的放大，子系统在满足 MTU 约束的前提下对小包进行聚合，同时为大块数据进行合理分片，确保链路利用率最大化。拥塞控制方面，子系统借鉴慢启动与拥塞避免的经典思想，在无拥塞阶段指数增长窗口以快速探测可用带宽，在拥塞事件发生时按比例退避，并以加性增大实现平滑逼近网络极限。

为了呈现传输状态与控制转换的逻辑，本文给出简化的发送端状态机图。该状态机强调了窗口大小与重传计时器的交互，以及对拥塞事件的响应方式。

```mermaid
graph TD
  IDLE[空闲]-->SS[慢启动]
  SS-->|rtt 正常/无丢包|CA[拥塞避免]
  SS-->|检测到丢包|FR[快速重传]
  CA-->|检测到拥塞|FR
  FR-->|完成恢复|CA
  CA-->|持续稳定|STEADY[稳态传输]
```

该子系统与调度层共享关键链路统计（如下行吞吐、上行速率、RTT 与丢包），并通过中转器对后端暴露可观测指标。在检测到拥塞与抖动上升时，上层调度能够主动降低并发或切换路径，从而以协同方式维持端到端的服务质量。整体而言，基于 UDP 的可靠通信在保障低延迟的同时提供了对丢包与拥塞的鲁棒性，使系统在复杂网络环境中获得可预期的性能边界。

---

```mermaid
%% ------------ 网络模块架构图（重新整理版） ------------
graph TD
    %% ---------- P2P 路由层 ----------
    subgraph P2P_Router["p2p::RouterTable 管理层"]
        RT([RouterTable])
        RT -.->|持有| SP[SuperPeer]
        RT -.->|在线表| PeerO[Peer]
        RT -.->|离线表| PeerInfoO[PeerInfo]
    end
```



---

```mermaid
%% ------------ 网络模块架构图（重新整理版） ------------
graph TD

    %% ---------- 可靠会话层 ----------
    subgraph REL["tcp_like::ReliableSession 层"]
        RS([ReliableSession])
        RS -.->|持有| US[UdpSocket]
        RS -.->|远端地址| EP[Endpoint]
    end

    %% ---------- UDP 套接字层 ----------
    subgraph UDP["tcp_like::UdpSocket 层"]
        US -.->|封装| FD[(fd_: int)]
        US -.->|收发地址| EP
    end

    %% ---------- 图例 ----------
    classDef core fill:#FFE4B5,stroke:#FF8C00;
    classDef sock  fill:#E0FFFF,stroke:#4682B4;
    classDef data  fill:#F0FFF0,stroke:#32CD32;

    class RT,RS core
    class US,EP sock
    class SP,PeerO,PeerInfoO data
```


---

4. 系统实现

4.1 用户界面与服务

4.1.1 搜索与查看

本系统的检索与观看路径围绕 `viewer/p2p-video-viewer` 前端与 `backend/app` 后端协作实现。前端通过 `src/services/searchApi.js` 与 `videoApi.js` 调用后端 `GET /api/search` 与 `GET /api/videos/{rid}`、`GET /api/videos/{rid}/stream` 接口，其中搜索接口由 `app/api/search.py` 代理至 `VideoService.search_videos()`，对模拟数据集中 `title/description` 字段执行小写包含式匹配，返回结构化的 `VideoSearchResult` 列表；视频详情与流信息分别由 `app/api/videos.py` 的 `get_video` 与 `get_video_stream` 提供，其中流信息在当前实现阶段返回公开可播示例 URL（`VideoService` 中的 `sample_url`），该设计确保了在 P2P 内核未集成前即可完成“端到端联调—页面验证—播放控件测试”的工程闭环。数据模型层由 `app/data/video_data.py` 提供，包含 `rid、title、description、thumbnail_url、duration、file_size` 等字段，后端以 Pydantic 模型进行出入参约束，强化接口一致性；前端则以路由驱动的页面切换组织“首页检索—结果列表—播放页”的用户流。为了保证抖动网络下的用户体验，前端采用固定周期的状态刷新策略，从后端获取播放状态与基础统计，以轻量化的键值对形式呈现。整体而言，此链路以稳定的 REST 语义与可替换的数据源为核心，既支撑了快速演示，又为后续切换到 P2P 数据面预留了充分的接口余量。

```mermaid
sequenceDiagram
  participant FE as Frontend(Viewer)
  participant BE as Backend(FastAPI)
  participant DS as Data(video_data)
  participant VS as VideoService
  FE->>BE: GET /api/search?q=...
  BE->>VS: search_videos(q)
  VS->>DS: 模拟数据检索
  DS-->>VS: 命中集合
  VS-->>BE: 列表(JSON)
  BE-->>FE: 渲染结果
  FE->>BE: GET /api/videos/{rid}/stream
  BE->>VS: get_video_stream(rid)
  VS-->>BE: 返回示例URL
  BE-->>FE: 播放地址
```

4.1.2 节点管理

节点管理功能围绕 `GET /api/peers`、`GET /api/peers/stats` 与 `PUT /api/peers/{pid}/toggle` 展开：接口在 `app/api/peers.py` 中注册，服务层由 `PeerService` 代理至 `app/data/peer_data.py`。数据端初始化 20 个模拟节点，记录在线状态、上下行速率、累计流量、是否启用与 NAT 类型等要素，其中 NAT 类型取自 `NAT_TYPES` 列表（包含 Full Cone、Restricted Cone、Port Restricted Cone、Symmetric）。统计接口在当前实现中聚合在线节点的上下行速率，并固定示例 NAT 类型，用于前端页头的运行概要展示；启停接口则直接修改内存字典中对应节点的 `is_enabled` 字段。该设计的初衷在于以“轻量模拟—接口固化”的方式稳定前端交互语义，使后续将数据源切换为 Repeater + P2P 内核时，前端无需改动即可接入真实运行态指标。

```mermaid
graph TD;
  A[PeerService.list_peers] --> B[peer_data.get_peers_list];
  C[PeerService.get_stats] --> D[peer_data.get_peer_stats];
  E[PeerService.toggle] --> F[peer_data.toggle_peer];
  B --> G[前端列表渲染];
  D --> H[页头统计/NAT类型];
  F --> I[启停状态变更];
```

4.1.3 历史记录

历史记录模块由 `GET/POST/DELETE /api/history` 构成，接口位于 `app/api/history.py`，服务层调用 `HistoryService`，最终落入 `app/data/history_data.py` 的内存存储。新增记录时，后端以 `get_current_time()` 生成 `play_time/created_at/updated_at`，并以 `generate_id()` 生成唯一标识；列表查询对记录按播放时间逆序返回，以满足“最近一次观看优先”的使用习惯。当前实现不引入持久化数据库，旨在降低演示复杂度，同时通过统一的数据访问层为后续的持久化演进（如落地 file store 或外部 DB）保留清晰的替换点。

```mermaid
graph TD;
  POSTH[POST api history];
  SVCC[HistoryService.create];
  ADDH[add_history record];
  GETH[GET api history];
  LSTH[list_history sort];
  DELH[DELETE api history id];
  RMH[delete_history id];
  POSTH --> SVCC;
  SVCC --> ADDH;
  GETH --> LSTH;
  DELH --> RMH;
```

4.1.4 设置功能

设置模块通过 `GET/PUT /api/settings` 提供获取与更新功能。后端 `SettingsService.update()` 以“部分字段合并”的方式应用配置变更，前端设置页据此形成“显式参数—即时生效”的交互契约。当前数据仍存放于内存结构，设计上建议将关键参数（如是否启用 P2P、本地缓存上限、上传限速、线程数等）映射为 Repeater 的字段化接口，以实现从 UI 到内核的端到端配置链路。

4.2 进程间数据中转传输

4.2.1 进程间通信（IPC）概述

为了在 Python Web 后端与 C++ 内核之间建立稳定且低开销的桥接，系统采用 Unix Domain Socket（默认 `/tmp/p2p_repeater.sock`）承载 IPC。消息采用固定 37 字节头部：`func_type(1B)`、`field(32B)` 与 `len(4B, BE)`，消息体为原始字节流。`func_type=1` 表示 Getter，`func_type=2` 表示 Setter，字段名以 UTF-8 编码，超出 32 字节将被拒绝。该“窄接口—宽语义”的设计大幅降低了跨语言绑定与并发读写的复杂度，便于字段的增量演进与兼容管理。

4.2.2 自定义通信协议与字段组织

C++ 侧在 `repeater/cpp/include/repeater/server.hpp` 中定义 `FieldRegistry`，将字段名映射至读写回调；Python 侧使用 `repeater/python/repeater/client.py` 的 `get(field)` 与 `set(field, value)` 发起调用。服务端在 `server.cpp` 中以 `accept_loop()` 接收连接，`handle_client()` 解析头部并分发至注册回调，随后按相同头部结构回写响应。字段建议以命名域方式组织（如 `peer.*、net.*、cfg.*、video.*`），由后端进行聚合与暴露。如下图展示了一个典型的 Getter 调用链路。

```mermaid
sequenceDiagram
  participant PY as Python(Backend)
  participant CL as RepeaterClient
  participant SV as IpcServer
  participant RG as FieldRegistry
  participant CC as C++ Core
  PY->>CL: get("peer.count")
  CL->>SV: UDS 发送(37B头)
  SV->>RG: 查找 getter 回调
  RG->>CC: 读取运行指标
  CC-->>SV: 字节流
  SV-->>CL: 回写(头+数据)
  CL-->>PY: 返回值
```

4.2.3 服务端（C++）

服务端以 `IpcServer` 封装监听、接入与请求处理。在 `handle_client()` 中依序完成：读取头部、按长度读取数据、裁剪字段名的 `\0` 终止、查找对应 getter/setter 回调并执行、再以同构头部回写响应。代码显式区分了读取完整头部与数据体的错误处理，从而在半双工场景下保证协议同步性。由于消息体保持原样传输，字段值的编码策略交由调用方定义，既适配数值型也兼容结构化序列化方案。

4.2.4 客户端（Python）

客户端以 `RepeaterClient` 封装 `_send(func_type, field, data)`，构造 37 字节头并发送，再读取响应头与数据体。`set()` 支持 `bool/int/float/bytes/str` 的自动编码，其中整数采用 64 位大端，浮点使用 IEEE754 双精度。该设计以最小代码量提供了一致的调用体验，并以异常抛出方式显式处理连接中断与半包问题，便于在后端服务层做统一重试与降级。

4.3 点对点通信模块

点对点通信模块承担了对等节点的发现与建链、应用层协议的组织与复用、数据与命令的可靠传输，以及与缓存子系统的协同。模块实现位于 `service/p2p`，与 `nat-traversal`、`tcp-like` 与 `store` 子系统在结构上保持解耦，可分别替换或升级而不破坏整体约束。其设计遵循“轻信令、强数据”的原则，通过对可靠 UDP 会话的抽象实现对上层的透明承载，同时以可扩展的协议编排支持协助连接、片段请求与拓扑维护等控制语义。

4.3.1 节点网络的建立

节点网络的建立围绕三个方面展开：端节点抽象与路由维护、连通性协商与会话建立、以及上线与同步后的稳态传输。首先，系统以 `Peer/SuperPeer` 抽象端节点的最小职责，前者负责连接与会话承载，后者作为始终在线的辅助节点在发现与回退场景中发挥作用。`RouterTable` 维护在线与离线的路由项，并在节点上线或下线事件中更新状态以供调度与观测使用。其次，连通性协商依赖 `nat-traversal` 子系统完成候选收集与路径选择，随后由 `tcp_like::ReliableSession` 完成握手与状态机驱动，从而建立具备顺序与可靠保证的单连接数据通道。最后，节点在完成注册后进入稳态传输阶段，持续以心跳与空闲超时维持 NAT 映射的活性，并向上层暴露链路统计以供调参。

图4-3 展示了从发现、穿透、建链到同步的路径。可以看出，数据面的形成是在轻量信令的前提下逐步推进的，既保证了跨 NAT 环境下的可达性，又控制了额外的状态与资源开销。

```mermaid
graph TD
  DISC[候选收集与邻居发现] --> NAT[连通性探测与打洞]
  NAT --> REL[可靠会话建立]
  REL --> REG[路由注册与上线]
  REG --> SYNC[命令/数据复用传输]
```

协助连接机制体现了信令与传输之间的协同。发起方向一个已在线且可达的邻居发送协助请求，由该邻居向目标节点转发“请发起连接”的提示；目标节点基于 NAT 模块提供的候选与通路选择策略主动回连至发起方，二者在可靠会话上完成握手，最终由路由表更新在线状态并进入数据交换阶段。该过程避免了对中心化信令面的强依赖，在复杂 NAT 条件下具有更好的鲁棒性。

```mermaid
sequenceDiagram
  participant A as 发起方(A)
  participant H as 协助方(Helper)
  participant B as 目标方(B)
  A->>H: AskAssist(target=B)
  H->>B: AssistOffer(peer=A)
  B->>A: NAT 协商/打洞
  A-->>B: ReliableSession 建立
  A->>A: 路由表更新: online(B)
```

4.3.2 节点间传输协议

节点间传输协议在统一头部的基础上复用一条可靠 UDP 会话，以实现“命令—数据”双语义的并行承载。协议定义了 8 字节的报文头部用于区分命令与数据并传递最小必要的长度信息，由此避免了多路 TCP 带来的状态管理与系统调用开销。命令语义主要覆盖协助连接、资源请求与上线通知等轻量控制，数据语义按片段载荷直接承载内容块，从而与上层的分片与调度逻辑自然耦合。底层的 `tcp_like::ReliableSession` 提供序号、选择性确认与乱序重组机制，并以固定心跳与空闲超时保证状态机稳定，这使得应用层在维持低延迟特性的同时获得近似 TCP 的交付语义。

在稳态阶段，协议通过单连接复用减少了上下文切换与缓冲区拷贝的次数，结合对 `ReliableSession::Stats` 的周期性读取，为上层策略提供带宽、时延与丢包的即时观测。这一“单头部—双语义—单会话复用”的设计，使节点能够在资源约束环境下保持较高的链路利用率与可观测性。

4.3.3 资源查找

资源查找以分布式哈希思想为基础，将资源标识通过 SHA-1 映射至 160 位键空间，并以简化的 Kademlia 表维护节点与键之间的近邻关系。在节点规模较小时，该表可近似为邻接列表的排序视图，用于快速筛选可能持有目标资源的若干候选节点；在规模扩展时，可升级为分桶与迭代查找的完整实现。数据请求流程由 `DataApi` 统一编排：当本地缓存未命中时，系统依据键空间近邻向若干候选节点发送资源请求；收到对端回包后将数据写入本地 `FileStore`，并可选地更新路由与 DHT 状态以提升后续命中率。

图4-4 给出了资源请求的抽象时序。该流程体现了寻址、传输与缓存的闭环关系，即“按键定位—并发请求—本地回填—状态更新”的最小可行路径。

```mermaid
sequenceDiagram
  participant C as 请求方
  participant DHT as DHT 映射
  participant P as 提供方
  C->>DHT: 查找近邻(key=SHA1(sid))
  DHT-->>C: 返回候选列表
  C->>P: Command(Get, sid)
  P-->>C: Data(片段)
  C->>C: 写入 FileStore 与状态更新
```

为便于对实现与接口进行一致性验证，表4-1 汇总了核心子模块的职责与关键接口。职责边界沿“连接—路由—寻址—数据—编排”的链路展开，并通过最小交叉依赖保持模块化可替换性。

| 模块 | 主要职责 | 关键接口 |
|---|---|---|
| Peer/SuperPeer | 连接与在线状态 | connect(nat)、disconnect()、send_command()/send_data() |
| RouterTable | 在线/离线维护与超级节点 | set_superpeer()、upsert_online()、get()/list_*() |
| KademliaTable | 键空间近邻查找 | add_node()、find_closest() |
| DataApi | 本地/网络一体化数据访问 | getData()、putData()、createData()、findData() |
| Node | 事件循环与消息分发、协助连接 | start()/stop()、request_assist()、request_get() |

综上所述，点对点通信模块以统一报文与单会话复用为基础，在保证工程复杂度可控的同时实现了较好的扩展弹性。其与 NAT 穿透、可靠会话与本地缓存的协作共同构成系统的数据面闭环，为上层的播放调度与状态观测提供了稳定的运行基座。

4.4 资源文件管理模块

4.4.1 资源的保存

资源以字符串 `sid` 标识，由 `store::FileStore` 承载。创建路径 `create(sid)` 在文件系统中新建空文件并写入元信息；追加路径 `append(sid, data)` 在容量校验与必要淘汰之后对文件顺序写入，并原子化更新 `lastModifiedEpochMs` 与 `sizeBytes`，同时累加 `usedSizeBytes_` 并持久化到根目录下的元文件。该实现优化了顺序写吞吐并控制了崩溃恢复后的状态一致性。

4.4.2 资源的读取

读取接口 `read(sid, begin, end)` 打开只读文件，进行范围读取；当入参非法或系统调用失败时返回空向量。该语义与上层的分片拉流相契合，使播放页可按需提取区间数据以构建缓存窗口。

4.4.3 缓存淘汰机制

存储上限通过 `set_storage_limit()` 配置；当写入导致 `used > limit` 时，`evict_until(bytesNeeded)` 以“最近修改时间”作为近似 LRU 的替代指标，逐个删除最久未修改的文件直至满足空间需求。此外，后台线程以固定间隔关闭 5 秒以上未操作的打开句柄，降低 fd 占用与泄漏风险。下图展示了写入与空间管理流程（按 Mermaid 9.3.0 语法书写）。

```mermaid
graph TD;
  WREQ([写入请求 data-sid]);
  CHK{容量检查};
  APP[[顺序追加写入]];
  EVICT[[基于最近修改时间淘汰]];
  META([更新元信息 时间戳 大小]);
  USED([刷新已用空间]);
  IDLE([后台线程回收空闲fd]);
  WREQ --> CHK;
  CHK --> APP;
  CHK --> EVICT;
  EVICT --> CHK;
  APP --> META;
  META --> USED;
  USED --> IDLE;
```

4.5 NAT 穿透模块

4.5.1 NAT 类别判定

`nat_traversal/nat_types.hpp` 定义了 `NatType` 枚举与 `toString()`，覆盖 OpenInternet、FullCone、RestrictedCone、PortRestrictedCone、Symmetric 与 Unknown。`NatDetector` 在头文件中暴露 `detect(local, stunServer)` 占位接口与 `natType()` 访问器，为集成 STUN/自研探测逻辑预留了入口。设计上，判定结果不仅用于 UI 展示，也将作为连通性协商与路径排序的输入。

4.5.2 穿透会话管理

`NatAgent` 封装候选收集、通路协商与保活维护：`gatherCandidates()` 返回本端候选；`negotiate(remoteCandidates)` 在占位实现中选择对端首个候选；`connectReliable(path, session)` 以 `tcp_like::ReliableSession` 建链并复用其心跳保持 NAT 映射存活。该抽象与可靠会话解耦，使后续引入更复杂的打洞策略与中继回退成为可能。下图给出协商与建链的时序示意。

```mermaid
sequenceDiagram
  participant A as NatAgent(A)
  participant B as NatAgent(B)
  A->>A: gatherCandidates()
  B->>B: gatherCandidates()
  A->>B: 交换候选
  B->>A: 交换候选
  A->>A: negotiate(remote)
  A->>RS: connectReliable(path)
  RS-->>A: Established
```

4.6 基于 UDP 的可靠通信

本节描述系统在 UDP 之上实现的可靠传输机制与其与其他子系统的协同。实现位于 `service/tcp-like`，核心为 `tcp_like::ReliableSession`，其提供了握手建链、选择性确认、乱序重排、固定超时重传、心跳保活、空闲超时与速率统计等能力，满足在不引入 RTT/RTO 自适应、拥塞控制、分片与 FIN 优雅关闭的前提下对上层 P2P 数据面的承载需求。

4.6.1 三次握手与状态机

可靠会话采用三次握手完成端到端状态同步。客户端通过 `connect(local, remote)` 进入 `SynSent` 并发送携带 `SYN` 标志的空载包；服务端 `listen(local)` 后以 `acceptFrom(remote)` 指定期望对端并进入 `SynReceived`，收到 `SYN` 后返回 `SYN|ACK`；客户端在接收 `SYN|ACK` 后回送 `ACK`，双方进入 `Established`。握手推进与必要的重发均由周期性调用的 `update()` 驱动。实现以 `State` 表示 `Closed、SynSent、SynReceived、Established、FinWait、TimeWait` 等状态，并在每次有效事件时更新最近活动时间以供超时判定。为避免错误报文干扰，接收路径仅处理源地址与端口与 `remote` 完全匹配的包头，构成对噪声与意外接入的第一层过滤。

报文头部统一为四个 32 位字段：发送序列号 `sequence`、最高确认号 `ack` 与最近 32 个序号的选择性确认位图 `ackBits`，以及用于握手与保活的 `flags`。下图给出握手阶段的简化时序，`update()` 为唯一推进点。

```mermaid
sequenceDiagram
  participant C as Client(ReliableSession)
  participant S as Server(ReliableSession)
  C->>S: UDP: SYN(seq=x)
  S-->>C: UDP: SYN|ACK(seq=y, ack=x)
  C->>S: UDP: ACK(seq=x+1, ack=y)
  Note over C,S: 状态均迁移至 Established
```

4.6.2 数据包重排与按序交付

数据平面在发送侧为每帧应用负载分配单调递增的 32 位 `sequence`，并在 `update()` 中按固定间隔对未确认分片进行重发。接收侧维护两组结构以矫正乱序与重复到达：一是 `lastReceivedSequence_` 与 `recvMask_`，用于记录已见到的最大序号及其之前 32 个位置的选择性接收状态；二是 `reorderBuffer_` 与 `expectedSequence_`，用于暂存超前到达的分片并在条件满足时一次性推入应用可读队列 `recvQueue_`。为处理 32 位序号的环回问题，实现选用带符号差值的序号比较辅助函数判定先后关系，保证长连接下的比较一致性。

在接收路径中，每当成功处理一个数据包，系统立即发送不含应用负载的 `ACK-only` 报文，将最新的 `ack/ackBits` 回传给对端。该策略缩短了确认反馈的路径，减少在途冗余并加速发送侧的队列回收。重排过程可以概括为“按序交付—尝试从缓存继续推进—若超前则暂存—若落后且在窗口内则仅置位”的循环，图4-6-1给出其抽象流程。



```mermaid
graph TD
  A[到达 seq=k 数据包]-->B{seq==expected?}
  B-->|是| C[写入 recvQueue_]
  C-->D[expected++]
  D-->E{reorderBuffer_<br/> 是否命中<br/> expected}
  E-->|是| C
  E-->|否| F[结束]
  B-->|大于| G[缓存到 reorderBuffer_]
  B-->|小于且在32内| H[设置 recvMask_ 对应位]
```

4.6.3 心跳保活与空闲超时

为维系 NAT 映射并监测链路活性，会话在空闲期按可配置的周期发送 `PING` 并在接收到 `PING` 时即时回复 `PONG`。心跳默认开启且间隔为 1000ms，可在运行时调整或关闭。所有心跳、握手与数据的处理均统一在 `update()` 中完成，避免引入多线程复杂性。系统同时维护空闲超时计时器，当连续一段时间未观测到有效数据或心跳往返时，状态迁移至 `TimeWait` 并释放底层套接字，从而避免僵死连接占用资源。心跳与超时逻辑均以最近活动时间戳为判据，任何方向的数据或心跳响应都会刷新该时间。

4.6.4 超时控制与统计

发送侧针对每个在途分片维护上次发送时间与重试计数，在 `update()` 中以固定阈值判断是否需要重发，超过最大重试次数的分片将被丢弃。尽管未实现 RTT/RTO 自适应与拥塞控制，固定超时与 ACK-only 的结合在轻度丢包环境下提供了可接受的恢复速度与资源占用。会话并行维护滑动时间窗的速率估计与累计上下行字节数，通过 `getStats()` 暴露给上层，以支持运行时观测与策略调参。示例程序周期性读取并打印速率数值，验证统计口径的稳定性。

可靠会话与 NAT 模块及 P2P 层之间的关系可概括为：NAT 模块提供可达路径与必要的保活，可靠会话在该路径上建立并维持具备顺序与可靠保证的数据通道，P2P 层则在该通道上复用命令与数据语义。该分层使得路径协商与数据承载解耦，便于在后续引入更复杂的穿透策略而无需变更传输语义。

```mermaid
graph TD;
  IDLE[空闲];
  SYN[SYN 发送];
  SYNACK[SYNACK 接收];
  EST[已建立];
  HB[心跳 数据收发 重传];
  FIN[关闭 超时];
  TW[TimeWait];
  IDLE --> SYN;
  SYN --> SYNACK;
  SYNACK --> EST;
  EST --> HB;
  EST --> FIN;
  FIN --> TW;
  TW --> IDLE;
```

---

```mermaid
graph TD
  NAT[NAT Traversal\n候选/路径/保活]-->REL[ReliableSession\n握手/ACK/重排/心跳]
  REL-->P2P[P2P 协议\n命令/数据复用]
  P2P-->APP[调度/存储]
```

---

# 总结

本文系统性地构建了一套面向视频分发的端到端 P2P 加速体系，采用“浏览器前端—Python 服务—IPC 中转—C++ 内核”的分层解耦架构，以字段化的轻量协议统一控制面与观测面的接口语义，并以 NAT 穿透与基于 UDP 的可靠会话构成低时延的数据面。在该架构下，前端以统一的交互范式承载检索、播放与可视化，后端以稳定的 REST 契约进行编排与降级，Repeater 通过 Unix 域套接字把键值化的 get/set 映射到内核字段处理，P2P 层围绕路由、寻址与分片调度组织数据交换，FileStore 则提供顺序友好、可解释淘汰的本地缓存基座。

在实现层面，系统完成了从协议到样例的最小可行闭环：后端以模拟数据固化 API 语义，保障在内核尚未接入时即可联调；Repeater 以 37 字节定长头实现“窄接口—宽语义”的跨语言协作；P2P 通过单会话复用的自定义头把“命令/数据”并行承载，并结合简化的 Kademlia 映射完成近邻搜索与资源请求；可靠会话实现三次握手、选择确认、乱序重排、ACK-only 与心跳空闲控制，向上暴露速率与在途统计；FileStore 以原子化元信息维护与近似 LRU 的回收策略稳定长时运行下的吞吐。各层关键指标通过字段上报形成观测—决策—执行的闭环。

综合来看，该体系在工程复杂度与可演进性之间取得平衡：接口契约稳定、模块边界清晰、替换成本低，能够随业务渐进引入拥塞控制、RTT/RTO 自适应、分片与 MTU 探测、FEC 与多路径、更加精细的淘汰与热点调度，以及完善的 NAT 类型识别与中继回退策略；同时可将模拟数据无缝切换为真实内核指标，面向生产特性逐步增强。

# 致谢

本文的顺利完成，首先谨向我的导师表示诚挚感谢。导师在选题凝练、技术路线规划与论文写作规范方面给予了耐心指导，使我得以在工程实现与理论把握之间取得较为稳妥的平衡。同时，感谢学校提供的良好学习与实践环境，完善的课程体系与实验资源为本课题奠定了坚实基础；亦感谢任课教师在计算机网络、操作系统、软件工程等课程中的启发，使我能够以系统化视角开展设计与验证。家人和同学在时间安排与心理支持上的理解与鼓励，是我坚持完成本工作的持续动力。

此外，本设计受益于开源生态与学术社区的长期积累：Linux 网络编程与 epoll 机制、CMake 构建体系、FastAPI/Starlette 与 Uvicorn 的高效后端框架、Vue 3 的组件化范式、OpenSSL Crypto 在哈希等基础能力上的支持，以及关于 Kademlia、可靠 UDP 与拥塞控制等公开资料，为实现与对比提供了可靠工具与可复现参考。由于个人能力与时间所限，文中实现仍有进一步改进空间，后续拟围绕拥塞控制与 RTO 自适应、NAT 探测与多路径传输、缓存淘汰的多目标优化等方向持续完善。