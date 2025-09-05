from pydantic_settings import BaseSettings, SettingsConfigDict

class Settings(BaseSettings):
    """应用配置"""
    app_name: str = "P2P视频加速器"
    version: str = "1.0.0"
    debug: bool = True

    # 服务器配置
    host: str = "0.0.0.0"
    port: int = 8000

    # CORS配置
    cors_origins: list[str] = ["http://localhost:8080", "http://127.0.0.1:8080"]

    # API配置
    api_prefix: str = "/api"

    # Pydantic Settings v2 配置
    model_config = SettingsConfigDict(env_file=".env", extra="ignore")

settings = Settings()





