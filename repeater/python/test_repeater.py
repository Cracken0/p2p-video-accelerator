from repeater.client import RepeaterClient
import binascii

if __name__ == "__main__":
    c = RepeaterClient("/tmp/p2p_repeater.sock")
    rb = c.get("store.used")
    print("GET store.used ->", binascii.hexlify(rb))
    print("SET player.playing True ->", c.set("player.playing", True))
