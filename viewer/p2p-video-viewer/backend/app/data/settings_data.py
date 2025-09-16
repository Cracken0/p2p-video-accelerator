"""
设置模拟数据
"""

from .store import db


def init_settings_data():
    db["settings"] = {
        "p2p_service_enabled": True,
        "local_cache_limit": 200.0,
        "upload_speed_limit": 512.0,
        "thread_count": 4
    }


def get_settings():
    return db["settings"]


def update_settings(partial: dict):
    db["settings"].update({k: v for k, v in partial.items() if v is not None})
    return db["settings"]
