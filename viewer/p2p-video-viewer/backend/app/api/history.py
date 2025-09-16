from fastapi import APIRouter, HTTPException
from typing import List, Dict, Any
from app.models.history import HistoryCreateRequest
from app.services.history_service import HistoryService

router = APIRouter(prefix="/history", tags=["history"])


@router.get("/", response_model=List[Dict[str, Any]])
async def get_history():
    return HistoryService.get_list()


@router.post("/")
async def create_history(req: HistoryCreateRequest):
    record = HistoryService.create(
        rid=req.rid,
        title=req.title,
        file_size=req.file_size,
        playback_progress=req.playback_progress,
        cache_exists=req.cache_exists,
        avg_download_speed=req.avg_download_speed,
    )
    return record


@router.put("/{hid}")
async def update_history(hid: str, req: HistoryCreateRequest):
    record = HistoryService.update(
        hid=hid,
        rid=req.rid,
        title=req.title,
        file_size=req.file_size,
        playback_progress=req.playback_progress,
        cache_exists=req.cache_exists,
        avg_download_speed=req.avg_download_speed,
        play_time=req.play_time,
    )
    if not record:
        raise HTTPException(status_code=404, detail="记录不存在")
    return record


@router.delete("/{hid}")
async def remove_history(hid: str):
    ok = HistoryService.delete(hid)
    if not ok:
        raise HTTPException(status_code=404, detail="记录不存在")
    return {"message": "deleted"}
