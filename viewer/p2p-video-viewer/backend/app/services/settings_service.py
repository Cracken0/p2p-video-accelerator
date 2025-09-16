from typing import Dict, Any
from app.data.settings_data import get_settings, update_settings
from app.data.history_data import cleanup_cache_by_limit, get_cached_history_size


class SettingsService:
    @staticmethod
    def get() -> Dict[str, Any]:
        return get_settings()

    @staticmethod
    def update(partial: Dict[str, Any]) -> Dict[str, Any]:
        # 更新设置
        updated_settings = update_settings(partial)
        
        # 如果更新了缓存上限，检查并清理缓存
        if "local_cache_limit" in partial:
            cache_limit = partial["local_cache_limit"]
            if cache_limit is not None:
                current_cache_size = get_cached_history_size()
                print(f"当前缓存大小: {current_cache_size:.2f}MB, 缓存上限: {cache_limit:.2f}MB")
                
                if current_cache_size > cache_limit:
                    cleaned_count = cleanup_cache_by_limit(cache_limit)
                    print(f"缓存清理触发：清理了 {cleaned_count} 个历史记录")
                else:
                    print("当前缓存大小未超过上限，无需清理")
        
        return updated_settings
