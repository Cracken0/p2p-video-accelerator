from pydantic import BaseModel
from typing import Optional
from datetime import datetime

class HistoryRecord(BaseModel):
    """历史记录模型"""
    id: str
    rid: str
    title: str
    play_time: datetime
    file_size: float  # MB
    playback_progress: float  # 百分比
    cache_exists: bool
    avg_download_speed: float  # MB/s
    created_at: datetime
    updated_at: datetime

class HistoryCreateRequest(BaseModel):
    """创建历史记录请求"""
    rid: str
    title: str
    file_size: float
    playback_progress: float
    cache_exists: bool
    avg_download_speed: float





