from typing import Optional
from app.data.video_data import get_video_by_rid, get_all_videos
from app.models.video import VideoStreamInfo, VideoPlaybackStatus


class VideoService:
    @staticmethod
    def get_video_by_rid(rid: str):
        return get_video_by_rid(rid)

    @staticmethod
    def search_videos(query: str):
        from app.data.video_data import search_videos as _search
        return _search(query)

    @staticmethod
    def get_video_stream(rid: str) -> Optional[VideoStreamInfo]:
        video = get_video_by_rid(rid)
        if not video:
            return None
        
        # 从视频对象中获取本地文件路径
        local_path = video.get("local_path")
        if not local_path:
            return None
            
        # 提取文件名
        import os
        filename = os.path.basename(local_path)
        # 返回完整的HTTP URL，避免前端路径拼接问题
        http_url = f"http://localhost:8000/videos/{filename}"
        # http_url = "https://interactive-examples.mdn.mozilla.net/media/cc0-videos/flower.mp4"
        
        return VideoStreamInfo(
            rid=rid,
            url=http_url,
            format="mp4",
            quality="720p",
            bitrate=2500,
        )

    @staticmethod
    def get_video_status(rid: str) -> Optional[VideoPlaybackStatus]:
        video = get_video_by_rid(rid)
        if not video:
            return None
        # 简单返回一个默认状态
        return VideoPlaybackStatus(
            rid=rid,
            current_time=0.0,
            duration=float(video["duration"]),
            is_playing=False,
            buffered=0.0,
            playback_rate=1.0,
        )

    @staticmethod
    def update_video_status(rid: str, status: VideoPlaybackStatus) -> bool:
        # 仅模拟，直接返回成功
        return get_video_by_rid(rid) is not None


