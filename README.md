# 🖱️ Ultra‑High Sensitivity Mouse Interceptor (libevdev + uinput)

This project creates a **userspace mouse interceptor** that bypasses libinput limits and allows **extreme mouse sensitivity** by multiplying raw mouse deltas.

It works by:

```
Physical mouse → /dev/input/eventX → libevdev (grab)
                                     ↓ (modify REL_X / REL_Y)
                                 uinput virtual mouse → system cursor
```

No acceleration curves. No caps. Near‑zero latency.

---

## ⚠️ WARNING
- This program **grabs your mouse**
- If it crashes, mouse input stops until the process is killed
- Always keep a second TTY (`Ctrl+Alt+F2`) or SSH ready

---

## ✅ Environment
- Distro: Arch Linux (or similar)
- Session: X11 or Wayland
- Language: C
- APIs: libevdev, uinput

---

## 📦 Dependencies

```bash
sudo pacman -S libevdev evtest
```

---

## 👤 Permissions Setup

### 1️⃣ Add user to input group
```bash
sudo usermod -aG input stof
```
(Reboot required)

### 2️⃣ Create udev rule for /dev/uinput
```bash
sudo nano /etc/udev/rules.d/99-uinput.rules
```

Content:
```text
KERNEL=="uinput", MODE="0660", GROUP="input"
```

Reload rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Reload module:
```bash
sudo modprobe -r uinput
sudo modprobe uinput
```

Verify:
```bash
ls -l /dev/uinput
```
Expected:
```
crw-rw---- root input /dev/uinput
```

---

## 🖱️ Identify Mouse Device

```bash
ls -l /dev/input/by-id/
```

Use **ONLY** the device ending with:
```
*-event-mouse
```

Example:
```
/dev/input/by-id/usb-Logitech_USB_Optical_Mouse-event-mouse
```

Ignore:
- `*-mouse`
- `*-hidraw`

Optional test:
```bash
sudo evtest /dev/input/by-id/usb-Logitech_USB_Optical_Mouse-event-mouse
```

---

## 🏗️ Build

```bash
gcc mouse_boost.c -o mouse_boost -levdev
```

---

## ▶️ Run (manual)

```bash
./mouse_boost /dev/input/by-id/usb-Logitech_USB_Optical_Mouse-event-mouse
```

Emergency stop:
```bash
pkill mouse_boost
```

---

## ⚙️ systemd Service

### Install binary
```bash
sudo mv mouse_boost /usr/local/bin/
sudo chmod 755 /usr/local/bin/mouse_boost
```

### Service file
```bash
sudo nano /etc/systemd/system/mouse-boost.service
```

```ini
[Unit]
Description=Ultra High Sensitivity Mouse Interceptor
After=systemd-udev-settle.service
Wants=systemd-udev-settle.service

[Service]
Type=simple
ExecStart=/usr/local/bin/mouse_boost /dev/input/by-id/usb-Logitech_USB_Optical_Mouse-event-mouse
Restart=on-failure
RestartSec=1
Nice=-10
NoNewPrivileges=true

[Install]
WantedBy=multi-user.target
```

Enable & start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable mouse-boost.service
sudo systemctl start mouse-boost.service
```

Control:
```bash
sudo systemctl stop mouse-boost.service
sudo systemctl restart mouse-boost.service
sudo systemctl disable mouse-boost.service
```

Logs:
```bash
journalctl -u mouse-boost.service -f
```

---

## 🛑 Safety

Disable from TTY if needed:
```bash
sudo systemctl disable --now mouse-boost.service
```

---

## 🚀 Future Improvements
- Hotkey toggle
- Dynamic acceleration
- Per‑axis boost
- Auto‑detect mouse by vendor/product
- Fail‑safe timeout

---

Enjoy your **uncapped mouse speed** 😈🖱️

