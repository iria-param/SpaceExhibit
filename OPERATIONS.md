# Operations Guide

Everything you need for day-to-day management of the exhibit — connecting remotely, changing videos, switching Wi-Fi networks, and what to do when you need physical access.

---

## Accessing the Pi

### Remote Access via SSH

SSH is how you control the Pi without being physically present. The Pi must be on the same Wi-Fi network as your laptop.

**From Mac / Linux:**

```bash
ssh raspi@<pi-ip>
```

**From Windows (PowerShell or Command Prompt):**

```bash
ssh raspi@<pi-ip>
```

If you don't know the Pi's IP address:
- **If you have a keyboard connected:** log in directly and run `hostname -I`
- **Check your router's admin page** — look for a device named `raspi`
- **From another device on the same network:** run `ping raspi.local` — this works if mDNS is enabled

> If SSH is not responding, see [TROUBLESHOOTING.md — Can't SSH In](./TROUBLESHOOTING.md#cant-ssh-in).

---

### Connecting a Keyboard Directly

Sometimes you won't be able to SSH — the Pi may have lost Wi-Fi, SSH may not be running, or you may be setting up at a new venue. In those cases you need a physical keyboard.

**What you need:**
- Any standard USB keyboard
- Optionally a USB mouse (not required — everything can be done with keyboard)

**How to connect:**
1. Plug the USB keyboard into any of the Pi's USB-A ports
2. If the exhibit is running, press `Ctrl+C` in the VLC session won't work — the script is running in the background under greetd. Instead, switch to a different virtual terminal:

```
Press: Ctrl + Alt + F2
```

This switches you to VT2 — a clean login prompt, away from the kiosk session on VT7.

3. Log in with your Pi credentials
4. Do your work
5. When done, reboot to return to the kiosk:

```bash
sudo reboot
```

> **Why Ctrl+Alt+F2?** The kiosk runs on virtual terminal 7 (VT7). VT2 is a separate terminal session that runs independently — switching to it doesn't stop the exhibit, it just gives you a separate login shell to work in.

---

## Changing Wi-Fi

When moving the exhibit to a new venue, or if the venue's Wi-Fi credentials change:

### Add a new network

```bash
sudo nmcli connection add type wifi ifname wlan0 con-name "NewSSID" ssid "NewSSID"
sudo nmcli connection modify "NewSSID" wifi-sec.key-mgmt wpa-psk
sudo nmcli connection modify "NewSSID" wifi-sec.psk "NewPassword"
sudo nmcli connection up "NewSSID"
```

**Verify it connected:**

```bash
ping google.com
hostname -I   # shows the new IP address
```

The new network will connect automatically on future boots. Old network profiles are kept — the Pi will try them in priority order.

### List saved networks

```bash
nmcli connection show
```

### Remove an old network

```bash
sudo nmcli connection delete "OldSSID"
```

### Check current connection

```bash
nmcli device status
# Look for wlan0 — should say "connected"
```

---

## Changing or Adding Videos

### Replace an existing video

If you're swapping a planet video for a new version **with the same filename**, just copy the new file over:

```bash
# From your laptop
scp NewJupiter.mp4 raspi@<pi-ip>:/home/raspi/planet/JUPITER.mp4
```

Then restart the exhibit:

```bash
sudo systemctl restart greetd
```

### Replace with a different filename

If the new file has a different name:

1. Copy the new file to the Pi:

```bash
scp NewFile.mp4 raspi@<pi-ip>:/home/raspi/planet/NewFile.mp4
```

2. Edit `VIDEO_MAP` in the script:

```bash
nano /home/raspi/planet_switched.py
```

Find the `VIDEO_MAP` section and update the relevant entry. Example — changing Jupiter's file:

```python
VIDEO_MAP = {
    ...
    5:  "NewFile.mp4",   # was JUPITER.mp4
    ...
}
```

Save: `Ctrl+O` → `Enter` → `Ctrl+X`

3. Restart:

```bash
sudo systemctl restart greetd
```

### Add a new planet (11th video etc.)

1. Copy the new video to the Pi:

```bash
scp NewPlanet.mp4 raspi@<pi-ip>:/home/raspi/planet/NewPlanet.mp4
```

2. Edit `planet_switched.py` — add the new entry to `VIDEO_MAP`:

```bash
nano /home/raspi/planet_switched.py
```

```python
VIDEO_MAP = {
    1:  "Sun.mp4",
    ...
    10: "PILLARS.mp4",
    11: "NewPlanet.mp4",   # ← add this
}
```

3. Re-flash the M5Dial firmware with `TOTAL = 11` — see [SETUP_M5DIAL.md](./SETUP_M5DIAL.md).

4. Restart the Pi script:

```bash
sudo systemctl restart greetd
```

### Pull videos back from Pi to your laptop

If you need to retrieve videos from the Pi (e.g. the source files are only on the Pi):

```bash
# Run on Pi — starts a temporary web server
python3 -m http.server 8000 --directory /home/raspi/planet
```

Then open `http://<pi-ip>:8000` in a browser on your laptop. You can download files directly from there.

Press `Ctrl+C` on the Pi when done.

---

## Starting, Stopping, Restarting

### Restart the exhibit

Use this if videos aren't playing, the script has crashed, or you've made changes:

```bash
sudo systemctl restart greetd
```

This stops everything (labwc, VLC, the Python script) and starts fresh. Takes about 5 seconds.

### Stop the exhibit

```bash
sudo systemctl stop greetd
```

This stops playback. The display will go blank or show whatever the compositor leaves behind.

### Start the exhibit (after stopping)

```bash
sudo systemctl start greetd
```

### Check if the exhibit is running

```bash
sudo systemctl status greetd
```

Look for `active (running)`. If it says `failed` or `inactive`, something went wrong — see [TROUBLESHOOTING.md](./TROUBLESHOOTING.md).

You can also check the individual processes:

```bash
ps aux | grep -E "labwc|planet_switched|vlc"
```

You should see 3 lines — one for labwc, one for planet_switched.py, one for vlc.

---

## Safe Shutdown

**Always shut down properly** before cutting power to the Pi. Cutting power mid-run can corrupt the SD card and require a full OS re-flash.

```bash
sudo shutdown -h now
```

Wait for the Pi's **green activity LED** to stop blinking completely — then it's safe to cut power.

**Power off sequence:**
1. Run `sudo shutdown -h now`
2. Wait for activity LED to go dark
3. Turn off display
4. Turn off Pi PSU
5. Turn off wall switch

---

## Routine Health Checks

Run these periodically to catch problems before they become failures:

```bash
# All 3 processes running?
ps aux | grep -E "labwc|planet_switched|vlc"

# Any errors in recent logs?
sudo journalctl -u greetd -n 50

# M5Dial still connected?
ls /dev/ttyACM*

# All 10 videos still there?
ls -lh /home/raspi/planet/

# CPU temperature OK? (should be below 70°C)
vcgencmd measure_temp

# Disk space OK? (should be >20% free)
df -h

# RAM OK?
free -m
```

---

## Useful File Locations

| What | Where |
|------|-------|
| Main script | `/home/raspi/planet_switched.py` |
| Video files | `/home/raspi/planet/` |
| greetd config | `/etc/greetd/config.toml` |
| labwc autostart | `~/.config/labwc/autostart` |
| System logs | `sudo journalctl -u greetd -n 50` |
