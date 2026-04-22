# Prakrithi Space Exhibit

> An interactive space exhibit where visitors rotate a physical knob to explore planets — from the Sun to the Pillars of Creation.

A visitor turns the **M5Dial** knob. The **Raspberry Pi** receives that signal and instantly switches to the matching full-screen planet video on the display. That's the whole system.

---

## Documentation

| File | What's in it |
|------|-------------|
| [OVERVIEW.md](./OVERVIEW.md) | How the system works, architecture diagram, hardware specs, power requirements |
| [SETUP_M5DIAL.md](./SETUP_M5DIAL.md) | How to flash the M5Dial firmware from Arduino IDE |
| [SETUP_RASPI.md](./SETUP_RASPI.md) | Full Raspberry Pi setup from scratch — Wi-Fi, dependencies, autostart |
| [OPERATIONS.md](./OPERATIONS.md) | Day-to-day — SSH access, changing videos, connecting a keyboard, Wi-Fi changes |
| [TROUBLESHOOTING.md](./TROUBLESHOOTING.md) | Fault table, log reading, known dead ends |

---

## Repository Structure

```
/
├── README.md                ← This file
├── SpaceExhibit.ino         ← M5Dial Arduino firmware
├── planet_switched.py       ← Raspberry Pi Python switcher script
└── docs/
    ├── OVERVIEW.md
    ├── SETUP_M5DIAL.md
    ├── SETUP_RASPI.md
    ├── OPERATIONS.md
    └── TROUBLESHOOTING.md
```

---

## Quick Start

Already set up and just need to do something?

- **SSH into the Pi** → [OPERATIONS.md — Remote Access](./OPERATIONS.md#remote-access-via-ssh)
- **Swap a video** → [OPERATIONS.md — Changing Videos](./OPERATIONS.md#changing-or-adding-videos)
- **Something broke** → [TROUBLESHOOTING.md](./TROUBLESHOOTING.md)
- **Fresh install** → [SETUP_RASPI.md](./SETUP_RASPI.md)

## IP Address

WiFi : Parsec-Guest | Passwd : Parsec@Guest
Raspi IP 1 : 192.168.30.254
Rapsi IP 2 : 192.168.30.14
