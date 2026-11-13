// save_store.h - v2.0: direct access to the game's encrypted save data.
//
// The save folder (ux0:user/00/savedata/<TITLEID>) is PFS-encrypted. SaveStore
// loads VitaShell's kernel/user modules and mounts that folder so files can be
// read and written in plain form; the system re-encrypts on write. Nothing
// outside this module needs to know about mounting.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// One editable save file found inside a game's save folder.
struct SaveSlot {
    std::string titleId;   // e.g. "PCSA00133"
    std::string label;     // e.g. "Ratchet & Clank Collection (US)" or the bare title id
    std::string dirPath;   // e.g. "ux0:user/00/savedata/PCSA00133" (the folder that gets mounted)
    std::string filePath;  // full path of the file inside dirPath
    std::string fileName;  // path relative to dirPath, for display
    size_t size = 0;       // bytes
};

namespace SaveStore {

// Loads the kernel + user modules needed for mounting. Call once at startup.
// Returns false and fills `error` (short, user-displayable) on failure.
bool Init(std::string& error);
bool IsReady();

// Lists candidate save files. No mounting is needed for this: names and sizes
// are visible on the encrypted folder. With knownOnly == true only the known
// Ratchet & Clank title ids are scanned; with false, every folder under
// ux0:user/00/savedata is. Only files of at least kMinSaveSize are returned.
std::vector<SaveSlot> Scan(bool knownOnly);

// Mounts slot.dirPath, reads the whole file decrypted, unmounts.
bool ReadFile(const SaveSlot& slot, std::vector<uint8_t>& out, std::string& error);

// Mounts slot.dirPath, overwrites the file in place (re-encrypted by the
// system), unmounts. Does not create backups; see WriteBackup.
bool WriteFile(const SaveSlot& slot, const std::vector<uint8_t>& data, std::string& error);

// Writes `data` (decrypted) to ux0:data/slimseditor/backups/<TITLEID>/<file>.bak,
// creating folders as needed. Slashes in fileName become underscores.
bool WriteBackup(const SaveSlot& slot, const std::vector<uint8_t>& data, std::string& error);

constexpr size_t kMinSaveSize = 650000;  // smallest file the editor recognises (RC1)

} // namespace SaveStore
