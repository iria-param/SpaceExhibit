# System Overview

This document explains **what the exhibit is, how it works, and why it's built the way it is**. Read this before touching anything else.

---

## What the Exhibit Does

A visitor walks up and rotates a physical knob. The screen immediately switches to a full-screen video of the corresponding planet — no buttons, no menus, no touch screen. The M5Dial knob is the entire interface.

There are 10 planets/objects in the sequence:

| Knob position | Object |
|---------------|--------|
| 1 | Sun |
| 2 | Venus |
| 3 | Moon |
| 4 | Mars |
| 5 | Jupiter |
| 6 | Saturn |
| 7 | Neptune |
| 8 | Kepler-452b |
| 9 | Andromeda Galaxy |
| 10 | Pillars of Creation |

The knob wraps — turning past 10 goes back to 1, and turning back past 1 goes to 10.

---

## System Architecture

```
[ VISITOR ]
    │
    │  Rotates knob
    ▼
┌─────────────────────────────┐
│         M5 DIAL             │
│                             │
│  Hardware:                  │
│    ESP32-S3 MCU             │
│    1.28" round TFT 240×240  │
│    Rotary encoder           │
│    Push button              │
│                             │
│  Firmware: SpaceExhibit.ino │
│    Counts encoder ticks     │
│    Every 4 ticks = 1 scene  │  ← SKIP constant
│    Draws planet art on TFT  │
│    Sends index over serial  │
│                             │
│    Serial.println(index+1)  │  e.g. sends "5\n" for Jupiter
└────────────┬────────────────┘
             │
             │  USB-C → USB-A cable
             │  Purpose: power (5V) + data (UART)
             │  Protocol: 115200 baud, 8N1
             │  Device node on Pi: /dev/ttyACM0
             │
             ▼
┌─────────────────────────────┐
│      RASPBERRY PI 4B        │
│                             │
│  Hardware:                  │
│    BCM2711 SoC, 1 GB RAM    │
│    MicroSD (OS + videos)    │
│    USB-A ports              │
│    HDMI output              │
│                             │
│  OS: RPi OS Lite            │
│    Debian Trixie (13)       │
│    Headless — no desktop    │
│                             │
│  Session: greetd → labwc    │
│    greetd starts on boot    │  ← replaces lightdm
│    launches labwc (Wayland) │  ← compositor for VLC window
│    labwc runs autostart     │  ← which launches the script
│                             │
│  Script: planet_switched.py │
│    Opens /dev/ttyACM0       │
│    Reads index from serial  │
│    Maps index → filename    │
│    Tells VLC to play it     │
│    Auto-reconnects if USB   │
│    drops and comes back     │
└────────────┬────────────────┘
             │
             │  HDMI cable
             │
             ▼
┌─────────────────────────────┐
│          DISPLAY            │
│  Full-screen planet video   │
│  No UI chrome, no cursor    │
└─────────────────────────────┘
```

---

## Why These Components

### Why M5Dial?
It has a built-in rotary encoder, a round colour display, and an ESP32-S3 — all in one unit. The display shows matching planet artwork so visitors get feedback even before looking at the main screen. It communicates over USB serial, which is simple and reliable.

### Why Raspberry Pi 4B?
Powerful enough to decode H.264 video smoothly via VLC. Has USB ports for the M5Dial and HDMI for the display. Runs a full Linux OS which makes file management, SSH, and scripting straightforward.

### Why VLC?
VLC has solid Python bindings (`python3-vlc`), can play fullscreen with no UI chrome, and handles video switching by swapping media objects. It's stable and well-documented.

### Why greetd + labwc instead of a plain systemd service?
VLC needs a Wayland compositor to open a window. A plain systemd service doesn't provide a display session — labwc does. greetd is a minimal session manager that starts labwc cleanly on boot without a full desktop environment.

### Why not mpv, --loop flag, or other approaches?
See [TROUBLESHOOTING.md — Approaches That Were Abandoned](./TROUBLESHOOTING.md#approaches-that-were-abandoned).

---

## Signal Flow Step by Step

1. Visitor rotates the M5Dial knob one detent
2. The encoder ISR in the firmware fires and increments a tick counter
3. Every **4 ticks** (`SKIP = 4`) the scene index changes by 1
4. The firmware draws the new planet's artwork on the M5Dial's TFT screen
5. The firmware calls `Serial.println(currentIndex + 1)` — sends e.g. `"5\n"` over USB
6. The Pi's `planet_switched.py` reads that line from `/dev/ttyACM0`
7. The script looks up `VIDEO_MAP[5]` → `"JUPITER.mp4"`
8. The script calls `vlc.set_media("/home/raspi/planet/JUPITER.mp4")` then `player.play()`
9. VLC switches to the new video fullscreen — the old video stops immediately
10. The new video plays and loops continuously until the next knob turn

---

## Hardware & Power

| Component | Model / Spec | Power requirement |
|-----------|-------------|-------------------|
| Rotary controller | M5Stack M5Dial (ESP32-S3) | 5V via USB — **bus-powered from Pi's USB-A port. No separate PSU needed.** |
| Single-board computer | Raspberry Pi 4B, 1 GB RAM | **5V / 3A minimum via USB-C. Use the official Raspberry Pi PSU.** Underpowering causes random crashes and SD card corruption. |
| USB cable (M5Dial ↔ Pi) | USB-C to USB-A, **data-capable** | Carries 5V power to M5Dial + UART data in both directions. Charge-only cables have no data lines and will not work. |
| Display | Any HDMI monitor | Separate mains via monitor's own PSU |
| Storage | MicroSD, Class 10 / A1, 16 GB min | — |

> **Total system draw:** ~3–5W idle, up to ~8W under VLC load. A standard 15W USB-C PSU is more than sufficient for the Pi.

---

## File Reference

| File | Location on Pi | Purpose |
|------|---------------|---------|
| `planet_switched.py` | `/home/raspi/planet_switched.py` | Main application — runs on every boot |
| `Sun.mp4` … `PILLARS.mp4` | `/home/raspi/planet/` | The 10 planet videos |
| `config.toml` | `/etc/greetd/config.toml` | Boot session configuration |
| `autostart` | `~/.config/labwc/autostart` | Launches the script after Wayland starts |
