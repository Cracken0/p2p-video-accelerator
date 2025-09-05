from fastapi import APIRouter, Query
from typing import List
from app.models.video import VideoSearchResult
from app.services.video_service import VideoService

router = APIRouter(prefix="/search", tags=["search"])


@router.get("/", response_model=List[VideoSearchResult])
async def search_videos(q: str = Query("", alias="query")):
    results = VideoService.search_videos(q or "")
    # 直接返回字典列表即可匹配Pydantic模型
    return results
