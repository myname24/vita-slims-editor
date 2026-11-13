# Vita Slim's Editor 2.0 — Specification

## Goal

Edit Ratchet & Clank saves in place. The user launches the app, picks a save
from a list, edits, and saves. No Apollo round trip, no file manager.

Version 1.x only edited files that had already been decrypted with Apollo and
copied to `ux0:/data/slimseditor/saves`. Version 2.0 does the decrypting and
re-encrypting itself.

## How decrypting works

Vita save folders (`ux0:user/00/savedata/<TITLEID>`) are PFS-encrypted. The
system decrypts them transparently once the folder is *mounted*. Homebrew
cannot normally mount another title's save folder; VitaShell's kernel module
(`kernel.skprx`) plus its user module (`user.suprx`) expose
`shellUserMountById`, which can. Apollo Save Tool uses exactly this.

1. At startup, load the kernel module with `taiLoadKernelModule` /
   `taiStartKernelModule` (skipped when `VitaShellKernel2` is already loaded),
   then the user module with `sceKernelLoadStartModule`. Each is looked up in
   `ux0:VitaShell/module/` first and in the app's own `sce_module/` second.
2. To read or write, call `shellUserMountById` with mount id `0x3ED`
   (ux0 savedata), falling back to the other ids VitaShell tries
   (`0x6E, 0x12E, 0x12F`). While mounted, the files are read and written in
   plain form **through their normal path**; the returned mount point is only
   used to unmount (`sceAppMgrUmount`).
3. Mount → do one read or one write → unmount. The folder is never left
   mounted while the user is editing.

The app must be built as an **unsafe** homebrew (`vita_create_self ... UNSAFE`).

## Screens

### Save list (replaces the file browser)

- On entry, scans the known title ids:
  `PCSA00133` (US collection), `PCSF00482` (EU trilogy), plus
  `PCSF00484`–`PCSF00486`, which are unconfirmed guesses for EU single-game
  releases and harmless if absent.
- Inside each folder, lists every file ≥ 650 000 bytes, one level of
  subfolders deep, skipping `sce_sys` and `sce_pfs`. Each row shows the game
  (from file size, as in 1.x), the file name, the region label and the size.
- Empty result: a message explaining that no Ratchet & Clank save was found,
  and that Triangle shows every save folder on the card.
- Controls: D-pad / touch to move, X to open, Square to rescan, Triangle to
  toggle between "Ratchet & Clank only" and "all save folders", START to exit.
  There is no way to navigate to arbitrary folders.
- If the modules failed to load, the screen shows the error and what to do
  (enable unsafe homebrew in HENkaku settings, reboot) instead of the list.

### Editor

Unchanged, except:

- Save / Reset / load failures and successes are shown as a short status
  message rather than silently ignored.
- The header shows the game name and title id.

## Saving

- The first time a loaded save is written, the bytes as originally loaded are
  written, decrypted, to
  `ux0:data/slimseditor/backups/<TITLEID>/<file>.bak`. A failed backup aborts
  the save.
- The file is then overwritten in place inside the mounted folder. No
  temp-file-and-rename inside the encrypted folder.
- The 1.x checksum behaviour is kept as is.
- Saving while the game is running may fail to mount; the error is shown.

## Code layout

| File | Role |
|------|------|
| `src/core/save_store.{h,cpp}` | module loading, scan, mount/read/write, backup |
| `src/core/save_file.{h,cpp}` | in-memory save; loads from / saves to a `SaveSlot` |
| `src/ui/save_browser.{h,cpp}` | save list screen |
| `src/ui/file_browser.*` | removed |
| `libs/` | VitaShell `kernel.skprx`, `user.suprx`, user stub library |

## Not verified

- Built for the Vita toolchain only; behaviour on hardware has to be tested
  on a console. Kernel modules do not run in Vita3K.
- The EU title ids other than `PCSF00482`, and the save file names, were not
  confirmed; the scan does not depend on file names for that reason.

## Licensing note

The bundled VitaShell modules are GPLv3 (© TheFloW); this project is MIT. The
binaries are shipped unmodified and credited in the README.
