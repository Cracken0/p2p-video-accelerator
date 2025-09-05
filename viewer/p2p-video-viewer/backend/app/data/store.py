"""
集中数据存储与工具函数，供各数据模块引用，避免循环依赖
"""
import uuid
from datetime import datetime
from typing import Dict, Any

# 全局内存数据存储
db: Dict[str, Any] = {
    "videos": {},
    "peers": {},
    "history": {},
    "settings": {},
    "search_results": {}
}


def generate_id() -> str:
    return str(uuid.uuid4())


def get_current_time() -> datetime:
    return datetime.now()


def format_datetime(dt: datetime) -> str:
    return dt.strftime("%Y/%m/%d %H:%M")
