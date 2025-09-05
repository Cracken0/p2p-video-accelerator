from pydantic import BaseModel
from typing import Optional
from datetime import datetime

class Peer(BaseModel):
    """Peer节点模型"""
    pid: str
    is_online: bool
    download_speed: float  # MB/s
    upload_speed: float    # KB/s
    total_download: float  # MB
    total_upload: float    # KB
    is_enabled: bool
    nat_type: str
    last_seen: datetime
    created_at: datetime

class PeerStats(BaseModel):
    """Peer统计信息"""
    total_peers: int
    online_peers: int
    total_download_speed: float  # MB/s
    total_upload_speed: float    # KB/s
    nat_type: str

class PeerToggleRequest(BaseModel):
    """Peer开关请求"""
    enabled: bool





