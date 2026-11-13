// save_file.h - With checksum validation
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "save_store.h"

class SaveFile {
public:
    SaveFile();
    ~SaveFile();

    // Reads the save decrypted through SaveStore. On failure the previously
    // loaded save (if any) is left untouched and GetLastError() says why.
    bool Load(const SaveSlot& slot);
    // Writes the save back in place. The first Save after a Load first backs
    // up the bytes as originally loaded; a failed backup aborts the save.
    bool Save();
    bool Reload();
    bool IsLoaded() const { return loaded; }
    bool IsModified() const { return modified; }
    bool IsChecksumValid() const { return checksumValid; }

    // Set on any failed Load/Save/Reload, cleared on success.
    const std::string& GetLastError() const { return lastError; }
    const SaveSlot& GetSlot() const { return slot; }
    std::string GetPath() const { return slot.filePath; }
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
    std::vector<uint8_t> originalData;  // bytes as loaded, for the backup
    SaveSlot slot;
    std::string lastError;
    bool backupDone;                    // backup written since the last Load
    bool loaded;
    bool modified;
    bool checksumValid;
    
    uint32_t storedChecksum;
    uint32_t checksumOffset;  // Where checksum is stored in file
    
    bool DetectChecksumLocation();
};