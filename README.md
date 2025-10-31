# CS2 ESP + Aimbot Base  
**Offline Development Tool (Bots Only)**

![CS2 ESP Example](https://i.imgur.com/EXAMPLE.png)  
*Red boxes on enemies — perfect for testing AI or debugging!*

---

## What Is This?

This is a **simple, safe, external tool** that:
- Draws **red boxes** around enemy bots in CS2
- Lets you **aim at heads** by holding **Left Mouse Button**
- Works **only in offline mode** (bots, practice, custom maps)

> **100% Legal & Safe** — **No VAC, No bans** — because it's **offline only**.

---

## Why Use This?

Perfect for:
- Learning how cheats work (educational)
- Testing AI bots
- Practicing aim in a controlled environment
- Debugging player positions

---

## Features (Easy Mode)

| Feature        | Hotkey       | What It Does |
|----------------|--------------|-------------|
| **Toggle All** | `Insert`     | Turn cheat ON/OFF |
| **ESP Boxes**  | `Home`       | Toggle red boxes |
| **Aimbot**     | `End` + LMB  | Hold LMB → aim at head |

---

## How to Use (Step-by-Step)

### Step 1: Open CS2 Offline
1. Launch **Counter-Strike 2**
2. Go to **Play → Practice → With Bots**
3. Start a match (no internet needed)

### Step 2: Build the Tool
1. Open this folder in **File Explorer**
2. **Double-click `BUILD.bat`**
   - It will compile the tool
   - Wait 10 seconds → `CS2OfflineTool.exe` appears

### Step 3: Run the Tool
1. **Right-click `CS2OfflineTool.exe` → Run as Administrator**
2. A black console opens → says “Waiting for CS2...”
3. Once CS2 is detected → **you’re in!**

### Step 4: Use Hotkeys
- Press `Insert` → cheat turns on
- Press `Home` → red boxes appear
- Press `End` + **hold LMB** → aimbot activates

---

## It Stopped Working? (Offsets Changed)

CS2 updates → numbers change → tool breaks.

**Fix in 2 minutes:**

1. Download: [cs2-dumper](https://github.com/a2x/cs2-dumper/releases)
2. Run `cs2-dumper.exe` while CS2 is open
3. Open `generated/offsets.json`
4. Copy the numbers into `src/main.cpp` (see guide below)
5. Double-click `BUILD.bat` again

**See `AUTO_UPDATE_GUIDE.txt` for pictures!**

---

## Is This Safe?

| Question | Answer |
|--------|--------|
| Will I get **VAC banned**? | **NO** — only works offline |
| Can I use this **online**? | **NEVER** — don’t even try |
| Is this **legal**? | **YES** — for private, offline use |

---

## Want to Learn More?

- Open `src/main.cpp` — it’s **simple C++**
- Change `smooth = 0.5f` → make aimbot slower/faster
- Change `fov = 90.0f` → bigger/smaller aimbot range

---

## Credits

- Built with love by **Dotty*
- Offsets from **cs2-dumper**
- GDI+ overlay (safe & external)

---

**Happy testing!**  
*Never use online. Stay safe. Learn cool stuff.*
