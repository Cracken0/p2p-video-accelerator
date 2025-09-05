from pydantic import BaseModel
from typing import Optional, List
from datetime import datetime

class Video(BaseModel):
    """视频模型"""
    rid: str
    title: str
    description: str
    thumbnail_url: str
    duration: int  # 秒
    file_size: int  # 字节
    created_at: datetime
    updated_at: datetime

class VideoSearchResult(BaseModel):
    """视频搜索结果"""
    rid: str
    title: str
    description: str
    thumbnail_url: str
    duration: int
    file_size: int

class VideoStreamInfo(BaseModel):
    """视频流信息"""
    rid: str
    url: str
    format: str
    quality: str
    bitrate: int

class VideoPlaybackStatus(BaseModel):
    """视频播放状态"""
    rid: str
    current_time: float
    duration: float
    is_playing: bool
    buffered: float
    playback_rate: float





