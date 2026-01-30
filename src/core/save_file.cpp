// save_file.cpp - With checksum validation
#include "save_file.h"
#include <fstream>
#include <cstring>
#include <cstdio>

SaveFile::SaveFile() 
    : loaded(false), modified(false), checksumValid(true), 
      storedChecksum(0), checksumOffset(0) {
}

SaveFile::~SaveFile() {
}

bool SaveFile::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    data.resize(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return false;
    }
    
    filePath = path;
    loaded = true;
    modified = false;
    
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
    if (!loaded) return false;
    
    // Recalculate checksum before saving if one exists
    if (checksumOffset > 0 && checksumOffset < data.size() - 4) {
        RecalculateChecksum();
    }
    
    // ATOMIC SAVE: Write to temp file first to prevent corruption
    std::string tempPath = filePath + ".tmp";
    std::ofstream file(tempPath, std::ios::binary);
    if (!file.is_open()) return false;
    
    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        file.close();
        std::remove(tempPath.c_str()); // Clean up temp if write failed
        return false;
    }
    file.close();
    
    // Replace original with temp
    std::remove(filePath.c_str());
    std::rename(tempPath.c_str(), filePath.c_str());
    
    modified = false;
    return true;
}

bool SaveFile::Reload() {
    if (filePath.empty()) return false;
    return Load(filePath);
}

uint8_t SaveFile::ReadByte(uint32_t offset) const {
    if (static_cast<size_t>(offset) >= data.size()) return 0;
    return data[static_cast<size_t>(offset)];
}

int32_t SaveFile::ReadInt32(uint32_t offset) const {
    if (static_cast<size_t>(offset) + sizeof(int32_t) > data.size()) return 0;

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
    if (static_cast<size_t>(offset) + sizeof(int32_t) > data.size()) return;

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

bool SaveFile::DetectChecksumLocation() {
    // R&C save files typically store checksum in last 4 bytes
    if (data.size() < 8) return false;
    
    // Common checksum locations:
    // 1. Last 4 bytes of file
    checksumOffset = data.size() - 4;
    storedChecksum = ReadInt32(checksumOffset);
    
    return true;
}

uint32_t SaveFile::CalculateChecksum() const {
    // Simple CRC32-like checksum
    // Calculate over all data except the checksum itself
    uint32_t checksum = 0xFFFFFFFF;
    
    for (size_t i = 0; i < data.size(); i++) {
        // Skip checksum location
        if (i >= checksumOffset && i < checksumOffset + 4) {
            continue;
        }
        
        checksum ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (checksum & 1) {
                checksum = (checksum >> 1) ^ 0xEDB88320;
            } else {
                checksum >>= 1;
            }
        }
    }
    
    return ~checksum;
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