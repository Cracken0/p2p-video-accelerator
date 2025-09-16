"""
历史记录模拟数据
"""

from datetime import timedelta, datetime
from .store import db, get_current_time, generate_id
from .settings_data import get_settings


def get_video_info_by_rid(rid: str):
    """从video_data中获取视频信息"""
    videos = db.get("videos", {})
    return videos.get(rid)


def init_history_data():
    now = get_current_time()
    history = {}
    
    # 定义历史记录基本信息
    history_samples = [
        {
            "id": "h_001",
            "rid": "video_001",
            "play_time": now - timedelta(hours=2),
            "playback_progress": 53.0,
            "cache_exists": False,
            "avg_download_speed": 3.2,
            "created_at": now - timedelta(days=1),
            "updated_at": now - timedelta(hours=2),
        },
        {
            "id": "h_002",
            "rid": "video_003",
            "play_time": now - timedelta(days=1, hours=1),
            "playback_progress": 27.5,
            "cache_exists": False,
            "avg_download_speed": 2.7,
            "created_at": now - timedelta(days=1, hours=3),
            "updated_at": now - timedelta(days=1, hours=1),
        }
    ]
    
    # 为每个历史记录同步视频信息
    for sample in history_samples:
        video_info = get_video_info_by_rid(sample["rid"])
        if video_info:
            # 从video_data中获取title和file_size
            sample["title"] = video_info["title"]
            # 将字节转换为MB，保留两位小数
            sample["file_size"] = round(video_info["file_size"] / (1024 * 1024), 2)
        else:
            # 如果找不到对应的视频信息，使用默认值
            sample["title"] = f"未知视频 {sample['rid']}"
            sample["file_size"] = 0.0
            
        history[sample["id"]] = sample
    
    db["history"] = history


def list_history():
    return list(db["history"].values())


def add_history(record: dict):
    if not record.get("id"):
        record["id"] = generate_id()
    
    # 同步视频信息
    video_info = get_video_info_by_rid(record["rid"])
    if video_info:
        record["title"] = video_info["title"]
        # 将字节转换为MB，保留两位小数
        record["file_size"] = round(video_info["file_size"] / (1024 * 1024), 2)
    else:
        # 如果找不到对应的视频信息，使用默认值
        record["title"] = f"未知视频 {record['rid']}"
        record["file_size"] = 0.0
    
    db["history"][record["id"]] = record
    
    # 检查缓存限制
    _check_cache_limit_after_add()
    
    return record


def delete_history(hid: str) -> bool:
    return db["history"].pop(hid, None) is not None


def sync_history_with_videos():
    """同步所有历史记录的视频信息"""
    history = db.get("history", {})
    for record in history.values():
        video_info = get_video_info_by_rid(record["rid"])
        if video_info:
            record["title"] = video_info["title"]
            # 将字节转换为MB，保留两位小数
            record["file_size"] = round(video_info["file_size"] / (1024 * 1024), 2)
        else:
            record["title"] = f"未知视频 {record['rid']}"
            record["file_size"] = 0.0


def get_cached_history_size():
    """计算所有缓存存在的历史记录的视频大小之和(MB)"""
    history = db.get("history", {})
    total_size = 0.0
    for record in history.values():
        if record.get("cache_exists", False):
            total_size += record.get("file_size", 0.0)
    return total_size


def cleanup_cache_by_limit(cache_limit_mb: float):
    """
    根据缓存上限清理历史记录
    按照访问时间最早的顺序，将缓存存在设为否，直到不超过上限
    """
    history = db.get("history", {})
    
    # 获取所有缓存存在的记录，按访问时间排序（最早的在前）
    cached_records = []
    for record in history.values():
        if record.get("cache_exists", False):
            cached_records.append(record)
    
    # 按访问时间排序（最早的在前）
    def safe_sort_key(record):
        play_time = record.get("play_time")
        if isinstance(play_time, datetime):
            return play_time.isoformat()
        return str(play_time)
    
    cached_records.sort(key=safe_sort_key)
    
    # 计算当前缓存大小
    current_size = sum(record.get("file_size", 0.0) for record in cached_records)
    
    # 如果当前大小超过限制，开始清理
    if current_size > cache_limit_mb:
        cleaned_count = 0
        for record in cached_records:
            if current_size <= cache_limit_mb:
                break
            
            # 将缓存存在设为否
            record["cache_exists"] = False
            record["updated_at"] = get_current_time()
            
            # 更新当前大小
            current_size -= record.get("file_size", 0.0)
            cleaned_count += 1
        
        print(f"缓存清理完成：清理了 {cleaned_count} 个历史记录，当前缓存大小: {current_size:.2f}MB")
        return cleaned_count
    
    return 0


def _check_cache_limit_after_add():
    """在添加历史记录后检查缓存限制"""
    try:
        settings = get_settings()
        cache_limit = settings.get("local_cache_limit", 2048.0)
        
        current_cache_size = get_cached_history_size()
        
        if current_cache_size > cache_limit:
            cleaned_count = cleanup_cache_by_limit(cache_limit)
            print(f"添加历史记录触发缓存清理：清理了 {cleaned_count} 个历史记录")
        else:
            print(f"添加历史记录检查缓存：当前大小 {current_cache_size:.2f}MB，未超过限制 {cache_limit:.2f}MB")
    except Exception as e:
        print(f"添加历史记录后检查缓存限制时出错: {e}")
