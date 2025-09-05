"""
模拟数据基础文件
包含所有模拟数据的初始化和管理
"""

import uuid
from datetime import datetime, timedelta
from typing import Dict, List, Any

# 全局模拟数据存储
mock_data = {
    "videos": {},
    "peers": {},
    "history": {},
    "settings": {},
    "search_results": {}
}

def generate_id() -> str:
    """生成唯一ID"""
    return str(uuid.uuid4())

def get_current_time() -> datetime:
    """获取当前时间"""
    return datetime.now()

def format_datetime(dt: datetime) -> str:
    """格式化日期时间"""
    return dt.strftime("%Y/%m/%d %H:%M")

# 初始化模拟数据
def init_mock_data():
    """初始化所有模拟数据"""
    init_video_data()
    init_peer_data()
    init_history_data()
    init_settings_data()
    init_search_data()

def get_mock_data() -> Dict[str, Any]:
    """获取所有模拟数据"""
    return mock_data





