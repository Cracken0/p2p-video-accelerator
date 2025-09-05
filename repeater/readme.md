# repeater

实现一套基于进程间通信(ipc)的自定义通信协议，用于满足以下相关需求。

# 背景

目前使用了cpp作为p2p的peer，并且进行连接、传输、保存等功能
目前也使用python + html + vue实现了http服务，提供了前端界面和中转的后端
现在需要一个中转器,就是使用进程间通信，定义一套通信协议，用来在cpp实现的服务和python的web服务之间传输数据。

# 通信需求

以下几种传输类型：
* 基础类型getter
报文结构表：
字段名 类型 占用字节数 取值 含义
func_type u8 1 1 1为getter
field string 32 发送者定义 要获取的字段名
len int 4 发送方定义 data的长度
data 字节流 len 发送方定义 字段的值。只可能是bool 整数 浮点数 字符串类型

* 基础类型setter
报文结构表：
字段名 类型 占用字节数 取值 含义
func_type u8 1 1 2为setter
field string 32 发送者定义 要获取的字段名
len int 4 发送方定义 data的长度
data 字节流 len 发送方定义 字段的值。只可能是bool 整数 浮点数 字符串类型

# 实现需求

在repeater目录下的cpp目录里实现cpp的通信接口
在repeater目录下的python目录里实现python的通信接口

最后在p2p-video-accelerator/service/bin下service.cpp作为最终的p2p后端main入口,它会引用repeater里的接口、p2p里的接口，实现系统级集成整合。
在p2p-video-accelerator/viewer/p2p-video-viewer/backend/app/main.py里之前预留的，后端获取和设置数据的，直接写临时变量的地方，都改成来调用repeater获取和设置数据。

# 其他需求

代码层次结构清晰，功能模块划分清晰，内外部接口清晰。
可以适当调整其他目录中的代码来适配整体结构。
