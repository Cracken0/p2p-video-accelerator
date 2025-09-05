from typing import Optional
from datetime import datetime
from app.models.video import Video, VideoStreamInfo, VideoPlaybackStatus

try:
    from repeater.client import RepeaterClient
    _repeater = RepeaterClient("/tmp/p2p_repeater.sock")
except Exception:  # pragma: no cover
    _repeater = None


class VideoService:
    @staticmethod
    def get_video_by_rid(rid: str) -> Optional[Video]:
        # 演示：从 repeater 获取占位信息，实际业务可从 p2p 查询
        if _repeater:
            _ = _repeater.get("store.used")  # 触发一次通信，忽略返回
        return Video(
            rid=rid,
            title=f"Video {rid}",
            description="demo",
            thumbnail_url="/static/demo.png",
            duration=600,
            file_size=123456789,
            created_at=datetime.utcnow(),
            updated_at=datetime.utcnow(),
        )

    @staticmethod
    def get_video_stream(rid: str) -> Optional[VideoStreamInfo]:
        return VideoStreamInfo(
            rid=rid,
            url=f"/stream/{rid}",
            format="mp4",
            quality="720p",
            bitrate=1500000,
        )

    @staticmethod
    def get_video_status(rid: str) -> Optional[VideoPlaybackStatus]:
        return VideoPlaybackStatus(
            rid=rid,
            current_time=0.0,
            duration=600.0,
            is_playing=False,
            buffered=0.0,
            playback_rate=1.0,
        )

    @staticmethod
    def update_video_status(rid: str, status: VideoPlaybackStatus) -> bool:
        if _repeater:
            try:
                _repeater.set("player.playing", bool(status.is_playing))
            except Exception:
                pass
        return True


