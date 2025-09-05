from fastapi import APIRouter, HTTPException, Query
from typing import List, Optional
from app.models.video import Video, VideoSearchResult, VideoStreamInfo, VideoPlaybackStatus
from app.services.video_service import VideoService

router = APIRouter(prefix="/videos", tags=["videos"])

@router.get("/{rid}", response_model=Video)
async def get_video(rid: str):
    """获取视频详情"""
    video = VideoService.get_video_by_rid(rid)
    if not video:
        raise HTTPException(status_code=404, detail="视频不存在")
    return video

@router.get("/{rid}/stream", response_model=VideoStreamInfo)
async def get_video_stream(rid: str):
    """获取视频流信息"""
    stream_info = VideoService.get_video_stream(rid)
    if not stream_info:
        raise HTTPException(status_code=404, detail="视频流不存在")
    return stream_info

@router.get("/{rid}/status", response_model=VideoPlaybackStatus)
async def get_video_status(rid: str):
    """获取视频播放状态"""
    status = VideoService.get_video_status(rid)
    if not status:
        raise HTTPException(status_code=404, detail="视频不存在")
    return status

@router.put("/{rid}/status")
async def update_video_status(rid: str, status: VideoPlaybackStatus):
    """更新视频播放状态"""
    success = VideoService.update_video_status(rid, status)
    if not success:
        raise HTTPException(status_code=404, detail="视频不存在")
    return {"message": "状态更新成功"}





