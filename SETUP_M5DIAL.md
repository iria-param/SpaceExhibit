# M5Dial Setup — Flashing the Firmware

This is a **one-time operation**. Once flashed, the M5Dial doesn't need to be touched again unless you're changing the number of videos or the knob sensitivity.

> **Do this from a Windows / Mac / Linux machine with Arduino IDE. The Raspberry Pi is not involved in this step.**

---

## What the Firmware Does

`SpaceExhibit.ino` runs on the M5Dial's ESP32-S3 chip and does three things:

1. **Reads the rotary encoder** — counts ticks as the knob turns. Every `SKIP` ticks (default: 4) = one scene change. This prevents accidental jumps from slight knob vibration.
2. **Draws planet artwork** on the 1.28" round TFT display — so visitors get immediate visual feedback on the dial itself, not just the main screen.
3. **Sends the planet index** over USB serial to the Raspberry Pi — a plain number followed by a newline, e.g. `"5\n"` for Jupiter.

The button on the M5Dial flashes the screen white — it's visual feedback only and does not send a signal to the Pi.

---

## What You Need

| Tool | Where to get it | Why it's needed |
|------|----------------|-----------------|
| Arduino IDE 2.x | [arduino.cc/en/software](https://www.arduino.cc/en/software) | IDE to compile and flash the sketch |
| M5Stack board package | Arduino Boards Manager | Teaches Arduino IDE what an M5Dial is |
| M5Dial Arduino library | Arduino Library Manager | Provides display, encoder, and button APIs |
| CP210x or CH340 USB driver | Usually auto-installs on Win 10+. If not: Silicon Labs / WCH website | The M5Dial appears as a serial port via this chip — without the driver it won't show up |
| USB-C cable (data-capable) | Any data cable — **not a charge-only cable** | To connect M5Dial to your computer for flashing |

---

## Step 1 — Install Arduino IDE

Download and install from [arduino.cc/en/software](https://www.arduino.cc/en/software). Version 2.x is recommended.

---

## Step 2 — Add the M5Stack Board Package

Arduino IDE needs to know about M5Stack boards before you can target the M5Dial.

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Find the **"Additional Boards Manager URLs"** field
4. Add this URL:

```
https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
```

5. Click OK
6. Go to **Tools → Board → Boards Manager**
7. Search `M5Stack`
8. Click **Install** on the M5Stack package

> This downloads ~200MB. Allow a few minutes.

---

## Step 3 — Install the M5Dial Library

1. Go to **Tools → Manage Libraries** (or Sketch → Include Library → Manage Libraries)
2. Search `M5Dial`
3. Click **Install** on the M5Dial library by M5Stack
4. When prompted to install dependencies, click **Install All**

---

## Step 4 — Connect the M5Dial

Plug the M5Dial into your computer via USB-C.

**Verify the port appears:**
- **Windows:** Device Manager → Ports (COM & LPT) → look for `CP210x` or `CH340`. Note the COM number (e.g. COM5).
- **Mac/Linux:** Run `ls /dev/tty.*` or `ls /dev/ttyUSB*` — look for a new entry.

> If no port appears, the USB driver isn't installed. Download CP210x drivers from Silicon Labs or CH340 drivers from WCH depending on which chip your M5Dial batch uses.

---

## Step 5 — Select Board and Port

In Arduino IDE:

```
Tools → Board → M5Stack → M5Dial
Tools → Port → (select the COM port from Step 4)
```

---

## Step 6 — Configure the Sketch

Open `SpaceExhibit.ino`. At the very top, two constants control the exhibit behaviour:

```cpp
#define SKIP  4   // encoder ticks per scene change
#define TOTAL 10  // total number of planet videos
```

**SKIP — knob sensitivity:**
- `4` is the default and works well for most setups
- If the video switches too easily when visitors barely touch the knob → **increase to 5 or 6**
- If the knob feels unresponsive and you have to turn a lot → **decrease to 3**
- This is the most common thing to tune after installation

**TOTAL — number of planets:**
- Leave at `10` unless you add or remove videos
- If you add an 11th video, set `TOTAL = 11` here AND add it to `VIDEO_MAP` in `planet_switched.py`

---

## Step 7 — Flash

Click **Upload** (the right-arrow button). Wait for the status bar to show:

```
Done uploading.
```

The M5Dial screen should light up and show the first planet (Sun).

---

## Step 8 — Verify

1. Go to **Tools → Serial Monitor**
2. Set baud rate to **115200** (bottom-right dropdown)
3. Turn the M5Dial knob

You should see numbers 1–10 printing as you turn. The numbers wrap — turning past 10 goes back to 1, and turning back past 1 goes to 10.

If you see garbage characters instead of numbers, the baud rate is wrong — make sure it's set to 115200.

---

## Reconnecting the M5Dial to the Pi

After flashing, unplug the M5Dial from your computer and plug it into the **Raspberry Pi's USB-A port** via the USB-C → USB-A cable.

The Pi will see it as `/dev/ttyACM0`. Verify:

```bash
ls /dev/ttyACM*
# Should show: /dev/ttyACM0
```

---

## Planet Art — What's on the M5Dial Screen

The firmware draws each planet from scratch using the M5Canvas graphics library — no image files are stored. Each scene has its own draw function:

| Function | What it renders |
|----------|----------------|
| `drawSun()` | Corona rays, photosphere gradient, convection cells, hot core |
| `drawVenus()` | Thick cloud bands, atmosphere glow, limb darkening |
| `drawMoon()` | Craters with bright rims, albedo regions, terminator shading |
| `drawMars()` | Polar ice cap, Valles Marineris canyon, Olympus Mons hint |
| `drawJupiter()` | Alternating atmospheric bands, Great Red Spot |
| `drawSaturn()` | Ring system (back + front half), subtle bands |
| `drawNeptune()` | Great Dark Spot, bright cloud streaks, thin ring |
| `drawKepler()` | Ocean world, continents, cloud systems, host star |
| `drawAndromeda()` | Spiral arms, galaxy disc gradient, dust lane, bright nucleus |
| `drawPillars()` | Nebula gradient, three pillars with ionisation glow tips |

The display also shows:
- The **planet name** as a text label at the bottom
- **Index dots** in a bottom arc — the current position is highlighted in gold
