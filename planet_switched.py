#!/usr/bin/env python3
"""
Space Exhibit – Planet Video Switcher
M5Dial (USB serial) -> Raspberry Pi 4B -> VLC fullscreen video

Install deps:
    sudo apt install vlc python3-vlc python3-serial
"""

import vlc
import serial
import time
import os
import sys
import threading

# ─── CONFIG ──────────────────────────────────────────────────────────────────

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE   = 115200

VIDEO_DIR   = "/home/raspi/planet"

# Map dial value (1-10) to exact filename inside VIDEO_DIR.
# Edit filenames here – extension can differ per entry.
VIDEO_MAP = {
    1:  "Sun.mp4",
    2:  "Venus.mp4",
    3:  "Moon.mp4",
    4:  "Mars.mp4",
    5:  "JUPITER.mp4",
    6:  "SATURN.mp4",
    7:  "NEPTUNE.mp4",
    8:  "KEPLER-452b.mp4",
    9:  "ANDROMEDA.mp4",
    10: "PILLARS.mp4",
}

# ─── HELPERS ─────────────────────────────────────────────────────────────────

def video_path(index: int) -> str:
    filename = VIDEO_MAP.get(index)
    if not filename:
        return None
    return os.path.join(VIDEO_DIR, filename)


def verify_videos():
    print("Checking video files...")
    all_ok = True
    for index, filename in VIDEO_MAP.items():
        p = os.path.join(VIDEO_DIR, filename)
        status = "OK" if os.path.exists(p) else "MISSING"
        if status == "MISSING":
            all_ok = False
        print(f"  [{index:2}] {filename:30} {status}")
    if not all_ok:
        print("WARNING: Some videos are missing. Those slots will be skipped.")
    print()


# ─── VLC PLAYER ──────────────────────────────────────────────────────────────

class PlanetPlayer:
    def __init__(self):
        self.instance = vlc.Instance(
            "--quiet",
            "--no-osd",
            "--no-video-title-show",
            "--loop",
            "--fullscreen",
        )
        self.player = self.instance.media_player_new()
        self.player.set_fullscreen(True)

        self.current_index = None
        self._lock = threading.Lock()

    def play(self, index: int):
        """Switch to video by dial value. Thread-safe."""
        with self._lock:
            if index == self.current_index:
                # Same video – just restart from beginning
                self.player.set_time(0)
                return

            path = video_path(index)
            if not path:
                print(f"No mapping for dial value: {index}")
                return
            if not os.path.exists(path):
                print(f"File not found: {path}")
                return

            media = self.instance.media_new(path)
            self.player.set_media(media)
            self.player.play()
            self.current_index = index
            print(f"[dial={index}] Playing: {os.path.basename(path)}")

    def stop(self):
        self.player.stop()


# ─── SERIAL READER ───────────────────────────────────────────────────────────

def open_serial(port: str, baud: int, retries: int = 10) -> serial.Serial:
    for attempt in range(retries):
        try:
            s = serial.Serial(port, baud, timeout=1)
            print(f"Serial connected: {port} @ {baud}\n")
            return s
        except serial.SerialException as e:
            print(f"Serial not ready ({e}), retrying... ({attempt + 1}/{retries})")
            time.sleep(2)
    print("Could not open serial port. Exiting.")
    sys.exit(1)


def serial_loop(player: PlanetPlayer, port: str, baud: int):
    """
    Reads dial values from M5Dial forever.
    M5Dial sends: Serial.println(currentValue)  ->  e.g. "5\n"
    Handles USB reconnection automatically.
    """
    ser = open_serial(port, baud)

    while True:
        try:
            raw = ser.readline().decode("utf-8", errors="ignore").strip()
            if not raw:
                continue

            try:
                value = int(raw)
            except ValueError:
                print(f"Unexpected serial data: {repr(raw)}")
                continue

            if value in VIDEO_MAP:
                player.play(value)
            else:
                print(f"Dial value {value} not in VIDEO_MAP, ignoring.")

        except (serial.SerialException, OSError) as e:
            print(f"Serial error: {e} – reconnecting...")
            try:
                ser.close()
            except Exception:
                pass
            time.sleep(2)
            ser = open_serial(port, baud)


# ─── MAIN ────────────────────────────────────────────────────────────────────

def main():
    print("=" * 40)
    print(" Space Exhibit – Planet Switcher")
    print("=" * 40)
    print()

    verify_videos()

    player = PlanetPlayer()
    player.play(1)

    t = threading.Thread(
        target=serial_loop,
        args=(player, SERIAL_PORT, BAUD_RATE),
        daemon=True,
    )
    t.start()

    print("Running. Press Ctrl+C to exit.")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nShutting down.")
        player.stop()
        sys.exit(0)


if __name__ == "__main__":
    main()
