# Troubleshooting

**Start here when something goes wrong.** Read the log first — it almost always tells you exactly what happened.

---

## Read the Log First — Always

Before touching anything, run this:

```bash
sudo journalctl -u greetd -n 50
```

This shows the last 50 lines of the greetd service log — which includes the Python script's output and any error messages. The error is almost always the line just before where the script stopped.

---

## Fault Table

| Symptom | Most likely cause | First thing to try |
|---------|-------------------|-------------------|
| Display is black, nothing plays | greetd not running or crashed | `sudo systemctl restart greetd` |
| Video plays but knob does nothing | M5Dial not detected on USB | `ls /dev/ttyACM*` — see below |
| Video switches too easily / accidentally | SKIP value too low in firmware | Increase SKIP, re-flash M5Dial |
| Video jumps around randomly | Loose USB cable causing reconnects | Re-seat the USB-C cable on M5Dial end |
| Script crashes immediately on startup | Missing Python package or wrong video filename | Read the log — `sudo journalctl -u greetd -n 50` |
| Video plays once then freezes on last frame | VLC loop not configured correctly | Check script is using `--loop` in VLC instance args |
| Video switches but with a long delay | Script is re-opening VLC each time instead of swapping media | Verify `planet_switched.py` is the correct version — it should reuse one VLC instance |
| Can't SSH into the Pi | Wi-Fi dropped or SSH not running | Connect keyboard directly — see below |
| Pi is very hot (>80°C) | No airflow or missing heatsink | Add heatsink, improve ventilation |
| Serial port exists but no data arrives | Baud mismatch or charge-only cable | Confirm `BAUD_RATE = 115200`, swap cable |
| M5Dial screen is blank | Firmware not flashed or USB not supplying power | Re-flash firmware, check USB cable |
| Knob has to turn very far to change video | SKIP value too high | Decrease SKIP, re-flash M5Dial |

---

## Common Log Errors and Fixes

Run `sudo journalctl -u greetd -n 50` and look for these:

| What the log says | What it means | Fix |
|-------------------|--------------|-----|
| `No module named vlc` | python3-vlc not installed | `sudo apt install python3-vlc` |
| `No module named serial` | python3-serial not installed | `sudo apt install python3-serial` |
| `could not open /dev/ttyACM0` | M5Dial not detected on USB | Re-seat the USB cable → `ls /dev/ttyACM*` → reboot if still absent |
| `[Errno 2] No such file or directory: '/home/raspi/planet/JUPITER.mp4'` | Filename mismatch between VIDEO_MAP and actual file | `ls /home/raspi/planet/` and compare exactly to VIDEO_MAP in the script |
| `Failed to open display :0` | Wayland compositor wasn't ready when VLC started | Change `sleep 2` to `sleep 4` in `~/.config/labwc/autostart` |
| `serial.serialutil.SerialException` | Serial read failed mid-run | Usually a cable issue — the script auto-reconnects, but if it keeps happening, replace the USB cable |
| `Unit greetd.service entered failed state` | greetd itself crashed | `sudo systemctl restart greetd` — then check logs for root cause |

---

## Specific Scenarios

### M5Dial not detected

**Symptom:** Video plays fine on last selected planet, but turning the knob does nothing.

**Diagnosis:**

```bash
ls /dev/ttyACM*
# If this returns nothing, the Pi doesn't see the M5Dial at all
```

**Steps:**

1. Re-seat the USB-C cable at the M5Dial end — pull it out fully and push it back in firmly
2. Run `ls /dev/ttyACM*` again
3. If still nothing: `sudo reboot` — sometimes a fresh boot re-enumerates USB devices
4. If still nothing after reboot: try a different USB-C cable. **The cable must be data-capable** — charge-only cables have no data lines and will not work even though they supply power
5. If a different cable doesn't help: plug the M5Dial into a laptop and open Arduino IDE Serial Monitor at 115200 baud — if you see numbers when you turn the knob, the M5Dial is fine and the Pi's USB port may be faulty

---

### Can't SSH In

**Symptom:** `ssh raspi@<ip>` times out or refuses connection.

**Diagnosis and fix:**

1. **Connect a USB keyboard** to the Pi directly (any USB-A port)
2. Press `Ctrl + Alt + F2` to switch to VT2 (away from the kiosk on VT7)
3. Log in with your Pi credentials
4. Check SSH:

```bash
sudo systemctl status ssh
# If not running:
sudo systemctl start ssh
sudo systemctl enable ssh
```

5. Check Wi-Fi:

```bash
nmcli device status
# If wlan0 shows "disconnected":
sudo nmcli connection up "YourSSID"
```

6. Get the current IP:

```bash
hostname -I
```

7. Try SSH again from your laptop using the new IP

---

### Exhibit Doesn't Start on Boot

**Symptom:** Pi boots but the display stays blank or shows a terminal instead of a video.

**Diagnosis:**

```bash
sudo systemctl status greetd
# If failed: check the error
sudo journalctl -u greetd -n 30
```

**Common causes:**

- **greetd not enabled:** `sudo systemctl enable greetd && sudo reboot`
- **labwc autostart file missing:** Check `cat ~/.config/labwc/autostart` — should contain `sleep 2 && python3 /home/raspi/planet_switched.py &`
- **Script has an error:** Run the script manually to see the error:

```bash
export DISPLAY=:0 && export XDG_RUNTIME_DIR=/run/user/1000
python3 /home/raspi/planet_switched.py
```

- **VLC not installed:** `sudo apt install vlc python3-vlc`

---

### Video Doesn't Switch When Knob Turns

**Symptom:** The M5Dial screen changes (planet art updates), but the display stays on the same video.

**This means the M5Dial is working but the Pi isn't receiving the signal.**

**Diagnosis:**

```bash
# Check the serial port exists
ls /dev/ttyACM*

# Test serial live — turn the knob while watching
python3 -c "
import serial
s = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
while True:
    line = s.readline().decode().strip()
    if line: print('Received:', line)
"
```

If you see numbers printing when you turn the knob, the serial link is fine — the issue is in the script or VLC. Check the log.

If you see nothing, the serial link is broken — re-seat the USB cable or replace it.

---

### Video Plays Once Then Freezes

**Symptom:** A planet video plays through to the end and then freezes on the last frame instead of looping.

**Cause:** The VLC instance is started with `--loop` in the instance arguments. This should loop correctly. If it doesn't:

1. Check the VLC instance args in `planet_switched.py`:

```python
self.instance = vlc.Instance(
    "--quiet",
    "--no-osd",
    "--no-video-title-show",
    "--loop",          # ← this must be here
    "--fullscreen",
)
```

2. If `--loop` is present and it's still not looping after `set_media()` is called, the alternative is to use the `MediaPlayerEndReached` event:

```python
events = self.player.event_manager()
events.event_attach(vlc.EventType.MediaPlayerEndReached, self._on_end_reached)

def _on_end_reached(self, event):
    self.player.set_time(0)
    self.player.play()
```

---

## Approaches That Were Abandoned

These were tested during development and ruled out. Don't spend time re-trying them.

| Approach | Why it failed | What to use instead |
|----------|--------------|---------------------|
| **mpv with IPC socket** | Video switching wasn't smooth enough — there was a visible gap between videos | VLC with `set_media()` |
| **VLC `--loop` flag alone (without instance args)** | Doesn't reliably loop after `set_media()` swap when passed as a media option | Pass `--loop` to the VLC Instance constructor, not to the media |
| **`kiosk.service` (plain systemd service)** | labwc needs a TTY seat to open a Wayland session — a plain systemd service doesn't provide one, so VLC had no display to open | greetd → labwc |
| **greetd on `vt=1`** | VT1 is owned by `getty@tty1` — conflict caused greetd to fail to start | Use `vt = 7` in greetd config |
| **MJPEG preload to RAM** | Explored to reduce switching latency — not needed once VLC single-instance `set_media()` was working smoothly | VLC single-instance |

---

## If Nothing Works

1. Run the script manually and read the full output:

```bash
export DISPLAY=:0 && export XDG_RUNTIME_DIR=/run/user/1000
python3 /home/raspi/planet_switched.py
```

2. Check full system log for anything unusual:

```bash
sudo journalctl -n 100 --no-pager
```

3. Verify all files are in place:

```bash
ls -lh /home/raspi/planet_switched.py
ls -lh /home/raspi/planet/
cat /etc/greetd/config.toml
cat ~/.config/labwc/autostart
```

4. If the Pi itself seems unstable (random reboots, SD errors in logs), the SD card may be corrupted — re-flash the OS from scratch and restore from a backup.
