"""
Peer节点模拟数据
"""

from datetime import timedelta
from .store import db, get_current_time
import random

NAT_TYPES = [
    "Full Cone", "Restricted Cone", "Port Restricted Cone", "Symmetric"
]

def init_peer_data():
    peers = {}
    base_time = get_current_time() - timedelta(minutes=5)
    for i in range(1, 3):
        pid = f"peer_{i:03d}"
        peers[pid] = {
            "id": i,
            "pid": pid,
            "is_online": True if i % 7 != 0 else False,
            "download_speed": 0.0, # round(2.0 + (i % 5) * 0.3, 2),
            "upload_speed": 0.0, # round(20.0 + (i % 10) * 1.1, 2),
            "total_download": 0.0, # round(300 + i * 5.3, 2),
            "total_upload": 0.0, # round(2000 + i * 13.7, 2),
            "is_enabled": True if i % 11 != 0 else False,
            "nat_type": NAT_TYPES[i % len(NAT_TYPES)],
            "last_seen": base_time + timedelta(seconds=i * 5),
            "created_at": base_time - timedelta(minutes=i)
        }
    db["peers"] = peers


need_download = 0.0 # MB
had_download = 0.0 # MB

def update_need_download_on_cache_change(rid: str, new_cache_exists: bool, old_cache_exists: bool = None):
    """
    在新建历史记录或缓存状态从否变为是时，更新need_download为目标视频资源的文件大小
    """
    global need_download
    
    print(f"DEBUG: 更新need_download - rid: {rid}, new_cache_exists: {new_cache_exists}, old_cache_exists: {old_cache_exists}")
    
    # 如果没有提供old_cache_exists，从历史记录中查找
    if old_cache_exists is None:
        history = db.get("history", {})
        for record in history.values():
            if record.get("rid") == rid:
                old_cache_exists = record.get("cache_exists", False)
                print(f"DEBUG: 找到旧记录 - old_cache_exists: {old_cache_exists}")
                break
    
    # 获取视频信息
    video_info = get_video_info_by_rid(rid)
    if not video_info:
        print(f"找不到视频信息: {rid}")
        return
    
    video_size_mb = round(video_info["file_size"] / (1024 * 1024), 2)
    print(f"DEBUG: 视频大小: {video_size_mb:.2f}MB")
    
    # 如果找不到历史记录，说明是新视频
    if old_cache_exists is None:
        # 新建历史记录时，无论缓存状态如何，都更新need_download为目标视频大小
        need_download = round(need_download + video_size_mb, 2)
        print(f"新建历史记录，添加需要下载: {video_size_mb:.2f}MB, 总需下载: {need_download:.2f}MB")
        return
    
    # 如果old_cache_exists是False，说明这是新建记录（因为新记录的默认状态是False）
    if old_cache_exists is False and new_cache_exists is False:
        # 新建历史记录时，无论缓存状态如何，都更新need_download为目标视频大小
        need_download = round(need_download + video_size_mb, 2)
        print(f"新建历史记录（状态为False），添加需要下载: {video_size_mb:.2f}MB, 总需下载: {need_download:.2f}MB")
        return
    
    # 检查缓存状态变化
    if not old_cache_exists and new_cache_exists:
        # 缓存状态从否变为是，更新need_download为目标视频大小
        need_download = round(need_download + video_size_mb, 2)
        print(f"缓存状态从否变为是，添加需要下载: {video_size_mb:.2f}MB, 总需下载: {need_download:.2f}MB")
    elif old_cache_exists and not new_cache_exists:
        # 缓存状态从是变为否，增加需要下载的量
        need_download = round(need_download + video_size_mb, 2)
        print(f"缓存状态从是变为否，增加需要下载: {video_size_mb:.2f}MB, 总需下载: {need_download:.2f}MB")
    else:
        # 缓存状态没有变化
        print(f"缓存状态未变化，无需更新need_download")


def get_video_info_by_rid(rid: str):
    """从video_data中获取视频信息"""
    videos = db.get("videos", {})
    return videos.get(rid)

def get_peers_list():
    global need_download, had_download
    print(f"DEBUG: 当前need_download: {need_download:.2f}MB, had_download: {had_download:.2f}MB")
    peers = list(db["peers"].values())
    
    # 确保有至少2个节点
    if len(peers) < 2:
        return peers
    
    # 获取节点1和节点2
    peer1 = peers[0]  # 节点1
    peer2 = peers[1]  # 节点2
    
    # 检查节点状态：只有当两个节点都在线且启用时才进行数据传输
    both_online_and_enabled = (peer1["is_online"] and peer1["is_enabled"] and 
                              peer2["is_online"] and peer2["is_enabled"])
    
    if both_online_and_enabled and need_download > 0:
        # 生成同步的速度值 (5-20 MB/s)
        sync_speed_mb = round(5.0 + random.random() * 15.0, 2)  # 5-20 MB/s
        
        # 节点1：只下载，不上传
        peer1["download_speed"] = sync_speed_mb
        peer1["upload_speed"] = 0.0  # P1不需要上行
        
        # 节点2：只上传，不下载
        peer2["upload_speed"] = sync_speed_mb  # 与P1下载速度相同单位(MB/s)
        peer2["download_speed"] = 0.0  # P2不需要下行
        
        # 模拟数据传输：节点1下载 = 节点2上传
        # 计算本次传输量 (0.5秒的传输量，使用MB单位)
        transfer_amount_mb = min(sync_speed_mb * 0.5, need_download)
        
        # 更新下载总量 (MB) - 节点1
        peer1["total_download"] = round(peer1["total_download"] + transfer_amount_mb, 2)
        
        # 更新上传总量 (MB) - 节点2，与节点1下载量相等
        peer2["total_upload"] = round(peer2["total_upload"] + transfer_amount_mb, 2)
        
        # 更新had_download (MB)
        had_download = round(had_download + transfer_amount_mb, 2)
        
        # 减少need_download (MB)
        need_download = round(max(0, need_download - transfer_amount_mb), 2)
        
        print(f"数据传输: 节点1下载 {transfer_amount_mb:.2f}MB, 节点2上传 {transfer_amount_mb:.2f}MB, 剩余需下载: {need_download:.2f}MB")
        
        # 如果need_download归零，停止下载
        if need_download <= 0:
            print(f"下载完成！need_download已归零，停止下载")
    else:
        # 如果任一节点离线或未启用，或者need_download为0，所有速率设为0
        peer1["download_speed"] = 0.0
        peer1["upload_speed"] = 0.0
        peer2["download_speed"] = 0.0
        peer2["upload_speed"] = 0.0
        
        if need_download <= 0:
            print(f"need_download为0，停止下载")
        else:
            print(f"节点状态: 节点1(在线:{peer1['is_online']}, 启用:{peer1['is_enabled']}), 节点2(在线:{peer2['is_online']}, 启用:{peer2['is_enabled']}) - 速率设为0")
    
    # 处理其他节点
    for i in range(2, len(peers)):
        peer = peers[i]
        if peer["is_online"] and peer["is_enabled"]:
            peer["download_speed"] = round(10.0 + random.random() * 50.0, 2)  # MB/s
            peer["upload_speed"] = round(10.0 + random.random() * 50.0, 2)   # MB/s
            # 其他节点也按0.5秒增长总量
            peer["total_download"] = round(peer["total_download"] + peer["download_speed"] * 0.5, 2)
            peer["total_upload"] = round(peer["total_upload"] + peer["upload_speed"] * 0.5, 2)
        else:
            peer["download_speed"] = 0.0
            peer["upload_speed"] = 0.0
    
    return peers


def get_peer(pid: str):
    return db["peers"].get(pid)


def toggle_peer(pid: str, enabled: bool) -> bool:
    peer = db["peers"].get(pid)
    if not peer:
        return False
    peer["is_enabled"] = enabled
    return True


def get_peer_stats():
    peers = get_peers_list()
    total = len(peers)
    online = sum(1 for p in peers if p["is_online"]) 
    total_down = round(sum(p["download_speed"] for p in peers if p["is_online"]) , 2)
    total_up = round(sum(p["upload_speed"] for p in peers if p["is_online"]) , 2)
    nat = NAT_TYPES[1]
    return {
        "total_peers": total,
        "online_peers": online,
        "total_download_speed": total_down,
        "total_upload_speed": total_up,
        "nat_type": nat
    }
