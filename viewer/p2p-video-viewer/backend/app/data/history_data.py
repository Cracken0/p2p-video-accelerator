"""
历史记录模拟数据
"""

from datetime import timedelta
from .store import db, get_current_time, generate_id


def init_history_data():
    now = get_current_time()
    history = {}
    samples = [
        {
            "id": "h_001",
            "rid": "video_001",
            "title": "P2P技术详解",
            "play_time": now - timedelta(hours=2),
            "file_size": 104.27,
            "playback_progress": 53.0,
            "cache_exists": True,
            "avg_download_speed": 3.2,
            "created_at": now - timedelta(days=1),
            "updated_at": now - timedelta(hours=2),
        },
        {
            "id": "h_002",
            "rid": "video_003",
            "title": "网络优化技巧",
            "play_time": now - timedelta(days=1, hours=1),
            "file_size": 70.52,
            "playback_progress": 27.5,
            "cache_exists": False,
            "avg_download_speed": 2.7,
            "created_at": now - timedelta(days=1, hours=3),
            "updated_at": now - timedelta(days=1, hours=1),
        }
    ]
    for r in samples:
        history[r["id"]] = r
    db["history"] = history


def list_history():
    return list(db["history"].values())


def add_history(record: dict):
    if not record.get("id"):
        record["id"] = generate_id()
    db["history"][record["id"]] = record
    return record


def delete_history(hid: str) -> bool:
    return db["history"].pop(hid, None) is not None
