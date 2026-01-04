// file_browser.cpp - FIXED scrolling and last items visibility
#include "file_browser.h"
#include "../utils/colors.h"
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <algorithm>
#include <cstdio>

FileBrowser::FileBrowser(SDL_Renderer* r, TTF_Font* f) 
    : renderer(r), font(f), selectedIndex(0), scrollOffset(0), needsRescan(true),
      touchStartY(0), touchStartScroll(0), isDragging(false) {
    currentPath = "ux0:/data/slimseditor/saves";
}

FileBrowser::~FileBrowser() {
}

void FileBrowser::SetPath(const std::string& path) {
    currentPath = path;
    needsRescan = true;
}

std::string FileBrowser::GetSelectedPath() const {
    if (selectedIndex >= 0 && selectedIndex < (int)entries.size()) {
        return entries[selectedIndex].fullPath;
    }
    return "";
}

bool FileBrowser::HasSelection() const {
    return selectedIndex >= 0 && selectedIndex < (int)entries.size() && 
           !entries[selectedIndex].isDirectory;
}

void FileBrowser::ScanDirectory() {
    entries.clear();
    
    // Add parent directory entry if not at root
    if (currentPath != "ux0:" && currentPath.find('/') != std::string::npos) {
        FileEntry parent;
        parent.name = ".. (Parent Directory)";
        parent.isDirectory = true;
        
        // Get parent path
        size_t lastSlash = currentPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            parent.fullPath = currentPath.substr(0, lastSlash);
            if (parent.fullPath.empty()) parent.fullPath = "ux0:";
        }
        
        entries.push_back(parent);
    }
    
    SceUID dir = sceIoDopen(currentPath.c_str());
    if (dir < 0) {
        needsRescan = false;
        return;
    }
    
    SceIoDirent dirent;
    while (sceIoDread(dir, &dirent) > 0) {
        if (dirent.d_name[0] == '.') continue;
        
        FileEntry entry;
        entry.name = dirent.d_name;
        entry.fullPath = currentPath + "/" + dirent.d_name;
        entry.isDirectory = SCE_S_ISDIR(dirent.d_stat.st_mode);
        entry.size = dirent.d_stat.st_size;
        
        entries.push_back(entry);
    }
    
    sceIoDclose(dir);
    
    // Sort: directories first, then files, alphabetically
    std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
        if (a.name == ".. (Parent Directory)") return true;
        if (b.name == ".. (Parent Directory)") return false;
        if (a.isDirectory != b.isDirectory) return a.isDirectory;
        return a.name < b.name;
    });
    
    selectedIndex = 0;
    scrollOffset = 0;
    needsRescan = false;
}

void FileBrowser::Update(const InputState& input) {
    if (needsRescan) {
        ScanDirectory();
    }
    
    if (entries.empty()) return;
    
    const int VISIBLE_ITEMS = 6;
    int maxScroll = (entries.size() > VISIBLE_ITEMS) ? (entries.size() - VISIBLE_ITEMS) : 0;
    
    // D-PAD navigation with FIXED scrolling
    if (input.IsPressed(SCE_CTRL_DOWN) && selectedIndex < (int)entries.size() - 1) {
        selectedIndex++;
        
        // FIXED: Ensure last items are visible
        if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
        }
    }
    
    if (input.IsPressed(SCE_CTRL_UP) && selectedIndex > 0) {
        selectedIndex--;
        
        // FIXED: Proper scroll up
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
    }
    
    // Clamp scroll
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Enter directory or select file
    if (input.IsPressed(SCE_CTRL_CROSS) && selectedIndex >= 0 && selectedIndex < (int)entries.size()) {
        if (entries[selectedIndex].isDirectory) {
            SetPath(entries[selectedIndex].fullPath);
        }
        // If it's a file, parent will handle it
    }
    
    // Refresh
    if (input.IsPressed(SCE_CTRL_SQUARE)) {
        needsRescan = true;
    }
    
    // Go to parent with Circle
    if (input.IsPressed(SCE_CTRL_CIRCLE) && !entries.empty() && entries[0].name == ".. (Parent Directory)") {
        SetPath(entries[0].fullPath);
    }
    
    // TOUCH SCROLLING - Fixed implementation
    if (input.touchPressed && input.touchY >= 140 && input.touchY < 500 && !isDragging) {
        touchStartY = input.touchY;
        touchStartScroll = scrollOffset;
        isDragging = true;
    }
    
    // While dragging
    if (isDragging) {
        // Check if still touching in the list area
        if (input.touchY >= 140 && input.touchY < 500) {
            int dragDistance = touchStartY - input.touchY;
            int scrollDelta = dragDistance / 60;  // Each item is 60px
            
            scrollOffset = touchStartScroll + scrollDelta;
            
            // Clamp
            if (scrollOffset < 0) scrollOffset = 0;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        }
    }
    
    if (input.touchReleased) {
        isDragging = false;
    }
    
    // Touch selection (tap, not drag)
    if (input.touchPressed && !isDragging && input.touchY >= 140 && input.touchY < 500) {
        int y = 140;
        for (int i = scrollOffset; i < (int)entries.size() && i < scrollOffset + VISIBLE_ITEMS; i++) {
            if (input.touchY >= y && input.touchY < y + 60) {
                if (selectedIndex == i) {
                    // Double tap - enter directory or select file
                    if (entries[i].isDirectory) {
                        SetPath(entries[i].fullPath);
                    }
                } else {
                    selectedIndex = i;
                }
                break;
            }
            y += 60;
        }
    }
}

void FileBrowser::Render() {
    RenderHeader();
    RenderFileList();
    RenderFooter();
}

void FileBrowser::RenderHeader() {
    // Header background
    SDL_Color panelColor = Colors::Panel();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect headerRect = {0, 0, 960, 80};
    SDL_RenderFillRect(renderer, &headerRect);
    
    // Title
    SDL_Color accentColor = Colors::Accent();
    SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(font, "FILE BROWSER", accentColor);
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_Rect titleRect = {20, 15, titleSurface->w, titleSurface->h};
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
        SDL_DestroyTexture(titleTexture);
        SDL_FreeSurface(titleSurface);
    }
    
    // Current path
    SDL_Color textDimColor = Colors::TextDim();
    SDL_Surface* pathSurface = TTF_RenderUTF8_Blended(font, currentPath.c_str(), textDimColor);
    if (pathSurface) {
        SDL_Texture* pathTexture = SDL_CreateTextureFromSurface(renderer, pathSurface);
        SDL_Rect pathRect = {20, 50, pathSurface->w, pathSurface->h};
        SDL_RenderCopy(renderer, pathTexture, nullptr, &pathRect);
        SDL_DestroyTexture(pathTexture);
        SDL_FreeSurface(pathSurface);
    }
    
    // Control bar
    SDL_Color selectedColor = Colors::Selected();
    SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, selectedColor.a);
    SDL_Rect controlRect = {0, 80, 960, 40};
    SDL_RenderFillRect(renderer, &controlRect);
    
    SDL_Color textColor = Colors::Text();
    SDL_Surface* controlSurface = TTF_RenderUTF8_Blended(font, "D-Pad: Navigate | X: Select | O: Parent | []: Refresh", textColor);
    if (controlSurface) {
        SDL_Texture* controlTexture = SDL_CreateTextureFromSurface(renderer, controlSurface);
        SDL_Rect controlTextRect = {20, 90, controlSurface->w, controlSurface->h};
        SDL_RenderCopy(renderer, controlTexture, nullptr, &controlTextRect);
        SDL_DestroyTexture(controlTexture);
        SDL_FreeSurface(controlSurface);
    }
}

void FileBrowser::RenderFileList() {
    int y = 140;
    int visibleCount = 0;
    const int VISIBLE_ITEMS = 6;
    
    for (int i = scrollOffset; i < (int)entries.size() && visibleCount < VISIBLE_ITEMS; i++, visibleCount++) {
        bool selected = (i == selectedIndex);
        
        // Background
        SDL_Color bgColor = selected ? Colors::Selected() : Colors::Panel();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {10, y - 5, 940, 55};
        SDL_RenderFillRect(renderer, &itemRect);
        
        // Icon and name
        std::string displayName = entries[i].isDirectory ? "[DIR] " + entries[i].name : entries[i].name;
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        
        SDL_Surface* nameSurface = TTF_RenderUTF8_Blended(font, displayName.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            SDL_Rect nameRect = {25, y + 5, nameSurface->w, nameSurface->h};
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameSurface);
        }
        
        // Size (for files)
        if (!entries[i].isDirectory) {
            std::string sizeStr = FormatSize(entries[i].size);
            SDL_Color dimColor = Colors::TextDim();
            SDL_Surface* sizeSurface = TTF_RenderUTF8_Blended(font, sizeStr.c_str(), dimColor);
            if (sizeSurface) {
                SDL_Texture* sizeTexture = SDL_CreateTextureFromSurface(renderer, sizeSurface);
                SDL_Rect sizeRect = {920 - sizeSurface->w, y + 5, sizeSurface->w, sizeSurface->h};
                SDL_RenderCopy(renderer, sizeTexture, nullptr, &sizeRect);
                SDL_DestroyTexture(sizeTexture);
                SDL_FreeSurface(sizeSurface);
            }
        }
        
        y += 60;
    }
    
    // Empty state
    if (entries.empty()) {
        SDL_Color dimColor = Colors::TextDim();
        SDL_Surface* emptySurface = TTF_RenderUTF8_Blended(font, "Empty directory", dimColor);
        if (emptySurface) {
            SDL_Texture* emptyTexture = SDL_CreateTextureFromSurface(renderer, emptySurface);
            SDL_Rect emptyRect = {960/2 - emptySurface->w/2, 272 - emptySurface->h/2, emptySurface->w, emptySurface->h};
            SDL_RenderCopy(renderer, emptyTexture, nullptr, &emptyRect);
            SDL_DestroyTexture(emptyTexture);
            SDL_FreeSurface(emptySurface);
        }
    }
}

void FileBrowser::RenderFooter() {
    SDL_Color panelColor = Colors::PanelDark();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect footerRect = {0, 500, 960, 44};
    SDL_RenderFillRect(renderer, &footerRect);
    
    char info[128];
    snprintf(info, sizeof(info), "%d items | Selected: %d/%d", 
             (int)entries.size(), selectedIndex + 1, (int)entries.size());
    
    SDL_Color dimColor = Colors::TextDim();
    SDL_Surface* infoSurface = TTF_RenderUTF8_Blended(font, info, dimColor);
    if (infoSurface) {
        SDL_Texture* infoTexture = SDL_CreateTextureFromSurface(renderer, infoSurface);
        SDL_Rect infoRect = {20, 510, infoSurface->w, infoSurface->h};
        SDL_RenderCopy(renderer, infoTexture, nullptr, &infoRect);
        SDL_DestroyTexture(infoTexture);
        SDL_FreeSurface(infoSurface);
    }
}

std::string FileBrowser::FormatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = (double)bytes;
    
    while (size >= 1024.0 && unit < 3) {
        size /= 1024.0;
        unit++;
    }
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unit]);
    return std::string(buffer);
}