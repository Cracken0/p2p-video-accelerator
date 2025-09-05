# Python 客户端使用文档

## 使用示例

```python
from repeater.client import RepeaterClient

c = RepeaterClient("/tmp/p2p_repeater.sock")

# 获取基础类型
value_bytes = c.get("store.used")
print("used bytes(raw):", value_bytes)

# 设置基础类型
ok = c.set("player.playing", True)
print("set ok:", ok)
```

编码规则：
- getter 返回原始字节流，由服务器决定编码方式。
- setter 发送原始字节流，常用：
  - bool: 0x00/0x01
  - int: 8字节 big-endian 有符号整数
  - float: IEEE 754 双精度 (big-endian)
  - string: UTF-8
