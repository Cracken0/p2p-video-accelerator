"""
模拟数据初始化入口
"""
from typing import Dict, Any
from .store import db
from .video_data import init_video_data
from .peer_data import init_peer_data
from .history_data import init_history_data
from .settings_data import init_settings_data


def init_mock_data():
    init_video_data()
    init_peer_data()
    init_history_data()
    init_settings_data()


def get_mock_data() -> Dict[str, Any]:
    return db





