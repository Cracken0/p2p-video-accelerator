from fastapi import APIRouter, HTTPException
from typing import List, Dict, Any
from app.models.peer import PeerToggleRequest
from app.services.peer_service import PeerService

router = APIRouter(prefix="/peers", tags=["peers"])


@router.get("/", response_model=List[Dict[str, Any]])
async def list_peers():
    return PeerService.list_peers()


@router.get("/stats", response_model=Dict[str, Any])
async def peer_stats():
    return PeerService.get_stats()


@router.put("/{pid}/toggle")
async def toggle_peer(pid: str, req: PeerToggleRequest):
    ok = PeerService.toggle(pid, req.enabled)
    if not ok:
        raise HTTPException(status_code=404, detail="Peer不存在")
    return {"message": "ok"}
