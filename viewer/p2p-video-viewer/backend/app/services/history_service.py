from typing import List, Dict, Any
from datetime import datetime
from app.data.history_data import list_history, add_history, delete_history
from app.data.store import generate_id, get_current_time


class HistoryService:
    @staticmethod
    def get_list() -> List[Dict[str, Any]]:
        return sorted(list_history(), key=lambda r: r["play_time"], reverse=True)

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
        return add_history(record)

    @staticmethod
    def delete(hid: str) -> bool:
        return delete_history(hid)
