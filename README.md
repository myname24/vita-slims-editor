# Vita Slim's Editor

A clean, modern save editor for the **Ratchet & Clank HD Collection** on PS Vita.

![Version](https://img.shields.io/badge/version-1.0-blue)
![Platform](https://img.shields.io/badge/platform-PS%20Vita-orange)
![License](https://img.shields.io/badge/license-MIT-green)

## 🎮 Features

- **Multi-game support**: Ratchet & Clank 1, 2, and 3 HD
- **Auto-detection**: Automatically detects which game you're editing
- **Touch controls**: Full touchscreen support for intuitive editing
- **D-pad editor**: Precise value editing with customizable step sizes
- **Beautiful UI**: Modern, polished interface with smooth scrolling
- **178+ editable items**:
  - Bolts, Raritanium (RC2)
  - Weapon ammo (all weapons from all 3 games)
  - Gadgets (toggle ownership)
  - Unlockables (toggle unlocks)
  - Weapon EXP (RC3)

## 🎯 Supported Games

| Game | File Size | Items |
|------|-----------|-------|
| Ratchet & Clank HD | ~700 KB | 50+ items |
| Ratchet & Clank 2: Going Commando HD | ~900 KB | 70+ items |
| Ratchet & Clank 3: Up Your Arsenal HD | ~2 MB | 60+ items |

## 📥 Installation

### Method 1: VPK Installation

1. Download `vitaslimseditor.vpk` from [Releases](https://github.com/myname24/vita-slims-editor/releases)
2. Transfer to your PS Vita via FTP or USB
3. Install using VitaShell
4. Launch from LiveArea

### Method 2: Build from Source

**Prerequisites:**
- VitaSDK installed and configured
- CMake 3.12+
- Git

**Build steps:**
```bash
# Clone repository
git clone https://github.com/myname24/vita-slims-editor.git
cd vita-slims-editor

# Create build directory
mkdir build && cd build

# Build
cmake ..
make

# VPK will be created as: vitaslimseditor.vpk
```

## 🎮 Usage

### Getting Your Save Files

**Option 1: Apollo Save Tool**
1. Use Apollo to decrypt your saves
2. Export decrypted save files
3. Copy to editing location

### Editing

1. Launch **Vita Slim's Editor**
2. Browse to your save file
3. Select save file to load
4. Use tabs to navigate:
   - **Values**: Bolts, Raritanium, etc.
   - **Weapons**: Weapon ammo
   - **Gadgets**: Toggle gadget ownership
   - **Unlocks**: Toggle unlockables

### Controls

**Navigation:**
- **D-pad Up/Down**: Navigate items
- **Touch**: Tap items or scroll
- **L/R Triggers**: Switch tabs

**Editing:**
- **X**: Edit value or toggle
- **O**: Cancel/Back
- **START**: Quick save
- **Touch SAVE button**: Save changes
- **Touch BACK button**: Return to file browser

**Value Editor (D-pad mode):**
- **Up/Down**: Increase/decrease by step
- **Left**: Jump to minimum
- **Right**: Jump to maximum
- **L Trigger**: Decrease step size
- **R Trigger**: Increase step size
- **X**: Confirm changes
- **O**: Cancel

**Step sizes:** 1, 10, 100, 1K, 10K, 100K, 1M

### Restoring Edited Saves

1. Copy edited `SAVEDATA.BIN` back to Apollo save folder
2. Import the save file using Apollo
3. Launch game and verify changes

## ⚙️ Technical Details

### Game Detection

The editor uses file size to automatically detect which game:
- **700-850 KB**: Ratchet & Clank 1 HD
- **850-1150 KB**: Ratchet & Clank 2 HD
- **1150+ KB**: Ratchet & Clank 3 HD

### Save File Format

All saves are decrypted `.BIN` files with:
- Little-endian 32-bit integers for values
- Bitflags for gadgets/unlockables
- No encryption (when using decrypted saves)

### Offset Verification

All offsets have been verified and tested with:
- Actual PS Vita save files
- Multiple save states
- All three games in the collection

## 🛠️ Development

### Prerequisites
- VitaSDK
- SDL2 (included in VitaSDK)
- SDL2_ttf (included in VitaSDK)
- CMake

### Building
```bash
mkdir build && cd build
cmake ..
make
```

### Project Structure
- **C++17** standard
- **SDL2** for rendering and input
- **TTF fonts** for text rendering
- **CMake** build system

### Adding New Offsets

Edit `src/data/rac_vita_games_data.h`:
```cpp
// Example: Adding a new value
{"New Value", "Description", OFFSET, MIN, MAX, 4},
```

Offset sources:
- [RatchetModding/slimseditor](https://github.com/RatchetModding/slimseditor) JSON files
- Manual hex editor analysis
- Community contributions

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### What to Contribute
- New game offsets
- UI improvements
- Bug fixes
- Documentation
- Screenshots
- Additional game support

## 🐛 Known Issues

- Some offsets may vary between game versions
- Always backup your saves before editing
- Some values may not take effect until area reload


## 🙏 Credits

- **Offset data**: [RatchetModding/slimseditor](https://github.com/RatchetModding/slimseditor)
- **VitaSDK**: The VitaSDK team
- **SDL2**: Sam Lantinga and contributors
- **Community**: PS Vita homebrew community

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## ⚠️ Disclaimer

This tool is for educational purposes only. The author is not responsible for any damage to your saves or console. Always backup your saves before editing.

## 🔗 Links

- [Release Page](https://github.com/myname24/vita-slims-editor/releases)
- [Issue Tracker](https://github.com/myname24/vita-slims-editor/issues)

## 💬 Support

- Open an [issue](https://github.com/myname24/vita-slims-editor/issues) for bugs
- Check existing issues before creating new ones
- Provide save file details when reporting bugs

---

Made with ❤️ and claude AI for the PS Vita community
