// save_file.h - With checksum validation
#pragma once
#include <vector>
#include <string>
#include <cstdint>

class SaveFile {
public:
    SaveFile();
    ~SaveFile();
    
    bool Load(const std::string& path);
    bool Save();
    bool IsLoaded() const { return loaded; }
    bool IsModified() const { return modified; }
    bool IsChecksumValid() const { return checksumValid; }
    
    std::string GetPath() const { return filePath; }
    size_t GetSize() const { return data.size(); }
    
    // Read operations
    uint8_t ReadByte(uint32_t offset) const;
    int32_t ReadInt32(uint32_t offset) const;
    bool ReadBool(uint32_t offset, uint8_t bitIndex) const;
    
    // Write operations
    void WriteByte(uint32_t offset, uint8_t value);
    void WriteInt32(uint32_t offset, int32_t value);
    void WriteBool(uint32_t offset, bool value, uint8_t bitIndex);
    
    // Checksum operations
    void RecalculateChecksum();
    bool ValidateChecksum();
    uint32_t CalculateChecksum() const;
    
private:
    std::vector<uint8_t> data;
    std::string filePath;
    bool loaded;
    bool modified;
    bool checksumValid;
    
    uint32_t storedChecksum;
    uint32_t checksumOffset;  // Where checksum is stored in file
    
    bool DetectChecksumLocation();
};