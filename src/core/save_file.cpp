// save_file.cpp - With checksum validation
#include "save_file.h"
#include <cstring>

SaveFile::SaveFile()
    : backupDone(false), loaded(false), modified(false), checksumValid(true),
      storedChecksum(0), checksumOffset(0) {
}

SaveFile::~SaveFile() {
}

bool SaveFile::Load(const SaveSlot& newSlot) {
    std::vector<uint8_t> bytes;
    std::string err;
    if (!SaveStore::ReadFile(newSlot, bytes, err)) {
        lastError = err.empty() ? "Could not read the save file." : err;
        return false;
    }

    // newSlot may alias this->slot (Reload), so copy before assigning.
    SaveSlot slotCopy = newSlot;
    slot = slotCopy;
    originalData = bytes;
    data.swap(bytes);
    backupDone = false;
    loaded = true;
    modified = false;
    lastError.clear();

    // Try to detect and validate checksum
    if (DetectChecksumLocation()) {
        checksumValid = ValidateChecksum();
    } else {
        // No checksum found, assume valid
        checksumValid = true;
    }
    
    return true;
}

bool SaveFile::Save() {
    if (!loaded) {
        lastError = "No save is loaded.";
        return false;
    }

    // Recalculate the section checksum if this save uses one (RC3)
    if (checksumOffset > 0) {
        RecalculateChecksum();
    }

    std::string err;

    // Back up the bytes as originally loaded, once per Load.
    if (!backupDone) {
        if (!SaveStore::WriteBackup(slot, originalData, err)) {
            lastError = err.empty() ? "Could not write the backup." : err;
            return false;
        }
        backupDone = true;
    }

    // Overwrite in place inside the mounted save folder.
    if (!SaveStore::WriteFile(slot, data, err)) {
        lastError = err.empty() ? "Could not write the save file." : err;
        return false;
    }

    modified = false;
    lastError.clear();
    return true;
}

bool SaveFile::Reload() {
    if (slot.filePath.empty()) {
        lastError = "No save is loaded.";
        return false;
    }
    return Load(slot);
}

uint8_t SaveFile::ReadByte(uint32_t offset) const {
    if (static_cast<size_t>(offset) >= data.size()) return 0;
    return data[static_cast<size_t>(offset)];
}

int32_t SaveFile::ReadInt32(uint32_t offset) const {
    // Written as a subtraction so a huge offset (e.g. the 0xFFFFFFFF "action row"
    // marker) cannot wrap around on the Vita's 32-bit size_t and pass the check.
    if (data.size() < sizeof(int32_t) || static_cast<size_t>(offset) > data.size() - sizeof(int32_t)) return 0;

    int32_t value;
    std::memcpy(&value, &data[static_cast<size_t>(offset)], sizeof(int32_t));
    return value;
}

bool SaveFile::ReadBool(uint32_t offset, uint8_t bitIndex) const {
    if (offset >= data.size() || bitIndex > 7) return false;
    
    uint8_t byte = data[offset];
    return (byte >> bitIndex) & 1;
}

void SaveFile::WriteByte(uint32_t offset, uint8_t value) {
    if (static_cast<size_t>(offset) >= data.size()) return;

    data[static_cast<size_t>(offset)] = value;
    modified = true;
}

void SaveFile::WriteInt32(uint32_t offset, int32_t value) {
    if (data.size() < sizeof(int32_t) || static_cast<size_t>(offset) > data.size() - sizeof(int32_t)) return;

    std::memcpy(&data[static_cast<size_t>(offset)], &value, sizeof(int32_t));
    modified = true;
}

void SaveFile::WriteBool(uint32_t offset, bool value, uint8_t bitIndex) {
    if (bitIndex > 7) return;
    if (static_cast<size_t>(offset) >= data.size()) return;

    uint8_t byte = data[static_cast<size_t>(offset)];
    if (value) {
        byte |= (1 << bitIndex);
    } else {
        byte &= ~(1 << bitIndex);
    }
    data[static_cast<size_t>(offset)] = byte;
    modified = true;
}

// Save layout (all three games): two u32 sizes, then the first section as
//   u32 size @8, u32 checksum @12, `size` bytes of data @16.
// RC3 stores a CRC16 of that data in the checksum field and ignores a save
// whose checksum is wrong; RC1 and RC2 leave the field at 0. The checksum is
// only treated as present when the stored value matches the calculation, so
// a file that does not use it is never altered.
static const uint32_t kSectionSizeOffset = 8;
static const uint32_t kSectionChecksumOffset = 12;
static const uint32_t kSectionDataOffset = 16;

bool SaveFile::DetectChecksumLocation() {
    checksumOffset = 0;
    storedChecksum = 0;
    if (data.size() < kSectionDataOffset) return false;

    uint32_t stored = static_cast<uint32_t>(ReadInt32(kSectionChecksumOffset));
    if (stored == 0) return false;

    checksumOffset = kSectionChecksumOffset;
    if (CalculateChecksum() != stored) {
        checksumOffset = 0;
        return false;
    }
    storedChecksum = stored;
    return true;
}

uint32_t SaveFile::CalculateChecksum() const {
    // CRC16 used by the Ratchet & Clank save format: polynomial 0x1F45,
    // initial value 0x8320, over the first section's data.
    if (data.size() < kSectionDataOffset) return 0;
    size_t size = static_cast<uint32_t>(ReadInt32(kSectionSizeOffset));
    if (size == 0 || size > data.size() - kSectionDataOffset) return 0;

    uint32_t crc = 0x8320;
    for (size_t i = 0; i < size; i++) {
        crc ^= static_cast<uint32_t>(data[kSectionDataOffset + i]) << 8;
        for (int j = 0; j < 8; j++) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1F45) : (crc << 1);
            crc &= 0xFFFF;
        }
    }
    return crc;
}

bool SaveFile::ValidateChecksum() {
    if (checksumOffset == 0) return true;  // No checksum
    
    uint32_t calculated = CalculateChecksum();
    return (calculated == storedChecksum);
}

void SaveFile::RecalculateChecksum() {
    if (checksumOffset == 0) return;  // No checksum
    
    uint32_t newChecksum = CalculateChecksum();
    WriteInt32(checksumOffset, newChecksum);
    storedChecksum = newChecksum;
    checksumValid = true;
}