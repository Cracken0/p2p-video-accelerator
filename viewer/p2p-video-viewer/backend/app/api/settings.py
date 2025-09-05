from fastapi import APIRouter
from typing import Dict, Any
from app.models.settings import SettingsUpdateRequest
from app.services.settings_service import SettingsService

router = APIRouter(prefix="/settings", tags=["settings"])


@router.get("/", response_model=Dict[str, Any])
async def get_settings():
    return SettingsService.get()


@router.put("/", response_model=Dict[str, Any])
async def update_settings(req: SettingsUpdateRequest):
    return SettingsService.update(req.model_dump(exclude_none=True))
