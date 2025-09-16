from typing import List, Dict, Any
from datetime import datetime
from app.data.history_data import list_history, add_history, delete_history, sync_history_with_videos
from app.data.store import db
from app.data.store import generate_id, get_current_time


class HistoryService:
    @staticmethod
    def get_list() -> List[Dict[str, Any]]:
        history_list = list_history()
        # 处理datetime比较问题，统一转换为字符串进行比较
        def safe_sort_key(record):
            play_time = record.get("play_time")
            if isinstance(play_time, datetime):
                return play_time.isoformat()
            return str(play_time)
        
        return sorted(history_list, key=safe_sort_key, reverse=True)

    @staticmethod
    def create(rid: str, title: str, file_size: float, playback_progress: float,
               cache_exists: bool, avg_download_speed: float) -> Dict[str, Any]:
        now = get_current_time()
        record = {
            "id": generate_id(),
            "rid": rid,
            "title": title,
            "play_time": now,
            "file_size": file_size,
            "playback_progress": playback_progress,
            "cache_exists": cache_exists,
            "avg_download_speed": avg_download_speed,
            "created_at": now,
            "updated_at": now,
        }
        # 使用add_history会自动同步视频信息
        return add_history(record)

    @staticmethod
    def update(hid: str, rid: str, title: str, file_size: float, playback_progress: float,
               cache_exists: bool, avg_download_speed: float, play_time: datetime = None) -> Dict[str, Any]:
        history = db.get("history", {})
        if hid not in history:
            return None
        
        record = history[hid]
        update_data = {
            "rid": rid,
            "title": title,
            "file_size": file_size,
            "playback_progress": playback_progress,
            "cache_exists": cache_exists,
            "avg_download_speed": avg_download_speed,
            "updated_at": get_current_time(),
        }
        
        # 如果提供了play_time，则更新它
        if play_time:
            update_data["play_time"] = play_time
        
        record.update(update_data)
        
        # 同步视频信息
        sync_history_with_videos()
        
        return record

    @staticmethod
    def delete(hid: str) -> bool:
        return delete_history(hid)
