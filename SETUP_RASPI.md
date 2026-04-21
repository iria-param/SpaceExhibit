# Raspberry Pi Setup

Full setup guide for a **fresh Raspberry Pi OS Lite installation**. Follow every section in order. Don't skip the verify steps — they catch problems early.

> **Before starting:** You need either a USB keyboard + monitor connected directly to the Pi, or a way to SSH in. On a brand new Pi with no Wi-Fi configured yet, use a keyboard directly.

---

## Physical Setup — Before First Boot

### Connect a keyboard
The Pi boots headless (no desktop). On first boot you **must** have a USB keyboard plugged in — there's no other way to type commands before Wi-Fi and SSH are configured.

Plug a standard USB keyboard into any of the Pi's USB-A ports. You don't need a mouse.

### Connect the display
Plug the HDMI cable into **HDMI port 0** (the port closest to the USB-C power port) on the Pi. Connect the other end to your display.

### Do NOT connect the M5Dial yet
Set it aside. Connect the M5Dial only after the Pi is fully configured and rebooted.

### Power on
Plug in the official Raspberry Pi USB-C PSU. The Pi boots to a login prompt. Default credentials:

```
Username: raspi
Password: (whatever you set during imaging)
```

---

## Step 1 — Connect to Wi-Fi

The Pi needs internet for package installation. Run these commands:

```bash
sudo nmcli connection add type wifi ifname wlan0 con-name "YourSSID" ssid "YourSSID"
sudo nmcli connection modify "YourSSID" wifi-sec.key-mgmt wpa-psk
sudo nmcli connection modify "YourSSID" wifi-sec.psk "YourPassword"
sudo nmcli connection up "YourSSID"
```

**Verify it's connected:**

```bash
ping google.com
```

You should see reply lines. Press `Ctrl+C` to stop. If ping fails, double-check the SSID and password — they're case-sensitive.

**Get the Pi's IP address** — you'll need this for SSH:

```bash
hostname -I
```

Write this IP down. Example: `192.168.1.45`

> The Pi will reconnect to this Wi-Fi automatically on every boot. If the exhibit moves to a different venue with a different Wi-Fi, see [OPERATIONS.md — Changing Wi-Fi](./OPERATIONS.md#changing-wi-fi).

---

## Step 2 — Enable SSH

SSH lets you control the Pi remotely from your laptop — you won't need the keyboard after this.

```bash
sudo systemctl enable ssh
sudo systemctl start ssh
```

**Verify:**

```bash
sudo systemctl status ssh
# Should show: active (running)
```

From this point on you can unplug the keyboard and use SSH from your laptop:

```bash
# On your laptop
ssh raspi@<pi-ip>
```

---

## Step 3 — Install Dependencies

These are all the packages the exhibit needs to run.

```bash
sudo apt update
sudo apt install vlc python3-vlc python3-serial greetd labwc
```

**What each package does:**

| Package | Why it's needed |
|---------|----------------|
| `vlc` | The media player engine that plays the videos |
| `python3-vlc` | Python bindings — lets the script control VLC programmatically |
| `python3-serial` | Lets the script read data from the M5Dial over USB serial |
| `greetd` | Session manager — starts the display session on boot (replaces lightdm) |
| `labwc` | Wayland compositor — VLC needs this to open a fullscreen window |

**Verify the critical ones:**

```bash
python3 -c "import vlc; print('vlc ok')"
python3 -c "import serial; print('serial ok')"
```

Both should print `ok`. If either fails, re-run the apt install command.

---

## Step 4 — Transfer the Script

Copy `planet_switched.py` from your laptop to the Pi:

```bash
# Run this on your laptop
scp planet_switched.py raspi@<pi-ip>:/home/raspi/planet_switched.py
```

Verify it arrived:

```bash
# On the Pi
ls -lh /home/raspi/planet_switched.py
```

---

## Step 5 — Transfer Videos

The Pi expects all 10 video files in `/home/raspi/planet/`. Create the folder first:

```bash
mkdir -p /home/raspi/planet
```

Then copy each file from your laptop:

```bash
# Run on your laptop — repeat for each video
scp Sun.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp Venus.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp Moon.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp Mars.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp JUPITER.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp SATURN.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp NEPTUNE.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp "KEPLER-452b.mp4" raspi@<pi-ip>:/home/raspi/planet/
scp ANDROMEDA.mp4 raspi@<pi-ip>:/home/raspi/planet/
scp PILLARS.mp4 raspi@<pi-ip>:/home/raspi/planet/
```

> **Windows users:** Replace `scp` with WinSCP (GUI) or use the same `scp` command in PowerShell/Command Prompt. Replace forward slashes with backslashes for the local path.

**Verify all 10 are there:**

```bash
ls -lh /home/raspi/planet/
# Should list 10 files, all non-zero size
```

> ⚠️ **Filenames are case-sensitive on Linux.** `JUPITER.mp4` and `jupiter.mp4` are different files. The names in `/home/raspi/planet/` must exactly match the names in `VIDEO_MAP` inside `planet_switched.py`.

---

## Step 6 — Test the Script Manually

**Plug in the M5Dial now** via the USB-C → USB-A cable.

Verify the Pi sees it:

```bash
ls /dev/ttyACM*
# Should show: /dev/ttyACM0
```

Now run the script manually to confirm everything works before setting up autostart:

```bash
export DISPLAY=:0
export XDG_RUNTIME_DIR=/run/user/1000
python3 /home/raspi/planet_switched.py
```

You should see:
- The script printing video filenames as it loads
- VLC opening fullscreen on the display
- Turning the M5Dial knob switches videos

Press `Ctrl+C` to stop. If this works, you're ready to set up autostart.

---

## Step 7 — Set Up Autologin

The Pi needs to log in automatically on boot (no password prompt) so the script can start without anyone typing anything.

```bash
sudo raspi-config
```

Navigate: **System Options → S5 Boot / Auto Login → B2 Console Autologin**

Select it, then **Finish** and reboot when prompted.

---

## Step 8 — Configure greetd + labwc (Kiosk Autostart)

This is what makes the exhibit launch automatically on every boot, even without anyone logged in via SSH.

**Why greetd + labwc?**
The script uses VLC, which needs a Wayland display session to open a window. A plain `@reboot` cron job or systemd service doesn't provide a display session — greetd does by starting labwc (a minimal Wayland compositor), and labwc then runs our script via its autostart file.

### Configure greetd

```bash
sudo nano /etc/greetd/config.toml
```

Delete everything in the file and replace with:

```toml
[terminal]
vt = 7

[default_session]
command = "labwc"
user = "raspi"
```

> `vt = 7` means virtual terminal 7. This avoids a conflict with `getty@tty1` which owns VT1. This is why `vt = 1` was tried and abandoned.

Save: `Ctrl+O` → `Enter` → `Ctrl+X`

### Configure labwc autostart

```bash
mkdir -p ~/.config/labwc
nano ~/.config/labwc/autostart
```

Add this single line:

```bash
sleep 2 && python3 /home/raspi/planet_switched.py &
```

> The `sleep 2` gives the Wayland compositor 2 seconds to fully initialise before VLC tries to open a window. Without this, VLC fails with `Failed to open display :0`. If that error still appears, increase to `sleep 4`.

Save and exit.

### Enable greetd

```bash
sudo systemctl enable greetd
sudo systemctl disable lightdm 2>/dev/null
```

---

## Step 9 — Reboot and Verify

```bash
sudo reboot
```

After the Pi restarts (allow ~30 seconds), SSH back in:

```bash
ssh raspi@<pi-ip>
```

Check everything is running:

```bash
sudo systemctl status greetd
# Expect: active (running)

ps aux | grep -E "labwc|planet_switched|vlc"
# Expect: 3 separate processes listed
```

The display should be showing the first planet video. Turn the M5Dial knob — the video should switch.

**Setup is complete.**

---

## Installed Software Summary

| Software | Version check | Installed via |
|----------|-------------|---------------|
| Raspberry Pi OS Lite | `cat /etc/os-release` | Pre-imaged on SD card |
| Python 3 | `python3 --version` | Pre-installed |
| VLC | `vlc --version` | `apt install vlc` |
| python3-vlc | `python3 -c "import vlc"` | `apt install python3-vlc` |
| python3-serial | `python3 -c "import serial"` | `apt install python3-serial` |
| greetd | `systemctl status greetd` | `apt install greetd` |
| labwc | `labwc --version` | `apt install labwc` |
