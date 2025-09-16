from typing import List, Dict, Any
from datetime import datetime
from app.data.history_data import list_history, add_history, delete_history, sync_history_with_videos, cleanup_cache_by_limit, get_cached_history_size
from app.data.settings_data import get_settings
from app.data.peer_data import update_need_download_on_cache_change
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
            "cache_exists": False, #cache_exists,
            "avg_download_speed": avg_download_speed,
            "created_at": now,
            "updated_at": now,
        }
        # 使用add_history会自动同步视频信息
        result = add_history(record)
        
        # 更新需要下载的量（新建历史记录时总是更新）
        # 新建历史记录时，old_cache_exists为None，表示这是新记录
        print(f"DEBUG: HistoryService.create调用update_need_download_on_cache_change - rid: {rid}, cache_exists: {cache_exists}")
        update_need_download_on_cache_change(rid, cache_exists, None)
        
        # 检查缓存限制
        HistoryService._check_cache_limit()
        
        return result

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
        
        # 保存更新前的缓存状态
        old_cache_exists = record.get("cache_exists", False)
        
        record.update(update_data)
        
        # 同步视频信息
        sync_history_with_videos()
        
        # 检查缓存限制
        HistoryService._check_cache_limit()
        
        # 更新需要下载的量（只在缓存状态变化时）- 在缓存限制检查之后
        # 使用更新前的状态和新的状态进行比较
        update_need_download_on_cache_change(rid, cache_exists, old_cache_exists)
        
        return record

    @staticmethod
    def delete(hid: str) -> bool:
        return delete_history(hid)
    
    @staticmethod
    def _check_cache_limit():
        """检查缓存限制，如果超过限制则清理缓存"""
        try:
            settings = get_settings()
            cache_limit = settings.get("local_cache_limit", 2048.0)
            
            current_cache_size = get_cached_history_size()
            
            if current_cache_size > cache_limit:
                cleaned_count = cleanup_cache_by_limit(cache_limit)
                # print(f"历史记录更新触发缓存清理：清理了 {cleaned_count} 个历史记录")
            else:
                pass
                # print(f"历史记录更新检查缓存：当前大小 {current_cache_size:.2f}MB，未超过限制 {cache_limit:.2f}MB")
        except Exception as e:
            print(e)
            pass
            # print(f"检查缓存限制时出错: {e}")
