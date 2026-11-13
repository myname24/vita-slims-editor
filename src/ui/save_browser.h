// save_browser.h - v2.0 save list screen (replaces the file browser)
#pragma once
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../core/save_store.h"
#include "../utils/input.h"

class SaveBrowser {
public:
    SaveBrowser(SDL_Renderer* renderer, TTF_Font* font);
    ~SaveBrowser();

    // Result of SaveStore::Init; when not ready the error is shown instead of the list
    void SetStoreStatus(bool ready, const std::string& error);
    void Rescan();

    // Transient message shown in the footer (e.g. a load error)
    void SetStatus(const std::string& message);

    void Update(const InputState& input);
    void Render();

    bool HasSelection() const;
    const SaveSlot& GetSelectedSlot() const;

    // True once after the user tapped the already-selected row (touch equivalent of X)
    bool ConsumeOpenRequest();

private:
    void RenderHeader();
    void RenderList();
    void RenderUnavailable();
    void RenderFooter();
    void DrawText(const std::string& text, int x, int y, SDL_Color color);
    void DrawTextCentered(const std::string& text, int y, SDL_Color color);
    std::vector<std::string> WrapText(const std::string& text, int maxWidth);
    std::string FormatSize(size_t bytes);

    SDL_Renderer* renderer;
    TTF_Font* font;

    std::vector<SaveSlot> slots;
    bool knownOnly;
    bool storeReady;
    std::string storeError;
    std::string statusMessage;
    int statusTimer;

    int selectedIndex;
    int scrollOffset;
    int scrollTimer;
    bool openRequested;

    // Touch scrolling
    int touchStartY;
    int touchStartScroll;
    bool wasTouching;
    bool tapOnSelected;
};
