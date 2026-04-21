# Prakrithi Space Exhibit

**M5Dial rotary controller → Raspberry Pi 4B → VLC fullscreen video switcher**

---

## Repository Structure

```
/
├── README.md                  ← You are here — setup, ops, troubleshooting
├── SpaceExhibit.ino           ← M5Dial Arduino firmware
└── planet_switched.py         ← Raspberry Pi Python switcher script
```

---

## How It Works

```
[ VISITOR ]
    │
    │  Rotates knob
    ▼
┌─────────────────────┐
│      M5 DIAL        │  ESP32-S3 MCU
│   SpaceExhibit.ino  │  1.28" round TFT — shows planet art
│                     │  Rotary encoder + push button
│   SKIP  = 4 ticks   │
│   TOTAL = 10 videos │  Serial.println(currentIndex + 1)
└────────┬────────────┘
         │
         │  USB-C → USB-A  (5V bus power + UART data)
         │  115200 baud, 8N1
         ▼
┌─────────────────────┐  /dev/ttyACM0
│   RASPBERRY PI 4B   │
│   1 GB RAM          │  planet_switched.py
│   RPi OS Lite       │    reads serial line
│   Debian Trixie 13  │    maps index → filename
│   greetd + labwc    │    calls vlc.set_media() + play()
└────────┬────────────┘    reconnects on USB drop automatically
         │
         │  HDMI
         ▼
┌─────────────────────┐
│      DISPLAY        │  Full-screen planet video, no UI chrome
└─────────────────────┘
```

### Planet Index Mapping

| Index | Planet | Video File |
|-------|--------|------------|
| 1 | Sun | `Sun.mp4` |
| 2 | Venus | `Venus.mp4` |
| 3 | Moon | `Moon.mp4` |
| 4 | Mars | `Mars.mp4` |
| 5 | Jupiter | `JUPITER.mp4` |
| 6 | Saturn | `SATURN.mp4` |
| 7 | Neptune | `NEPTUNE.mp4` |
| 8 | Kepler-452b | `KEPLER-452b.mp4` |
| 9 | Andromeda | `ANDROMEDA.mp4` |
| 10 | Pillars of Creation | `PILLARS.mp4` |

> **Note:** Filenames are case-sensitive on Linux. `JUPITER.mp4` ≠ `jupiter.mp4`.

---

## Hardware & Power Requirements

| Component | Spec | Power |
|-----------|------|-------|
| M5Stack M5Dial | ESP32-S3, 1.28" TFT 240×240, rotary encoder | 5V via USB (bus-powered from Pi) |
| Raspberry Pi 4B | 1 GB RAM, BCM2711, RPi OS Lite | **5V / 3A minimum** — use official PSU |
| USB-C PSU | Official Raspberry Pi 5V/3A PSU | Mains (230V/50Hz or 110V/60Hz) |
| USB-C → USB-A cable | **Data-capable** — charge-only cables have no data lines | Carries 5V to M5Dial |
| HDMI monitor | Any HDMI display | Separate mains via monitor PSU |
| MicroSD card | Class 10 / A1, 16 GB minimum | — |

> Total system draw (Pi + M5Dial): ~3–5W idle, up to ~8W under VLC load.

---

## Part 1 — Flash the M5Dial

> **Do this from a Windows / Mac / Linux machine. The Pi is not involved.**

### Prerequisites

| Tool | Source |
|------|--------|
| Arduino IDE 2.x | [arduino.cc/en/software](https://www.arduino.cc/en/software) |
| M5Stack board package | Arduino Boards Manager (URL below) |
| M5Dial library | Arduino Library Manager → search `M5Dial` |
| CP210x / CH340 USB driver | Auto-installs Win 10+, or manufacturer site |

**Add this URL to Boards Manager** (Arduino IDE → File → Preferences):
```
https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
```

Then: `Tools → Board → Boards Manager → search M5Stack → Install`

### Flash Steps

```
1. Tools → Board → M5Stack → M5Dial
2. Tools → Port → (COM port showing CP210x or CH340)
3. Open SpaceExhibit.ino
4. Click Upload — wait for "Done uploading"
5. Tools → Serial Monitor → 115200 baud
6. Turn the knob → numbers 1–10 should print
```

### Tunable Constants

```cpp
#define SKIP  4   // encoder ticks per scene change — lower = more sensitive (try 3–6)
#define TOTAL 10  // number of videos — update if you add/remove planets
```

---

## Part 2 — Set Up the Raspberry Pi

### 2.1 Wi-Fi

```bash
sudo nmcli connection add type wifi ifname wlan0 con-name "YourSSID" ssid "YourSSID"
sudo nmcli connection modify "YourSSID" wifi-sec.key-mgmt wpa-psk
sudo nmcli connection modify "YourSSID" wifi-sec.psk "YourPassword"
sudo nmcli connection up "YourSSID"
ping google.com   # verify
```

### 2.2 SSH

```bash
sudo systemctl enable ssh && sudo systemctl start ssh
hostname -I   # note this IP
```

### 2.3 Install Dependencies

```bash
sudo apt update
sudo apt install vlc python3-vlc python3-serial greetd labwc

# Verify
python3 -c "import vlc; print('vlc ok')"
python3 -c "import serial; print('serial ok')"
```

### 2.4 Transfer Videos

```bash
# From Windows — repeat for all 10 files
scp C:\Users\YourName\Videos\Sun.mp4 raspi@<pi-ip>:/home/raspi/planet/

# Verify on Pi
ls -lh /home/raspi/planet/   # should list all 10 files
```

> To pull files back to Windows: run `python3 -m http.server 8000 --directory /home/raspi/planet` on the Pi, then open `http://<pi-ip>:8000` in a browser.

### 2.5 Deploy the Switcher Script

```bash
# Copy planet_switched.py to the Pi
scp planet_switched.py raspi@<pi-ip>:/home/raspi/planet_switched.py

# Test it manually first
export DISPLAY=:0 && export XDG_RUNTIME_DIR=/run/user/1000
python3 /home/raspi/planet_switched.py
# VLC should open. Turn the knob — video should switch. Ctrl+C to stop.
```

### 2.6 Autologin

```bash
sudo raspi-config
# System Options → S5 Boot/Auto Login → B2 Console Autologin
```

### 2.7 Kiosk Autostart — greetd + labwc

**Configure greetd** (`sudo nano /etc/greetd/config.toml`):

```toml
[terminal]
vt = 7

[default_session]
command = "labwc"
user = "raspi"
```

**Configure labwc autostart:**

```bash
mkdir -p ~/.config/labwc
nano ~/.config/labwc/autostart
```

Add this single line:

```bash
sleep 2 && python3 /home/raspi/planet_switched.py &
```

**Enable greetd:**

```bash
sudo systemctl enable greetd
sudo systemctl disable lightdm 2>/dev/null
sudo reboot
```

**Verify after reboot:**

```bash
sudo systemctl status greetd                        # expect: active (running)
ps aux | grep -E "labwc|planet_switched|vlc"        # expect: 3 processes
```

---

## Part 3 — Day-to-Day Operations

### Start / Stop / Restart

| Action | Command |
|--------|---------|
| Restart everything | `sudo systemctl restart greetd` |
| Stop the exhibit | `sudo systemctl stop greetd` |
| Check status | `sudo systemctl status greetd` |
| View logs | `sudo journalctl -u greetd -n 50` |
| Safe shutdown | `sudo shutdown -h now` |

> ⚠️ **Never cut power while the Pi is running** — it will corrupt the SD card and require a full OS re-flash.

### Health Checks

| Check | Command | Expected |
|-------|---------|----------|
| All processes running | `ps aux \| grep -E "labwc\|planet_switched\|vlc"` | 3 lines |
| M5Dial on USB | `ls /dev/ttyACM*` | `/dev/ttyACM0` |
| No errors in logs | `sudo journalctl -u greetd -n 50` | No ERROR lines |
| All videos present | `ls -lh /home/raspi/planet/ \| wc -l` | 11 (10 files + header) |
| CPU temperature | `vcgencmd measure_temp` | Below 70°C |
| Disk space | `df -h` | >20% free on `/` |
| RAM | `free -m` | Adequate free |

### Adding or Replacing a Video

```bash
# 1. Copy new file to Pi (filename must match VIDEO_MAP — case-sensitive)
scp NewPlanet.mp4 raspi@<pi-ip>:/home/raspi/planet/

# 2. Edit VIDEO_MAP in planet_switched.py if filename changed
nano /home/raspi/planet_switched.py

# 3. Restart
sudo systemctl restart greetd
```

---

## Part 4 — Troubleshooting

### Fault Table

| Symptom | Most likely cause | Fix |
|---------|-------------------|-----|
| Black screen, nothing plays | greetd not running | `sudo systemctl restart greetd` |
| Video plays but knob does nothing | M5Dial not on USB | `ls /dev/ttyACM*` — re-seat cable, reboot |
| Video switches too easily / randomly | SKIP too low in firmware | Increase SKIP to 5–6, re-flash M5Dial |
| Script crashes on startup | Missing package or wrong filename | `sudo journalctl -u greetd -n 50` — read line above crash |
| Video freezes on last frame | Wrong VLC loop method | Script uses `--loop` flag — switch to `MediaPlayerEndReached` event if issue recurs |
| Can't SSH in | Wi-Fi dropped or SSH stopped | Plug in keyboard → `sudo systemctl start ssh` → `nmcli connection up <SSID>` |
| Pi very hot (>80°C) | No airflow / heatsink | `vcgencmd measure_temp` — add heatsink, improve ventilation |
| Serial port exists but no data | Baud mismatch or charge-only cable | Confirm `BAUD_RATE = 115200`, replace cable with data-capable one |

### Read the Log First

```bash
sudo journalctl -u greetd -n 50
```

The error is almost always the line just before where the script stopped.

| Log message | Fix |
|-------------|-----|
| `No module named vlc` | `sudo apt install python3-vlc` |
| `No module named serial` | `sudo apt install python3-serial` |
| `could not open /dev/ttyACM0` | Re-seat USB cable → `ls /dev/ttyACM*` → reboot if still absent |
| `VLC cannot open file` | Filename in `VIDEO_MAP` must match file on disk exactly (case-sensitive) |
| `Failed to open display :0` | Change `sleep 2` to `sleep 4` in labwc autostart |

### Approaches That Were Tried and Abandoned

| Approach | Why abandoned |
|----------|---------------|
| mpv with IPC socket | Video switching not smooth enough |
| VLC `--loop` flag | Unreliable loop after `set_media()` swap — current script uses `--loop` via VLC instance args which works; `MediaPlayerEndReached` is the alternative if issues arise |
| `kiosk.service` (plain systemd) | labwc needs a TTY seat — can't launch from plain systemd |
| greetd on `vt=1` | Conflicts with getty@tty1 — must use `vt=7` |
| MJPEG preload to RAM | Not needed — VLC single-instance switching is fast enough |

---

## Script Configuration Reference

### `planet_switched.py`

| Variable | Default | Description |
|----------|---------|-------------|
| `SERIAL_PORT` | `/dev/ttyACM0` | M5Dial USB device node — verify with `ls /dev/ttyACM*` |
| `BAUD_RATE` | `115200` | Must match `Serial.begin()` in firmware |
| `VIDEO_DIR` | `/home/raspi/planet` | Absolute path to video folder |
| `VIDEO_MAP` | `{1: "Sun.mp4", ...}` | Dict mapping dial index to filename |

### `SpaceExhibit.ino`

| Constant | Default | Description |
|----------|---------|-------------|
| `SKIP` | `4` | Encoder ticks per scene change — lower = more sensitive (try 3–6) |
| `TOTAL` | `10` | Total number of videos — update if planets added/removed |
