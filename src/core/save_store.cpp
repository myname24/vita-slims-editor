// save_store.cpp - module loading, save scan, mount/read/write, backups.
#include "save_store.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <taihen.h>
#include <psp2/vshbridge.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>

extern "C" {
#include "vitashell_user.h"
}

namespace {

const char* const kAppTitleId   = "SLMS00001";
const char* const kSaveBase     = "ux0:user/00/savedata";
const char* const kVitaShellDir = "ux0:VitaShell/module/";
const char* const kAppModuleDir = "ux0:app/SLMS00001/sce_module/";

struct KnownTitle {
    const char* id;
    const char* label;
};

const KnownTitle kKnownTitles[] = {
    {"PCSA00133", "Ratchet & Clank Collection (US)"},
    {"PCSF00482", "Ratchet & Clank Trilogy (EU)"},
    {"PCSF00484", "Ratchet & Clank (EU)"},
    {"PCSF00485", "Ratchet & Clank (EU)"},
    {"PCSF00486", "Ratchet & Clank (EU)"},
};

bool g_ready = false;

std::string Hex(int code) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%08X", static_cast<unsigned int>(code));
    return buf;
}

bool PathExists(const std::string& path) {
    SceIoStat st;
    std::memset(&st, 0, sizeof(st));
    return sceIoGetstat(path.c_str(), &st) >= 0;
}

// VitaShell's own copy first, the copy bundled in our VPK second.
std::string ModulePath(const char* name) {
    std::string p = std::string(kVitaShellDir) + name;
    if (PathExists(p)) return p;
    return std::string(kAppModuleDir) + name;
}

std::string LabelFor(const std::string& titleId) {
    for (const KnownTitle& k : kKnownTitles) {
        if (titleId == k.id) return k.label;
    }
    return titleId;
}

// Mounts `dir` (a save folder). On success returns >= 0 and fills mountPoint
// (16 bytes), which is only needed for Unmount.
int MountDir(const std::string& dir, char mountPoint[16]) {
    static const int kIds[] = {0x3ED, 0x6E, 0x12E, 0x12F};
    static const uint8_t kZeroKey[16] = {0};

    int res = -1;
    for (int id : kIds) {
        std::memset(mountPoint, 0, 16);

        ShellMountIdArgs args;
        args.id = id;
        args.process_titleid = kAppTitleId;
        args.path = dir.c_str();
        args.desired_mount_point = NULL;
        args.klicensee = kZeroKey;
        args.mount_point = mountPoint;

        res = shellUserMountById(&args);
        if (res >= 0) return res;
    }
    return res;
}

void Unmount(const char* mountPoint) {
    if (mountPoint[0] != '\0') sceAppMgrUmount(mountPoint);
}

// Adds the qualifying files of `dir` to `out`. `rel` is the path of `dir`
// relative to the title folder ("" at the root); depth 0 is the root.
void ScanDir(const std::string& titleId, const std::string& titleDir,
             const std::string& rel, int depth, std::vector<SaveSlot>& out) {
    std::string dir = rel.empty() ? titleDir : titleDir + "/" + rel;
    SceUID dfd = sceIoDopen(dir.c_str());
    if (dfd < 0) return;

    std::vector<std::string> subdirs;
    SceIoDirent ent;
    for (;;) {
        std::memset(&ent, 0, sizeof(ent));
        if (sceIoDread(dfd, &ent) <= 0) break;

        std::string name = ent.d_name;
        if (name.empty() || name == "." || name == "..") continue;

        if (SCE_S_ISDIR(ent.d_stat.st_mode)) {
            if (depth == 0 && name != "sce_sys" && name != "sce_pfs") {
                subdirs.push_back(name);
            }
            continue;
        }

        if (ent.d_stat.st_size < static_cast<SceOff>(SaveStore::kMinSaveSize)) continue;

        SaveSlot slot;
        slot.titleId  = titleId;
        slot.label    = LabelFor(titleId);
        slot.dirPath  = titleDir;
        slot.fileName = rel.empty() ? name : rel + "/" + name;
        slot.filePath = titleDir + "/" + slot.fileName;
        slot.size     = static_cast<size_t>(ent.d_stat.st_size);
        out.push_back(slot);
    }
    sceIoDclose(dfd);

    for (const std::string& sub : subdirs) {
        ScanDir(titleId, titleDir, sub, depth + 1, out);
    }
}

bool WriteAll(SceUID fd, const std::vector<uint8_t>& data, int& code) {
    size_t done = 0;
    while (done < data.size()) {
        int n = sceIoWrite(fd, data.data() + done, data.size() - done);
        if (n <= 0) {
            code = n;
            return false;
        }
        done += static_cast<size_t>(n);
    }
    return true;
}

// sceIoMkdir, treating "already exists" as success.
void MakeDir(const std::string& path) {
    sceIoMkdir(path.c_str(), 0777);
}

} // namespace

namespace SaveStore {

bool Init(std::string& error) {
    error.clear();
    if (g_ready) return true;

    // Kernel module (skipped when VitaShell's is already loaded).
    int search_unk[2];
    if (_vshKernelSearchModuleByName("VitaShellKernel2", search_unk) < 0) {
        std::string path = ModulePath("kernel.skprx");
        SceUID id = taiLoadKernelModule(path.c_str(), 0, NULL);
        if (id < 0) {
            error = "Kernel module failed to load (" + Hex(id) +
                    "). Enable unsafe homebrew and reboot.";
            return false;
        }
        int res = taiStartKernelModule(id, 0, NULL, 0, NULL, NULL);
        if (res < 0) {
            taiStopUnloadKernelModule(id, 0, NULL, 0, NULL, NULL);
            error = "Kernel module failed to start (" + Hex(res) +
                    "). Enable unsafe homebrew and reboot.";
            return false;
        }
    }

    // User module.
    std::string path = ModulePath("user.suprx");
    SceUID uid = sceKernelLoadStartModule(path.c_str(), 0, NULL, 0, NULL, NULL);
    if (uid < 0) {
        error = "User module failed to load (" + Hex(uid) +
                "). Enable unsafe homebrew and reboot.";
        return false;
    }

    g_ready = true;
    return true;
}

bool IsReady() {
    return g_ready;
}

std::vector<SaveSlot> Scan(bool knownOnly) {
    std::vector<SaveSlot> out;
    std::vector<std::string> titles;

    if (knownOnly) {
        for (const KnownTitle& k : kKnownTitles) titles.push_back(k.id);
    } else {
        SceUID dfd = sceIoDopen(kSaveBase);
        if (dfd >= 0) {
            SceIoDirent ent;
            for (;;) {
                std::memset(&ent, 0, sizeof(ent));
                if (sceIoDread(dfd, &ent) <= 0) break;
                std::string name = ent.d_name;
                if (name.empty() || name == "." || name == "..") continue;
                if (SCE_S_ISDIR(ent.d_stat.st_mode)) titles.push_back(name);
            }
            sceIoDclose(dfd);
        }
    }

    for (const std::string& id : titles) {
        ScanDir(id, std::string(kSaveBase) + "/" + id, "", 0, out);
    }

    std::sort(out.begin(), out.end(), [](const SaveSlot& a, const SaveSlot& b) {
        if (a.titleId != b.titleId) return a.titleId < b.titleId;
        return a.fileName < b.fileName;
    });
    return out;
}

bool ReadFile(const SaveSlot& slot, std::vector<uint8_t>& out, std::string& error) {
    error.clear();
    out.clear();
    if (!IsReady()) {
        error = "Save access is not available (modules not loaded).";
        return false;
    }

    char mountPoint[16];
    int res = MountDir(slot.dirPath, mountPoint);
    if (res < 0) {
        error = "Could not mount the save folder (" + Hex(res) +
                "). Close the game and try again.";
        return false;
    }

    bool ok = false;
    SceUID fd = sceIoOpen(slot.filePath.c_str(), SCE_O_RDONLY, 0);
    if (fd < 0) {
        error = "Could not open the save file (" + Hex(fd) + ").";
    } else {
        SceOff size = sceIoLseek(fd, 0, SCE_SEEK_END);
        sceIoLseek(fd, 0, SCE_SEEK_SET);
        if (size <= 0) {
            error = "The save file is empty or unreadable (" +
                    Hex(static_cast<int>(size)) + ").";
        } else {
            out.resize(static_cast<size_t>(size));
            size_t done = 0;
            int n = 0;
            while (done < out.size()) {
                n = sceIoRead(fd, out.data() + done, out.size() - done);
                if (n <= 0) break;
                done += static_cast<size_t>(n);
            }
            if (done == out.size()) {
                ok = true;
            } else if (n < 0) {
                error = "Could not read the save file (" + Hex(n) + ").";
            } else {
                error = "The save file could not be read completely.";
            }
        }
        sceIoClose(fd);
    }

    Unmount(mountPoint);
    if (!ok) out.clear();
    return ok;
}

bool WriteFile(const SaveSlot& slot, const std::vector<uint8_t>& data, std::string& error) {
    error.clear();
    if (!IsReady()) {
        error = "Save access is not available (modules not loaded).";
        return false;
    }
    if (data.empty()) {
        error = "Nothing to write.";
        return false;
    }

    char mountPoint[16];
    int res = MountDir(slot.dirPath, mountPoint);
    if (res < 0) {
        error = "Could not mount the save folder (" + Hex(res) +
                "). Close the game and try again.";
        return false;
    }

    bool ok = false;
    SceUID fd = sceIoOpen(slot.filePath.c_str(),
                          SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) {
        error = "Could not open the save file for writing (" + Hex(fd) + ").";
    } else {
        int code = 0;
        ok = WriteAll(fd, data, code);
        int closeRes = sceIoClose(fd);
        if (!ok) {
            error = "Could not write the save file (" + Hex(code) + ").";
        } else if (closeRes < 0) {
            ok = false;
            error = "Could not finish writing the save file (" + Hex(closeRes) + ").";
        }
    }

    Unmount(mountPoint);
    return ok;
}

bool WriteBackup(const SaveSlot& slot, const std::vector<uint8_t>& data, std::string& error) {
    error.clear();

    std::string dir = "ux0:data";
    MakeDir(dir);
    dir += "/slimseditor";
    MakeDir(dir);
    dir += "/backups";
    MakeDir(dir);
    dir += "/" + slot.titleId;
    MakeDir(dir);

    std::string name = slot.fileName;
    std::replace(name.begin(), name.end(), '/', '_');
    std::string path = dir + "/" + name + ".bak";

    SceUID fd = sceIoOpen(path.c_str(), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) {
        error = "Could not create the backup (" + Hex(fd) + ").";
        return false;
    }

    int code = 0;
    bool ok = WriteAll(fd, data, code);
    int closeRes = sceIoClose(fd);
    if (!ok) {
        error = "Could not write the backup (" + Hex(code) + ").";
        return false;
    }
    if (closeRes < 0) {
        error = "Could not finish writing the backup (" + Hex(closeRes) + ").";
        return false;
    }
    return true;
}

} // namespace SaveStore
