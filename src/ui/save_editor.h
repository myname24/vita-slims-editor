// save_editor.h - SIMPLIFIED
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../core/save_file.h"
#include "../data/rac_vita_games_data.h"
#include "../utils/input.h"

enum class EditorTab {
    VALUES,
    WEAPONS,
    GADGETS,
    UNLOCKABLES
};

class SaveEditor {
public:
    SaveEditor(SDL_Renderer* renderer, TTF_Font* font);
    ~SaveEditor();
    
    void SetSaveFile(SaveFile* save);
    void Update(const InputState& input);
    void Render();
    
    bool WantsToGoBack() const { return wantsBack; }
    void ResetBackFlag() { wantsBack = false; }
    
    GameType GetCurrentGameType() const { return currentGameType; }
    std::string GetGameName() const { return currentGameData.name; }
    
private:
    void UpdateEditingMode(const InputState& input);
    void HandleCrossPress();
    void EditValue();
    void EditWeaponAmmo();
    void SaveEditedValue();
    void ToggleGadget();
    void ToggleUnlockable();
    
    void RenderHeader();
    void RenderTabs();
    void RenderTabContent();
    void RenderValuesTab();
    void RenderWeaponsTab();
    void RenderGadgetsTab();
    void RenderUnlockablesTab();
    void RenderEditingOverlay();
    void RenderFooter();
    
    void DrawCheckbox(int x, int y, bool checked, bool hovered);
    
    SDL_Renderer* renderer;
    TTF_Font* font;
    SaveFile* saveFile;
    
    GameType currentGameType;
    GameData currentGameData;
    
    EditorTab currentTab;
    int selectedIndex;
    int scrollOffset;
    bool wantsBack;
    
    // D-pad editor state
    bool isEditing;
    int editingValue;
    int editingMinValue;
    int editingMaxValue;
    int editingMultiplier;
    uint32_t editingOffset;
};