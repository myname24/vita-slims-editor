// app.h
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "ui/file_browser.h"
#include "ui/save_editor.h"
#include "core/save_file.h"
#include "utils/input.h"

enum class AppState {
    FILE_BROWSER,
    SAVE_EDITOR
};

class App {
public:
    App();
    ~App();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
private:
    void HandleEvents();
    void Update();
    void Render();
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    AppState state;
    InputState input;
    
    FileBrowser* fileBrowser;
    SaveEditor* saveEditor;
    SaveFile saveFile;
    
    bool running;
};
