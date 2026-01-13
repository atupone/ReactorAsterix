#!/usr/bin/env python3
import socket
import time

# Minimal ASTERIX Cat 001 packet (Binary)
# [01] Category 1
# [00 09] Length 9 bytes
# [80] FSPEC (Item 1 present)
# [01 02] SAC=1, SIC=2
# [00 00 00] Padding/Reserved
packet = b'\x01\x00\x0f\xF8\x01\x02\x20\x00\x80\x40\x00\x00\x00\x00\x00'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print("Sending ASTERIX packets to 127.0.0.1:4321... (Ctrl+C to stop)")

try:
    while True:
        sock.sendto(packet, ("127.0.0.1", 4321))
        print("Sent 9 bytes...")
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopped.")
