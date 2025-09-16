"""
历史记录模拟数据
"""

from datetime import timedelta
from .store import db, get_current_time, generate_id


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
            "cache_exists": True,
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
