"""
Peer节点模拟数据
"""

from datetime import timedelta
from .store import db, get_current_time

NAT_TYPES = [
    "Full Cone", "Restricted Cone", "Port Restricted Cone", "Symmetric"
]

def init_peer_data():
    peers = {}
    base_time = get_current_time() - timedelta(minutes=5)
    for i in range(1, 21):
        pid = f"peer_{i:03d}"
        peers[pid] = {
            "pid": pid,
            "is_online": True if i % 7 != 0 else False,
            "download_speed": round(2.0 + (i % 5) * 0.3, 2),
            "upload_speed": round(20.0 + (i % 10) * 1.1, 2),
            "total_download": round(300 + i * 5.3, 2),
            "total_upload": round(2000 + i * 13.7, 2),
            "is_enabled": True if i % 11 != 0 else False,
            "nat_type": NAT_TYPES[i % len(NAT_TYPES)],
            "last_seen": base_time + timedelta(seconds=i * 5),
            "created_at": base_time - timedelta(minutes=i)
        }
    db["peers"] = peers


def get_peers_list():
    return list(db["peers"].values())


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
