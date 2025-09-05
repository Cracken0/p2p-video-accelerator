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
        # 使用可播放的公共示例视频，方便前端验证
        sample_url = "https://interactive-examples.mdn.mozilla.net/media/cc0-videos/flower.mp4"
        return VideoStreamInfo(
            rid=rid,
            url=sample_url,
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


