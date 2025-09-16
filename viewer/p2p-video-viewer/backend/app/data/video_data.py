"""
视频模拟数据
"""

from datetime import timedelta
from .store import db, get_current_time


def init_video_data():
    videos = {
        "video_001": {
            "rid": "video_001",
            "title": "P2P技术详解",
            "description": "深入讲解P2P网络技术的原理和应用",
            "thumbnail_url": "https://via.placeholder.com/300x200/4A90E2/FFFFFF?text=P2P技术详解",
            "duration": 1800,
            "file_size": int(104.27 * 1024 * 1024),
            "created_at": get_current_time() - timedelta(days=5),
            "updated_at": get_current_time() - timedelta(days=1)
        },
        "video_002": {
            "rid": "video_002",
            "title": "视频加速器开发实战",
            "description": "从零开始开发一个视频加速器应用",
            "thumbnail_url": "https://via.placeholder.com/300x200/50C878/FFFFFF?text=视频加速器开发",
            "duration": 2400,
            "file_size": int(216.26 * 1024 * 1024),
            "created_at": get_current_time() - timedelta(days=3),
            "updated_at": get_current_time() - timedelta(hours=6)
        },
        "video_003": {
            "rid": "video_003",
            "title": "网络优化技巧",
            "description": "提升网络传输效率的实用技巧",
            "thumbnail_url": "https://via.placeholder.com/300x200/FF6B6B/FFFFFF?text=网络优化技巧",
            "duration": 1200,
            "file_size": int(70.52 * 1024 * 1024),
            "created_at": get_current_time() - timedelta(days=2),
            "updated_at": get_current_time() - timedelta(hours=12)
        },
        "video_004": {
            "rid": "video_004",
            "title": "分布式系统设计",
            "description": "设计高可用的分布式系统架构",
            "thumbnail_url": "https://via.placeholder.com/300x200/9B59B6/FFFFFF?text=分布式系统设计",
            "duration": 3600,
            "file_size": int(319.45 * 1024 * 1024),
            "created_at": get_current_time() - timedelta(days=7),
            "updated_at": get_current_time() - timedelta(days=2)
        },
        "video_005": {
            "rid": "video_005",
            "title": "Web开发最佳实践",
            "description": "现代Web开发的技术栈和最佳实践",
            "thumbnail_url": "https://via.placeholder.com/300x200/F39C12/FFFFFF?text=Web开发最佳实践",
            "duration": 1500,
            "file_size": int(127.84 * 1024 * 1024),
            "created_at": get_current_time() - timedelta(days=1),
            "updated_at": get_current_time() - timedelta(hours=2)
        }
    }
    db["videos"] = videos


def get_video_by_rid(rid: str):
    return db["videos"].get(rid)


def get_all_videos():
    return list(db["videos"].values())


def search_videos(query: str):
    results = []
    query_lower = (query or "").lower()
    for video in db["videos"].values():
        if query_lower in video["title"].lower() or query_lower in video["description"].lower():
            results.append(video)
    return results





