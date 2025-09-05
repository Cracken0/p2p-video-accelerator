from typing import Optional, List, Dict, Any
from app.data.peer_data import get_peers_list, get_peer, toggle_peer, get_peer_stats


class PeerService:
    @staticmethod
    def list_peers() -> List[Dict[str, Any]]:
        return get_peers_list()

    @staticmethod
    def get_stats() -> Dict[str, Any]:
        return get_peer_stats()

    @staticmethod
    def toggle(pid: str, enabled: bool) -> bool:
        return toggle_peer(pid, enabled)
