from pydantic import BaseModel, Field
from typing import Optional

class Settings(BaseModel):
    """设置模型"""
    p2p_service_enabled: bool = Field(default=True, description="开启p2p服务")
    local_cache_limit: float = Field(default=1024.0, description="本地缓存上限(MB)")
    upload_speed_limit: float = Field(default=1024.0, description="上传速度上限(KB/s)")
    thread_count: int = Field(default=4, ge=1, le=16, description="多线程数量")

class SettingsUpdateRequest(BaseModel):
    """设置更新请求"""
    p2p_service_enabled: Optional[bool] = None
    local_cache_limit: Optional[float] = Field(None, ge=0)
    upload_speed_limit: Optional[float] = Field(None, ge=0)
    thread_count: Optional[int] = Field(None, ge=1, le=16)





