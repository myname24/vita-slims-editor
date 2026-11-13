// save_browser.cpp - v2.0 save list screen
#include "save_browser.h"
#include "../utils/colors.h"
#include "../data/rac_vita_games_data.h"
#include <cstdio>

namespace {
const int VISIBLE_ITEMS = 6;
const int LIST_TOP = 140;
const int LIST_BOTTOM = 500;
const int ROW_PITCH = 60;
const int STATUS_FRAMES = 240; // ~4 seconds
}

SaveBrowser::SaveBrowser(SDL_Renderer* r, TTF_Font* f)
    : renderer(r), font(f), knownOnly(true), storeReady(false), statusTimer(0),
      selectedIndex(0), scrollOffset(0), scrollTimer(0), openRequested(false),
      touchStartY(0), touchStartScroll(0), wasTouching(false), tapOnSelected(false) {
}

SaveBrowser::~SaveBrowser() {
}

void SaveBrowser::SetStoreStatus(bool ready, const std::string& error) {
    storeReady = ready;
    storeError = error;
}

void SaveBrowser::Rescan() {
    slots = SaveStore::Scan(knownOnly);
    selectedIndex = 0;
    scrollOffset = 0;
    openRequested = false;
    wasTouching = false;
    tapOnSelected = false;
}

void SaveBrowser::SetStatus(const std::string& message) {
    statusMessage = message;
    statusTimer = STATUS_FRAMES;
}

bool SaveBrowser::HasSelection() const {
    return storeReady && selectedIndex >= 0 && selectedIndex < (int)slots.size();
}

const SaveSlot& SaveBrowser::GetSelectedSlot() const {
    static const SaveSlot empty;
    if (selectedIndex >= 0 && selectedIndex < (int)slots.size()) {
        return slots[selectedIndex];
    }
    return empty;
}

bool SaveBrowser::ConsumeOpenRequest() {
    bool requested = openRequested;
    openRequested = false;
    return requested;
}

void SaveBrowser::Update(const InputState& input) {
    if (statusTimer > 0) statusTimer--;

    // Rescan
    if (input.IsPressed(SCE_CTRL_SQUARE)) {
        Rescan();
        return;
    }

    // Toggle between R&C saves and all save folders
    if (input.IsPressed(SCE_CTRL_TRIANGLE)) {
        knownOnly = !knownOnly;
        Rescan();
        return;
    }

    if (!storeReady || slots.empty()) return;

    int maxScroll = ((int)slots.size() > VISIBLE_ITEMS) ? ((int)slots.size() - VISIBLE_ITEMS) : 0;

    // D-PAD navigation with continuous scrolling
    bool moveDown = input.IsPressed(SCE_CTRL_DOWN);
    bool moveUp = input.IsPressed(SCE_CTRL_UP);

    if (input.IsHeld(SCE_CTRL_DOWN)) {
        scrollTimer++;
        if (scrollTimer > 15 && (scrollTimer % 3 == 0)) moveDown = true;
    } else if (input.IsHeld(SCE_CTRL_UP)) {
        scrollTimer++;
        if (scrollTimer > 15 && (scrollTimer % 3 == 0)) moveUp = true;
    } else {
        scrollTimer = 0;
    }

    if (moveDown && selectedIndex < (int)slots.size() - 1) {
        selectedIndex++;
        if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
        }
    }

    if (moveUp && selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
    }

    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    if (scrollOffset < 0) scrollOffset = 0;

    // TOUCH: drag to scroll, tap to select, tap the selected row to open
    bool inList = input.touchY >= LIST_TOP && input.touchY < LIST_BOTTOM;

    if (input.touchPressed && inList) {
        touchStartY = input.touchY;
        touchStartScroll = scrollOffset;
        wasTouching = true;
        tapOnSelected = false;

        int row = scrollOffset + (input.touchY - LIST_TOP) / ROW_PITCH;
        if (row >= 0 && row < (int)slots.size() && row < scrollOffset + VISIBLE_ITEMS) {
            if (selectedIndex == row) {
                tapOnSelected = true; // opens on release unless it turns into a drag
            } else {
                selectedIndex = row;
            }
        }
    } else if (wasTouching && !input.touchReleased && inList) {
        int dragDistance = touchStartY - input.touchY;
        int scrollDelta = dragDistance / ROW_PITCH;

        if (scrollDelta != 0) {
            // It became a drag, not a tap
            tapOnSelected = false;
        }

        scrollOffset = touchStartScroll + scrollDelta;
        if (scrollOffset < 0) scrollOffset = 0;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;

        // Keep selection visible while scrolling
        if (selectedIndex < scrollOffset) {
            selectedIndex = scrollOffset;
        } else if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
            selectedIndex = scrollOffset + VISIBLE_ITEMS - 1;
        }
        if (selectedIndex >= (int)slots.size()) selectedIndex = (int)slots.size() - 1;
        if (selectedIndex < 0) selectedIndex = 0;
    }

    if (input.touchReleased) {
        if (wasTouching && tapOnSelected) openRequested = true;
        wasTouching = false;
        tapOnSelected = false;
    }
}

void SaveBrowser::Render() {
    if (!renderer || !font) return;
    RenderHeader();
    if (!storeReady) {
        RenderUnavailable();
    } else {
        RenderList();
    }
    RenderFooter();
}

void SaveBrowser::DrawText(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty()) return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

void SaveBrowser::DrawTextCentered(const std::string& text, int y, SDL_Color color) {
    if (text.empty()) return;
    int w = 0, h = 0;
    if (TTF_SizeUTF8(font, text.c_str(), &w, &h) != 0) return;
    DrawText(text, 960 / 2 - w / 2, y, color);
}

std::vector<std::string> SaveBrowser::WrapText(const std::string& text, int maxWidth) {
    std::vector<std::string> lines;
    std::string line;
    size_t pos = 0;

    while (pos < text.size()) {
        size_t end = text.find(' ', pos);
        if (end == std::string::npos) end = text.size();
        std::string word = text.substr(pos, end - pos);
        pos = end + 1;
        if (word.empty()) continue;

        std::string candidate = line.empty() ? word : line + " " + word;
        int w = 0, h = 0;
        if (!line.empty() && TTF_SizeUTF8(font, candidate.c_str(), &w, &h) == 0 && w > maxWidth) {
            lines.push_back(line);
            line = word;
        } else {
            line = candidate;
        }
    }

    if (!line.empty()) lines.push_back(line);
    return lines;
}

void SaveBrowser::RenderHeader() {
    // Header background
    SDL_Color panelColor = Colors::Panel();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect headerRect = {0, 0, 960, 80};
    SDL_RenderFillRect(renderer, &headerRect);

    DrawText("SAVE DATA", 20, 15, Colors::Accent());
    DrawText(knownOnly ? "Ratchet & Clank saves" : "All save folders", 20, 50, Colors::TextDim());

    // Control bar
    SDL_Color selectedColor = Colors::Selected();
    SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, selectedColor.a);
    SDL_Rect controlRect = {0, 80, 960, 40};
    SDL_RenderFillRect(renderer, &controlRect);

    std::string controls = std::string("X: Open | []: Rescan | /\\: ") +
                           (knownOnly ? "All saves" : "R&C only") + " | START: Exit";
    DrawText(controls, 20, 90, Colors::Text());
}

void SaveBrowser::RenderList() {
    if (slots.empty()) {
        SDL_Color dimColor = Colors::TextDim();
        if (knownOnly) {
            DrawTextCentered("No Ratchet & Clank save data found.", 250, Colors::Text());
            DrawTextCentered("Play the game once to create a save, or press /\\ to list all save folders.",
                             285, dimColor);
        } else {
            DrawTextCentered("No save files found.", 262, dimColor);
        }
        return;
    }

    int lineH = TTF_FontHeight(font);
    int y = LIST_TOP;

    for (int i = scrollOffset; i < (int)slots.size() && i < scrollOffset + VISIBLE_ITEMS; i++) {
        const SaveSlot& slot = slots[i];
        bool selected = (i == selectedIndex);

        SDL_Color bgColor = selected ? Colors::Selected() : Colors::PanelDark();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {20, y, 920, ROW_PITCH - 4};
        SDL_RenderFillRect(renderer, &itemRect);

        // Line 1: game (from file size), or the file name when unrecognised
        GameType type = DetectGameTypeBySize(slot.size);
        std::string title = (type == GameType::UNKNOWN) ? slot.fileName : GetGameName(type);
        DrawText(title, 40, y + 4, selected ? Colors::Accent() : Colors::Text());

        // Line 2: region label and file name
        DrawText(slot.label + " - " + slot.fileName, 40, y + 4 + lineH + 2, Colors::TextDim());

        // Size, right-aligned
        std::string sizeStr = FormatSize(slot.size);
        int w = 0, h = 0;
        if (TTF_SizeUTF8(font, sizeStr.c_str(), &w, &h) == 0) {
            DrawText(sizeStr, 920 - w, y + (ROW_PITCH - 4 - h) / 2, Colors::TextDim());
        }

        y += ROW_PITCH;
    }
}

void SaveBrowser::RenderUnavailable() {
    SDL_Color panelColor = Colors::Panel();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect panelRect = {30, 160, 900, 300};
    SDL_RenderFillRect(renderer, &panelRect);

    SDL_Color borderColor = Colors::Border();
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &panelRect);

    int lineH = TTF_FontHeight(font) + 6;
    int y = 185;

    DrawTextCentered("Save access is not available", y, Colors::Error());
    y += lineH + 14;

    std::vector<std::string> lines = WrapText(storeError, 860);
    for (size_t i = 0; i < lines.size() && i < 6; i++) {
        DrawTextCentered(lines[i], y, Colors::Text());
        y += lineH;
    }
    y += 14;

    DrawTextCentered("Enable 'Unsafe Homebrew' in HENkaku Settings, then restart the app.",
                     y, Colors::TextDim());
}

void SaveBrowser::RenderFooter() {
    SDL_Color panelColor = Colors::PanelDark();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect footerRect = {0, 500, 960, 44};
    SDL_RenderFillRect(renderer, &footerRect);

    // A transient status message takes the place of the item count
    if (statusTimer > 0 && !statusMessage.empty()) {
        DrawText(statusMessage, 20, 510, Colors::Error());
        return;
    }

    if (!storeReady) return;

    char info[128];
    int count = (int)slots.size();
    snprintf(info, sizeof(info), "%d saves | Selected: %d/%d",
             count, count > 0 ? selectedIndex + 1 : 0, count);
    DrawText(info, 20, 510, Colors::TextDim());
}

std::string SaveBrowser::FormatSize(size_t bytes) {
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
