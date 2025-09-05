import os
import socket
from typing import Union


class RepeaterClient:
    def __init__(self, socket_path: str = "/tmp/p2p_repeater.sock"):
        self.socket_path = socket_path

    def _send(self, func_type: int, field: str, data: bytes) -> bytes:
        if len(field.encode("utf-8")) > 32:
            raise ValueError("field name too long")
        hdr = bytearray(37)
        hdr[0] = func_type & 0xFF
        field_bytes = field.encode("utf-8")
        hdr[1:1+len(field_bytes)] = field_bytes
        length = len(data)
        hdr[33:37] = length.to_bytes(4, byteorder="big")

        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.connect(self.socket_path)
            s.sendall(hdr)
            if length > 0:
                s.sendall(data)
            # read response header
            rhdr = b""
            while len(rhdr) < 37:
                chunk = s.recv(37 - len(rhdr))
                if not chunk:
                    raise ConnectionError("connection closed while reading header")
                rhdr += chunk
            resp_len = int.from_bytes(rhdr[33:37], byteorder="big")
            resp = b""
            while len(resp) < resp_len:
                chunk = s.recv(resp_len - len(resp))
                if not chunk:
                    raise ConnectionError("connection closed while reading body")
                resp += chunk
            return resp

    def get(self, field: str) -> bytes:
        return self._send(1, field, b"")

    def set(self, field: str, value: Union[str, bytes, int, float, bool]) -> bool:
        if isinstance(value, bool):
            data = b"\x01" if value else b"\x00"
        elif isinstance(value, int):
            # 64-bit big-endian
            data = int(value).to_bytes(8, byteorder="big", signed=True)
        elif isinstance(value, float):
            import struct
            data = struct.pack(">d", value)
        elif isinstance(value, bytes):
            data = value
        else:
            data = str(value).encode("utf-8")
        resp = self._send(2, field, data)
        # setter 约定：len>0 表示成功，或返回具体回显
        return len(resp) >= 0


