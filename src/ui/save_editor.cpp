// save_editor.cpp - SIMPLIFIED - Max ammo only, no detection
#include "save_editor.h"
#include "../utils/colors.h"
#include "../data/rac_vita_games_data.h"
#include <algorithm>
#include <cstdio>

SaveEditor::SaveEditor(SDL_Renderer* r, TTF_Font* f) 
    : renderer(r), font(f), saveFile(nullptr), 
      currentGameType(GameType::UNKNOWN),
      currentGameData(), 
      currentTab(EditorTab::VALUES), selectedIndex(0), scrollOffset(0), 
      wantsBack(false), isEditing(false), editingValue(0), editingMultiplier(1) {
}

SaveEditor::~SaveEditor() {
}

void SaveEditor::SetSaveFile(SaveFile* save) {
    saveFile = save;
    
    if (saveFile && saveFile->IsLoaded()) {
        // AUTO-DETECT GAME TYPE
        std::vector<uint8_t> data;
        data.resize(saveFile->GetSize());
        for (size_t i = 0; i < saveFile->GetSize(); i++) {
            data[i] = saveFile->ReadByte(i);
        }
        
        currentGameType = DetectGameType(data);
        currentGameData = GameData(currentGameType);
        
        selectedIndex = 0;
        scrollOffset = 0;
        currentTab = EditorTab::VALUES;
        isEditing = false;
    }
}

void SaveEditor::Update(const InputState& input) {
    if (!saveFile) return;
    
    // If we're in editing mode, handle editing controls
    if (isEditing) {
        UpdateEditingMode(input);
        return;
    }
    
    // Touch button detection
    if (input.touchPressed) {
        // Save button (720, 20, 100, 40)
        if (input.touchX >= 720 && input.touchX <= 820 &&
            input.touchY >= 20 && input.touchY <= 60) {
            if (saveFile->Save()) {
                // Successfully saved
            }
            return;
        }
        
        // Back button (830, 20, 100, 40)
        if (input.touchX >= 830 && input.touchX <= 930 &&
            input.touchY >= 20 && input.touchY <= 60) {
            wantsBack = true;
            return;
        }
    }
    
    // Tab switching with L/R
    if (input.IsPressed(SCE_CTRL_LTRIGGER)) {
        int tab = (int)currentTab;
        tab = (tab - 1 + 4) % 4;
        currentTab = (EditorTab)tab;
        selectedIndex = 0;
        scrollOffset = 0;
    }
    
    if (input.IsPressed(SCE_CTRL_RTRIGGER)) {
        int tab = (int)currentTab;
        tab = (tab + 1) % 4;
        currentTab = (EditorTab)tab;
        selectedIndex = 0;
        scrollOffset = 0;
    }
    
    // Get current list size
    int listSize = 0;
    switch (currentTab) {
        case EditorTab::VALUES:
            listSize = currentGameData.values.size() + currentGameData.extra_values.size();
            break;
        case EditorTab::WEAPONS:
            listSize = currentGameData.weapons.size();
            break;
        case EditorTab::GADGETS:
            listSize = currentGameData.gadgets.size();
            break;
        case EditorTab::UNLOCKABLES:
            listSize = currentGameData.unlockables.size();
            break;
    }
    
    if (listSize == 0) return;
    
    const int VISIBLE_ITEMS = 6;
    int maxScroll = (listSize > VISIBLE_ITEMS) ? (listSize - VISIBLE_ITEMS) : 0;
    
    // Navigation
    if (input.IsPressed(SCE_CTRL_DOWN)) {
        selectedIndex++;
        if (selectedIndex >= listSize) selectedIndex = listSize - 1;
        
        if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
        }
    }
    
    if (input.IsPressed(SCE_CTRL_UP)) {
        selectedIndex--;
        if (selectedIndex < 0) selectedIndex = 0;
        
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
    }
    
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Edit/Toggle with X
    if (input.IsPressed(SCE_CTRL_CROSS)) {
        HandleCrossPress();
    }
    
    // Save with START
    if (input.IsPressed(SCE_CTRL_START)) {
        saveFile->Save();
    }
    
    // Back with CIRCLE
    if (input.IsPressed(SCE_CTRL_CIRCLE)) {
        wantsBack = true;
    }
    
    // Touch tab switching
    if (input.touchPressed && input.touchY >= 80 && input.touchY < 120) {
        int tabIndex = input.touchX / 240;
        if (tabIndex >= 0 && tabIndex < 4) {
            currentTab = (EditorTab)tabIndex;
            selectedIndex = 0;
            scrollOffset = 0;
        }
    }
    
    // Touch item selection
    if (input.touchPressed && input.touchY >= 140 && input.touchY < 500) {
        int itemY = 140;
        for (int i = scrollOffset; i < listSize && i < scrollOffset + VISIBLE_ITEMS; i++) {
            if (input.touchY >= itemY && input.touchY < itemY + 60) {
                selectedIndex = i;
                
                // For gadgets/unlockables, toggle on touch
                if ((currentTab == EditorTab::GADGETS || currentTab == EditorTab::UNLOCKABLES) && 
                    input.touchX >= 20 && input.touchX <= 50) {
                    HandleCrossPress();
                }
                break;
            }
            itemY += 60;
        }
    }
    
    // TOUCH SCROLLING
    static int touchStartY = 0;
    static int touchStartScroll = 0;
    static bool wasTouching = false;
    
    if (input.touchPressed && input.touchY >= 140 && input.touchY < 500) {
        if (!wasTouching) {
            touchStartY = input.touchY;
            touchStartScroll = scrollOffset;
            wasTouching = true;
        }
    }
    
    if (wasTouching && input.touchY >= 140 && input.touchY < 500) {
        int dragDistance = touchStartY - input.touchY;
        int scrollDelta = dragDistance / 60;
        
        scrollOffset = touchStartScroll + scrollDelta;
        
        if (scrollOffset < 0) scrollOffset = 0;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    }
    
    if (input.touchReleased) {
        wasTouching = false;
    }
}

void SaveEditor::UpdateEditingMode(const InputState& input) {
    // Change multiplier with L/R
    if (input.IsPressed(SCE_CTRL_LTRIGGER)) {
        if (editingMultiplier == 1) editingMultiplier = 1000000;
        else if (editingMultiplier == 10) editingMultiplier = 1;
        else if (editingMultiplier == 100) editingMultiplier = 10;
        else if (editingMultiplier == 1000) editingMultiplier = 100;
        else if (editingMultiplier == 10000) editingMultiplier = 1000;
        else if (editingMultiplier == 100000) editingMultiplier = 10000;
        else if (editingMultiplier == 1000000) editingMultiplier = 100000;
    }
    
    if (input.IsPressed(SCE_CTRL_RTRIGGER)) {
        if (editingMultiplier == 1) editingMultiplier = 10;
        else if (editingMultiplier == 10) editingMultiplier = 100;
        else if (editingMultiplier == 100) editingMultiplier = 1000;
        else if (editingMultiplier == 1000) editingMultiplier = 10000;
        else if (editingMultiplier == 10000) editingMultiplier = 100000;
        else if (editingMultiplier == 100000) editingMultiplier = 1000000;
        else if (editingMultiplier == 1000000) editingMultiplier = 1;
    }
    
    // Adjust value with D-pad
    if (input.IsPressed(SCE_CTRL_UP) || input.IsHeld(SCE_CTRL_UP)) {
        editingValue += editingMultiplier;
        if (editingValue > editingMaxValue) editingValue = editingMaxValue;
    }
    
    if (input.IsPressed(SCE_CTRL_DOWN) || input.IsHeld(SCE_CTRL_DOWN)) {
        editingValue -= editingMultiplier;
        if (editingValue < editingMinValue) editingValue = editingMinValue;
    }
    
    // Quick min/max with LEFT/RIGHT
    if (input.IsPressed(SCE_CTRL_LEFT)) {
        editingValue = editingMinValue;
    }
    
    if (input.IsPressed(SCE_CTRL_RIGHT)) {
        editingValue = editingMaxValue;
    }
    
    // Confirm with X
    if (input.IsPressed(SCE_CTRL_CROSS)) {
        SaveEditedValue();
        isEditing = false;
    }
    
    // Cancel with O
    if (input.IsPressed(SCE_CTRL_CIRCLE)) {
        isEditing = false;
    }
}

void SaveEditor::HandleCrossPress() {
    switch (currentTab) {
        case EditorTab::VALUES:
            EditValue();
            break;
        case EditorTab::WEAPONS:
            EditWeaponAmmo();
            break;
        case EditorTab::GADGETS:
            ToggleGadget();
            break;
        case EditorTab::UNLOCKABLES:
            ToggleUnlockable();
            break;
    }
}

void SaveEditor::EditValue() {
    int totalValues = currentGameData.values.size() + currentGameData.extra_values.size();
    if (selectedIndex >= totalValues || !saveFile) return;
    
    const GameValue* value = nullptr;
    if (selectedIndex < (int)currentGameData.values.size()) {
        value = &currentGameData.values[selectedIndex];
    } else {
        value = &currentGameData.extra_values[selectedIndex - currentGameData.values.size()];
    }
    
    int current = saveFile->ReadInt32(value->offset);
    
    isEditing = true;
    editingValue = current;
    editingMinValue = value->min_value;
    editingMaxValue = value->max_value;
    editingMultiplier = 1000;
    editingOffset = value->offset;
}

void SaveEditor::EditWeaponAmmo() {
    if (selectedIndex >= (int)currentGameData.weapons.size() || !saveFile) return;
    
    const GameWeapon& weapon = currentGameData.weapons[selectedIndex];
    int current = saveFile->ReadInt32(weapon.ammo_offset);
    
    isEditing = true;
    editingValue = current;
    editingMinValue = weapon.min_ammo;
    editingMaxValue = weapon.max_ammo;  // Just use max_ammo directly!
    editingMultiplier = 10;
    editingOffset = weapon.ammo_offset;
}

void SaveEditor::SaveEditedValue() {
    if (saveFile) {
        saveFile->WriteInt32(editingOffset, editingValue);
    }
}

void SaveEditor::ToggleGadget() {
    if (selectedIndex >= (int)currentGameData.gadgets.size() || !saveFile) return;
    
    const GameGadget& gadget = currentGameData.gadgets[selectedIndex];
    bool current = saveFile->ReadBool(gadget.offset, gadget.bit_index);
    saveFile->WriteBool(gadget.offset, !current, gadget.bit_index);
}

void SaveEditor::ToggleUnlockable() {
    if (selectedIndex >= (int)currentGameData.unlockables.size() || !saveFile) return;
    
    const GameUnlockable& unlockable = currentGameData.unlockables[selectedIndex];
    bool current = saveFile->ReadBool(unlockable.offset, unlockable.bit_index);
    saveFile->WriteBool(unlockable.offset, !current, unlockable.bit_index);
}

void SaveEditor::Render() {
    if (isEditing) {
        RenderHeader();
        RenderTabs();
        RenderTabContent();
        RenderEditingOverlay();
        RenderFooter();
    } else {
        RenderHeader();
        RenderTabs();
        RenderTabContent();
        RenderFooter();
    }
}

void SaveEditor::RenderHeader() {
    SDL_Color panelTop = Colors::Panel();
    SDL_SetRenderDrawColor(renderer, panelTop.r, panelTop.g, panelTop.b, panelTop.a);
    SDL_Rect headerRect = {0, 0, 960, 80};
    SDL_RenderFillRect(renderer, &headerRect);
    
    SDL_Color accentColor = Colors::Accent();
    SDL_SetRenderDrawColor(renderer, accentColor.r, accentColor.g, accentColor.b, accentColor.a);
    SDL_Rect borderRect = {0, 78, 960, 2};
    SDL_RenderFillRect(renderer, &borderRect);
    
    // Game icon
    SDL_Color iconColor = Colors::Accent();
    SDL_SetRenderDrawColor(renderer, iconColor.r, iconColor.g, iconColor.b, iconColor.a);
    SDL_Rect iconRect = {15, 15, 50, 50};
    SDL_RenderFillRect(renderer, &iconRect);
    
    SDL_Color iconBorder = Colors::AccentHover();
    SDL_SetRenderDrawColor(renderer, iconBorder.r, iconBorder.g, iconBorder.b, iconBorder.a);
    SDL_RenderDrawRect(renderer, &iconRect);
    
    // Game name
    const char* gameName = currentGameData.name.c_str();
    if (currentGameType == GameType::UNKNOWN) {
        gameName = "UNKNOWN GAME";
    }
    
    SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(font, gameName, Colors::Text());
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_Rect titleRect = {75, 15, titleSurface->w, titleSurface->h};
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
        SDL_DestroyTexture(titleTexture);
        SDL_FreeSurface(titleSurface);
    }
    
    // Status indicator
    if (saveFile && saveFile->IsModified()) {
        SDL_Color warningColor = Colors::Warning();
        SDL_Surface* modSurface = TTF_RenderUTF8_Blended(font, "UNSAVED CHANGES", warningColor);
        if (modSurface) {
            SDL_Texture* modTexture = SDL_CreateTextureFromSurface(renderer, modSurface);
            SDL_Rect modRect = {75, 45, modSurface->w, modSurface->h};
            SDL_RenderCopy(renderer, modTexture, nullptr, &modRect);
            SDL_DestroyTexture(modTexture);
            SDL_FreeSurface(modSurface);
        }
    } else {
        SDL_Color successColor = Colors::Success();
        SDL_Surface* savedSurface = TTF_RenderUTF8_Blended(font, "ALL SAVED", successColor);
        if (savedSurface) {
            SDL_Texture* savedTexture = SDL_CreateTextureFromSurface(renderer, savedSurface);
            SDL_Rect savedRect = {75, 45, savedSurface->w, savedSurface->h};
            SDL_RenderCopy(renderer, savedTexture, nullptr, &savedRect);
            SDL_DestroyTexture(savedTexture);
            SDL_FreeSurface(savedSurface);
        }
    }
    
    // SAVE button
    SDL_Rect saveBtn = {720, 20, 100, 40};
    SDL_Color saveBg = Colors::Success();
    SDL_SetRenderDrawColor(renderer, saveBg.r, saveBg.g, saveBg.b, saveBg.a);
    SDL_RenderFillRect(renderer, &saveBtn);
    
    SDL_Color btnBorder = Colors::Border();
    SDL_SetRenderDrawColor(renderer, btnBorder.r, btnBorder.g, btnBorder.b, btnBorder.a);
    SDL_RenderDrawRect(renderer, &saveBtn);
    
    SDL_Surface* saveSurface = TTF_RenderUTF8_Blended(font, "SAVE", Colors::Text());
    if (saveSurface) {
        SDL_Texture* saveTexture = SDL_CreateTextureFromSurface(renderer, saveSurface);
        SDL_Rect saveTextRect = {
            saveBtn.x + (saveBtn.w - saveSurface->w) / 2,
            saveBtn.y + (saveBtn.h - saveSurface->h) / 2,
            saveSurface->w,
            saveSurface->h
        };
        SDL_RenderCopy(renderer, saveTexture, nullptr, &saveTextRect);
        SDL_DestroyTexture(saveTexture);
        SDL_FreeSurface(saveSurface);
    }
    
    // BACK button
    SDL_Rect backBtn = {830, 20, 100, 40};
    SDL_Color backBg = Colors::Error();
    SDL_SetRenderDrawColor(renderer, backBg.r, backBg.g, backBg.b, backBg.a);
    SDL_RenderFillRect(renderer, &backBtn);
    
    SDL_SetRenderDrawColor(renderer, btnBorder.r, btnBorder.g, btnBorder.b, btnBorder.a);
    SDL_RenderDrawRect(renderer, &backBtn);
    
    SDL_Surface* backSurface = TTF_RenderUTF8_Blended(font, "BACK", Colors::Text());
    if (backSurface) {
        SDL_Texture* backTexture = SDL_CreateTextureFromSurface(renderer, backSurface);
        SDL_Rect backTextRect = {
            backBtn.x + (backBtn.w - backSurface->w) / 2,
            backBtn.y + (backBtn.h - backSurface->h) / 2,
            backSurface->w,
            backSurface->h
        };
        SDL_RenderCopy(renderer, backTexture, nullptr, &backTextRect);
        SDL_DestroyTexture(backTexture);
        SDL_FreeSurface(backSurface);
    }
}

void SaveEditor::RenderTabs() {
    const char* tabNames[] = {"Values", "Weapons", "Gadgets", "Unlocks"};
    int tabWidth = 240;
    int tabY = 80;
    
    for (int i = 0; i < 4; i++) {
        bool selected = ((int)currentTab == i);
        
        SDL_Color bgColor = selected ? Colors::Selected() : Colors::PanelDark();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect tabRect = {i * tabWidth, tabY, tabWidth, 40};
        SDL_RenderFillRect(renderer, &tabRect);
        
        if (selected) {
            SDL_Color indicatorColor = Colors::Accent();
            SDL_SetRenderDrawColor(renderer, indicatorColor.r, indicatorColor.g, indicatorColor.b, indicatorColor.a);
            SDL_Rect indicator = {i * tabWidth, tabY + 37, tabWidth, 3};
            SDL_RenderFillRect(renderer, &indicator);
        }
        
        SDL_Color borderColor = Colors::Border();
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &tabRect);
        
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, tabNames[i], textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {
                i * tabWidth + (tabWidth - textSurface->w) / 2,
                tabY + (40 - textSurface->h) / 2,
                textSurface->w,
                textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}

void SaveEditor::RenderTabContent() {
    switch (currentTab) {
        case EditorTab::VALUES:
            RenderValuesTab();
            break;
        case EditorTab::WEAPONS:
            RenderWeaponsTab();
            break;
        case EditorTab::GADGETS:
            RenderGadgetsTab();
            break;
        case EditorTab::UNLOCKABLES:
            RenderUnlockablesTab();
            break;
    }
}

void SaveEditor::RenderValuesTab() {
    int y = 140;
    int visibleCount = 0;
    const int VISIBLE_ITEMS = 6;
    
    int totalValues = currentGameData.values.size() + currentGameData.extra_values.size();
    
    for (int i = scrollOffset; i < totalValues && visibleCount < VISIBLE_ITEMS; i++, visibleCount++) {
        const GameValue* value = nullptr;
        if (i < (int)currentGameData.values.size()) {
            value = &currentGameData.values[i];
        } else {
            value = &currentGameData.extra_values[i - currentGameData.values.size()];
        }
        
        bool selected = (i == selectedIndex);
        
        SDL_Color bgColor = selected ? Colors::SelectedLight() : Colors::Panel();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {15, y - 5, 930, 55};
        SDL_RenderFillRect(renderer, &itemRect);
        
        if (selected) {
            SDL_Color indicatorColor = Colors::Accent();
            SDL_SetRenderDrawColor(renderer, indicatorColor.r, indicatorColor.g, indicatorColor.b, indicatorColor.a);
            SDL_Rect indicator = {15, y - 5, 5, 55};
            SDL_RenderFillRect(renderer, &indicator);
        }
        
        SDL_Color borderColor = selected ? Colors::Accent() : Colors::Border();
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &itemRect);
        
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        SDL_Surface* nameSurface = TTF_RenderUTF8_Blended(font, value->name.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            SDL_Rect nameRect = {30, y + 5, nameSurface->w, nameSurface->h};
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameSurface);
        }
        
        int currentValue = saveFile->ReadInt32(value->offset);
        char valueBuf[64];
        snprintf(valueBuf, sizeof(valueBuf), "%d", currentValue);
        
        SDL_Color valueColor = Colors::AccentHover();
        SDL_Surface* valueSurface = TTF_RenderUTF8_Blended(font, valueBuf, valueColor);
        if (valueSurface) {
            SDL_Texture* valueTexture = SDL_CreateTextureFromSurface(renderer, valueSurface);
            SDL_Rect valueRect = {920 - valueSurface->w, y + 5, valueSurface->w, valueSurface->h};
            SDL_RenderCopy(renderer, valueTexture, nullptr, &valueRect);
            SDL_DestroyTexture(valueTexture);
            SDL_FreeSurface(valueSurface);
        }
        
        SDL_Color dimColor = Colors::TextDim();
        SDL_Surface* descSurface = TTF_RenderUTF8_Blended(font, value->description.c_str(), dimColor);
        if (descSurface) {
            SDL_Texture* descTexture = SDL_CreateTextureFromSurface(renderer, descSurface);
            SDL_Rect descRect = {30, y + 30, descSurface->w, descSurface->h};
            SDL_RenderCopy(renderer, descTexture, nullptr, &descRect);
            SDL_DestroyTexture(descTexture);
            SDL_FreeSurface(descSurface);
        }
        
        y += 60;
    }
}

void SaveEditor::RenderWeaponsTab() {
    int y = 140;
    int visibleCount = 0;
    const int VISIBLE_ITEMS = 6;
    
    for (int i = scrollOffset; i < (int)currentGameData.weapons.size() && visibleCount < VISIBLE_ITEMS; i++, visibleCount++) {
        const GameWeapon& weapon = currentGameData.weapons[i];
        bool selected = (i == selectedIndex);
        
        SDL_Color bgColor = selected ? Colors::SelectedLight() : Colors::Panel();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {15, y - 5, 930, 55};
        SDL_RenderFillRect(renderer, &itemRect);
        
        if (selected) {
            SDL_Color indicatorColor = Colors::Accent();
            SDL_SetRenderDrawColor(renderer, indicatorColor.r, indicatorColor.g, indicatorColor.b, indicatorColor.a);
            SDL_Rect indicator = {15, y - 5, 5, 55};
            SDL_RenderFillRect(renderer, &indicator);
        }
        
        SDL_Color borderColor = selected ? Colors::Accent() : Colors::Border();
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &itemRect);
        
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        SDL_Surface* nameSurface = TTF_RenderUTF8_Blended(font, weapon.name.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            SDL_Rect nameRect = {30, y + 5, nameSurface->w, nameSurface->h};
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameSurface);
        }
        
        // Simple display: current / max
        int ammo = saveFile->ReadInt32(weapon.ammo_offset);
        
        char ammoBuf[64];
        snprintf(ammoBuf, sizeof(ammoBuf), "%d / %d", ammo, weapon.max_ammo);
        
        SDL_Color valueColor = Colors::AccentHover();
        SDL_Surface* ammoSurface = TTF_RenderUTF8_Blended(font, ammoBuf, valueColor);
        if (ammoSurface) {
            SDL_Texture* ammoTexture = SDL_CreateTextureFromSurface(renderer, ammoSurface);
            SDL_Rect ammoRect = {920 - ammoSurface->w, y + 5, ammoSurface->w, ammoSurface->h};
            SDL_RenderCopy(renderer, ammoTexture, nullptr, &ammoRect);
            SDL_DestroyTexture(ammoTexture);
            SDL_FreeSurface(ammoSurface);
        }
        
        SDL_Color dimColor = Colors::TextDim();
        SDL_Surface* descSurface = TTF_RenderUTF8_Blended(font, weapon.description.c_str(), dimColor);
        if (descSurface) {
            SDL_Texture* descTexture = SDL_CreateTextureFromSurface(renderer, descSurface);
            SDL_Rect descRect = {30, y + 30, descSurface->w, descSurface->h};
            SDL_RenderCopy(renderer, descTexture, nullptr, &descRect);
            SDL_DestroyTexture(descTexture);
            SDL_FreeSurface(descSurface);
        }
        
        y += 60;
    }
}

void SaveEditor::RenderGadgetsTab() {
    int y = 140;
    int visibleCount = 0;
    const int VISIBLE_ITEMS = 6;
    
    for (int i = scrollOffset; i < (int)currentGameData.gadgets.size() && visibleCount < VISIBLE_ITEMS; i++, visibleCount++) {
        const GameGadget& gadget = currentGameData.gadgets[i];
        bool selected = (i == selectedIndex);
        bool owned = saveFile->ReadBool(gadget.offset, gadget.bit_index);
        
        SDL_Color bgColor = selected ? Colors::SelectedLight() : Colors::Panel();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {15, y - 5, 930, 55};
        SDL_RenderFillRect(renderer, &itemRect);
        
        if (selected) {
            SDL_Color indicatorColor = Colors::Accent();
            SDL_SetRenderDrawColor(renderer, indicatorColor.r, indicatorColor.g, indicatorColor.b, indicatorColor.a);
            SDL_Rect indicator = {15, y - 5, 5, 55};
            SDL_RenderFillRect(renderer, &indicator);
        }
        
        SDL_Color borderColor = selected ? Colors::Accent() : Colors::Border();
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &itemRect);
        
        DrawCheckbox(30, y + 15, owned, selected);
        
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        SDL_Surface* nameSurface = TTF_RenderUTF8_Blended(font, gadget.name.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            SDL_Rect nameRect = {75, y + 5, nameSurface->w, nameSurface->h};
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameSurface);
        }
        
        SDL_Color dimColor = Colors::TextDim();
        SDL_Surface* descSurface = TTF_RenderUTF8_Blended(font, gadget.description.c_str(), dimColor);
        if (descSurface) {
            SDL_Texture* descTexture = SDL_CreateTextureFromSurface(renderer, descSurface);
            SDL_Rect descRect = {75, y + 30, descSurface->w, descSurface->h};
            SDL_RenderCopy(renderer, descTexture, nullptr, &descRect);
            SDL_DestroyTexture(descTexture);
            SDL_FreeSurface(descSurface);
        }
        
        y += 60;
    }
}

void SaveEditor::RenderUnlockablesTab() {
    int y = 140;
    int visibleCount = 0;
    const int VISIBLE_ITEMS = 6;
    
    for (int i = scrollOffset; i < (int)currentGameData.unlockables.size() && visibleCount < VISIBLE_ITEMS; i++, visibleCount++) {
        const GameUnlockable& unlockable = currentGameData.unlockables[i];
        bool selected = (i == selectedIndex);
        bool owned = saveFile->ReadBool(unlockable.offset, unlockable.bit_index);
        
        SDL_Color bgColor = selected ? Colors::SelectedLight() : Colors::Panel();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect itemRect = {15, y - 5, 930, 55};
        SDL_RenderFillRect(renderer, &itemRect);
        
        if (selected) {
            SDL_Color indicatorColor = Colors::Accent();
            SDL_SetRenderDrawColor(renderer, indicatorColor.r, indicatorColor.g, indicatorColor.b, indicatorColor.a);
            SDL_Rect indicator = {15, y - 5, 5, 55};
            SDL_RenderFillRect(renderer, &indicator);
        }
        
        SDL_Color borderColor = selected ? Colors::Accent() : Colors::Border();
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &itemRect);
        
        DrawCheckbox(30, y + 15, owned, selected);
        
        SDL_Color textColor = selected ? Colors::Accent() : Colors::Text();
        SDL_Surface* nameSurface = TTF_RenderUTF8_Blended(font, unlockable.name.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            SDL_Rect nameRect = {75, y + 5, nameSurface->w, nameSurface->h};
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameSurface);
        }
        
        SDL_Color dimColor = Colors::TextDim();
        SDL_Surface* descSurface = TTF_RenderUTF8_Blended(font, unlockable.description.c_str(), dimColor);
        if (descSurface) {
            SDL_Texture* descTexture = SDL_CreateTextureFromSurface(renderer, descSurface);
            SDL_Rect descRect = {75, y + 30, descSurface->w, descSurface->h};
            SDL_RenderCopy(renderer, descTexture, nullptr, &descRect);
            SDL_DestroyTexture(descTexture);
            SDL_FreeSurface(descSurface);
        }
        
        y += 60;
    }
}

void SaveEditor::RenderEditingOverlay() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, 960, 544};
    SDL_RenderFillRect(renderer, &overlay);
    
    SDL_Color panelColor = Colors::Panel();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, 255);
    SDL_Rect panel = {100, 120, 760, 300};
    SDL_RenderFillRect(renderer, &panel);
    
    SDL_Color borderColor = Colors::Accent();
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, 255);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {100 - i, 120 - i, 760 + i*2, 300 + i*2};
        SDL_RenderDrawRect(renderer, &border);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    SDL_Surface* titleSurf = TTF_RenderUTF8_Blended(font, "EDIT VALUE", Colors::Accent());
    if (titleSurf) {
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = {480 - titleSurf->w/2, 145, titleSurf->w, titleSurf->h};
        SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
        SDL_DestroyTexture(titleTex);
        SDL_FreeSurface(titleSurf);
    }
    
    char valueBuf[64];
    snprintf(valueBuf, sizeof(valueBuf), "%d", editingValue);
    
    SDL_Surface* valueSurf = TTF_RenderUTF8_Blended(font, valueBuf, Colors::AccentHover());
    if (valueSurf) {
        SDL_Texture* valueTex = SDL_CreateTextureFromSurface(renderer, valueSurf);
        SDL_Rect valueRect = {480 - valueSurf->w/2, 200, valueSurf->w, valueSurf->h};
        SDL_RenderCopy(renderer, valueTex, nullptr, &valueRect);
        SDL_DestroyTexture(valueTex);
        SDL_FreeSurface(valueSurf);
    }
    
    char multBuf[64];
    snprintf(multBuf, sizeof(multBuf), "Step: +/-%d", editingMultiplier);
    
    SDL_Surface* multSurf = TTF_RenderUTF8_Blended(font, multBuf, Colors::TextDim());
    if (multSurf) {
        SDL_Texture* multTex = SDL_CreateTextureFromSurface(renderer, multSurf);
        SDL_Rect multRect = {480 - multSurf->w/2, 245, multSurf->w, multSurf->h};
        SDL_RenderCopy(renderer, multTex, nullptr, &multRect);
        SDL_DestroyTexture(multTex);
        SDL_FreeSurface(multSurf);
    }
    
    char rangeBuf[64];
    snprintf(rangeBuf, sizeof(rangeBuf), "Range: %d - %d", editingMinValue, editingMaxValue);
    
    SDL_Surface* rangeSurf = TTF_RenderUTF8_Blended(font, rangeBuf, Colors::TextDim());
    if (rangeSurf) {
        SDL_Texture* rangeTex = SDL_CreateTextureFromSurface(renderer, rangeSurf);
        SDL_Rect rangeRect = {480 - rangeSurf->w/2, 280, rangeSurf->w, rangeSurf->h};
        SDL_RenderCopy(renderer, rangeTex, nullptr, &rangeRect);
        SDL_DestroyTexture(rangeTex);
        SDL_FreeSurface(rangeSurf);
    }
    
    const char* controls1 = "UP/DOWN: Adjust | LEFT: Min | RIGHT: Max";
    SDL_Surface* ctrl1Surf = TTF_RenderUTF8_Blended(font, controls1, Colors::Text());
    if (ctrl1Surf) {
        SDL_Texture* ctrl1Tex = SDL_CreateTextureFromSurface(renderer, ctrl1Surf);
        SDL_Rect ctrl1Rect = {480 - ctrl1Surf->w/2, 335, ctrl1Surf->w, ctrl1Surf->h};
        SDL_RenderCopy(renderer, ctrl1Tex, nullptr, &ctrl1Rect);
        SDL_DestroyTexture(ctrl1Tex);
        SDL_FreeSurface(ctrl1Surf);
    }
    
    const char* controls2 = "L/R: Change Step | X: Save | O: Cancel";
    SDL_Surface* ctrl2Surf = TTF_RenderUTF8_Blended(font, controls2, Colors::Text());
    if (ctrl2Surf) {
        SDL_Texture* ctrl2Tex = SDL_CreateTextureFromSurface(renderer, ctrl2Surf);
        SDL_Rect ctrl2Rect = {480 - ctrl2Surf->w/2, 365, ctrl2Surf->w, ctrl2Surf->h};
        SDL_RenderCopy(renderer, ctrl2Tex, nullptr, &ctrl2Rect);
        SDL_DestroyTexture(ctrl2Tex);
        SDL_FreeSurface(ctrl2Surf);
    }
}

void SaveEditor::RenderFooter() {
    SDL_Color panelColor = Colors::PanelDark();
    SDL_SetRenderDrawColor(renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
    SDL_Rect footerRect = {0, 500, 960, 44};
    SDL_RenderFillRect(renderer, &footerRect);
    
    const char* controls = "X: Edit/Toggle | O: Back | START: Save | L/R: Switch Tabs";
    
    SDL_Color dimColor = Colors::TextDim();
    SDL_Surface* controlsSurface = TTF_RenderUTF8_Blended(font, controls, dimColor);
    if (controlsSurface) {
        SDL_Texture* controlsTexture = SDL_CreateTextureFromSurface(renderer, controlsSurface);
        SDL_Rect controlsRect = {480 - controlsSurface->w/2, 510, controlsSurface->w, controlsSurface->h};
        SDL_RenderCopy(renderer, controlsTexture, nullptr, &controlsRect);
        SDL_DestroyTexture(controlsTexture);
        SDL_FreeSurface(controlsSurface);
    }
}

void SaveEditor::DrawCheckbox(int x, int y, bool checked, bool hovered) {
    SDL_Color borderColor = hovered ? Colors::Accent() : Colors::Border();
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_Rect boxRect = {x, y, 30, 30};
    SDL_RenderDrawRect(renderer, &boxRect);
    
    if (checked) {
        SDL_Color fillColor = Colors::Success();
        SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
        SDL_Rect fillRect = {x + 4, y + 4, 22, 22};
        SDL_RenderFillRect(renderer, &fillRect);
    }
}