// file_browser.h - Updated with touch scrolling support
#pragma once
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../utils/input.h"

struct FileEntry {
    std::string name;
    std::string fullPath;
    bool isDirectory;
    size_t size;
};

class FileBrowser {
public:
    FileBrowser(SDL_Renderer* renderer, TTF_Font* font);
    ~FileBrowser();
    
    void SetPath(const std::string& path);
    std::string GetSelectedPath() const;
    
    void Update(const InputState& input);
    void Render();
    
    bool HasSelection() const;
    
private:
    void ScanDirectory();
    void RenderFileList();
    void RenderHeader();
    void RenderFooter();
    std::string FormatSize(size_t bytes);
    
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::string currentPath;
    std::vector<FileEntry> entries;
    int selectedIndex;
    int scrollOffset;
    bool needsRescan;
    
    // Touch scrolling
    int touchStartY;
    int touchStartScroll;
    bool isDragging;
};