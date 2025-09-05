import os
import sys
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from app.config import settings
from app.api import videos, search, peers, history, settings as settings_api

# 为了能在不安装包的情况下导入本仓库的 repeater/python
_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../..'))
_REPEATER_PY = os.path.join(_ROOT, 'repeater', 'python')
if _REPEATER_PY not in sys.path:
    sys.path.append(_REPEATER_PY)

# 创建FastAPI应用
app = FastAPI(
    title=settings.app_name,
    version=settings.version,
    debug=settings.debug
)

# 配置CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 注册API路由
app.include_router(videos.router, prefix=settings.api_prefix)
app.include_router(search.router, prefix=settings.api_prefix)
app.include_router(peers.router, prefix=settings.api_prefix)
app.include_router(history.router, prefix=settings.api_prefix)
app.include_router(settings_api.router, prefix=settings.api_prefix)

@app.get("/")
async def root():
    """根路径"""
    return {
        "message": "P2P视频加速器API服务",
        "version": settings.version,
        "docs": "/docs"
    }

@app.get("/health")
async def health_check():
    """健康检查"""
    return {"status": "healthy"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "app.main:app",
        host=settings.host,
        port=settings.port,
        reload=settings.debug
    )





