## PicSimLab Setup

If you don't have a physical Arduino Uno/ATmega328 board available, you can simulate the full environment using **PicSimLab** (v0.8.x or higher) on Linux.

Since AVRMon relies on the **MiniCore** bootloader footprint and memory structure (which differs from the standard Arduino Optiboot), follow this complete guide to install PicSimLab, set up the virtual serial pair, and hook the MiniCore bootloader dump into the simulator.

---

### 1. Install PicSimLab

Choose the easiest installation method for your Linux distribution:

#### Option A: Flatpak 
```bash
flatpak install flathub com.lcgamboa.picsimlab
```

#### Option B: AppImage
1. Download the latest AppImage from the [PicSimLab Releases](https://github.com/lcgamboa/picsimlab/releases) page.
2. Make it executable and run:
   ```bash
   chmod +x PicSimLab-*.AppImage
   ./PicSimLab-*.AppImage
   ```

#### Option C: Ubuntu/Debian (.deb Package) [Recommended]
Download the `.deb` package matching your system from the releases page and install:
```bash
sudo apt install ./picsimlab_*.deb
```

---

### 2. Setup Virtual Serial Ports (`tty0tty`)

PicSimLab requires a virtual null-modem cable to bridge serial communication between the simulator and external terminals or scripts.

1. Clone and compile the kernel module:
   ```bash
   git clone https://github.com/freemed/tty0tty.git
   cd tty0tty/module
   make
   ```
   *(Note: If `make` fails with `gcc-XX not found`, install the matching GCC version for your kernel, e.g., `sudo apt install gcc-12`).*

2. Install and load the module:
   ```bash
   sudo cp tty0tty.ko /lib/modules/$(uname -r)/kernel/drivers/char/
   sudo depmod -a
   sudo modprobe tty0tty
   ```

3. Set port permissions & group access:
   ```bash
   sudo chmod 666 /dev/tnt*
   sudo usermod -aG dialout $USER
   ```
   *This creates mapped serial pairs (`/dev/tnt0` ↔ `/dev/tnt1`, `/dev/tnt2` ↔ `/dev/tnt3`, etc.).*

---

### 3. Configure MiniCore Bootloader in PicSimLab

By default, PicSimLab uses a standard `atmega328p` dump for the Arduino Uno board. To force PicSimLab to load the **MiniCore** bootloader image upon initialization:

1. Locate the hidden PicSimLab configuration folder in your home directory:
   ```bash
   cd ~/.picsimlab/
   ```

2. Backup the default board memory dump file:
   ```bash
   mv mdump_Arduino_Uno_atmega328p.hex mdump_Arduino_Uno_atmega328p.hex.old
   ```

3. Copy your compiled `arduino_MiniCore.hex` file to replace the default dump:
   ```bash
   cp /path/to/AVRMon/build/MiniCore.avr.328/arduino_MiniCore.hex ~/.picsimlab/mdump_Arduino_Uno_atmega328p.hex
   ```

---

### 4. Running the Simulation

1. Launch **PicSimLab**.
2. Select **Board > Arduino Uno**.
3. Configure the serial interface in PicSimLab:
   * Go to **Option > Serial Port** (or **Config > Serial**).
   * Set the port to `/dev/tnt0`.
   * Set **Serial Terminal** to **Internal** (to prevent host file-association errors with `.sterm`).
4. Load your compiled application binary via **File > Load Hex**.
5. Connect your external terminal (e.g., `cutecom`, `minicom`, or custom script) to `/dev/tnt1` at the configured baud rate.
6. Hit **Reset** on the PicSimLab interface to start executing AVRMon over the MiniCore core!