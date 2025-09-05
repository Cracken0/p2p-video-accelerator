from typing import Dict, Any
from app.data.settings_data import get_settings, update_settings


class SettingsService:
    @staticmethod
    def get() -> Dict[str, Any]:
        return get_settings()

    @staticmethod
    def update(partial: Dict[str, Any]) -> Dict[str, Any]:
        return update_settings(partial)
