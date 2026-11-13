// app.cpp - WITH static member definitions
#include "app.h"
#include "utils/colors.h"
#include "core/save_store.h"
#include <string>
#include <psp2/touch.h>
#include <psp2/io/stat.h>

App::App() 
    : window(nullptr), renderer(nullptr), font(nullptr),
      state(AppState::SAVE_BROWSER), saveBrowser(nullptr), 
      saveEditor(nullptr), running(true) {
}

App::~App() {
}

bool App::Initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        return false;
    }
    
    if (TTF_Init() < 0) {
        SDL_Quit();
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow("Slim's Editor",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              960, 544,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // Load font
    font = TTF_OpenFont("sa0:/data/font/pvf/jpn0.pvf", 18);
    if (!font) {
        font = TTF_OpenFont("vs0:/data/external/livearea/theme/font.ttf", 18);
    }
    
    if (!font) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // Enable touch
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    
    // Create directories
    sceIoMkdir("ux0:/data", 0777);
    sceIoMkdir("ux0:/data/slimseditor", 0777);
    
    // Initialize UI components
    saveBrowser = std::make_unique<SaveBrowser>(renderer, font);
    saveEditor = std::make_unique<SaveEditor>(renderer, font);
    
    // Load the save-access modules. Failure is not fatal: the browser shows the error.
    std::string err;
    bool ok = SaveStore::Init(err);
    saveBrowser->SetStoreStatus(ok, err);
    saveBrowser->Rescan();
    
    return true;
}

void App::Run() {
    while (running) {
        HandleEvents();
        Update();
        Render();
        SDL_Delay(16); // ~60 FPS
    }
}

void App::Shutdown() {
    // unique_ptr will clean up automatically; reset explicitly to be clear
    if (saveEditor) saveEditor.reset();
    if (saveBrowser) saveBrowser.reset();
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void App::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
    
    // Update input state
    input.Update();
    
    // Global exit
    if (input.IsPressed(SCE_CTRL_START) && state == AppState::SAVE_BROWSER) {
        running = false;
    }
}

void App::Update() {
    if (state == AppState::SAVE_BROWSER) {
        saveBrowser->Update(input);
        
        // Open the selected save (X, or a tap on the already-selected row)
        bool openRequested = saveBrowser->ConsumeOpenRequest();
        if ((input.IsPressed(SCE_CTRL_CROSS) || openRequested) && saveBrowser->HasSelection()) {
            const SaveSlot& slot = saveBrowser->GetSelectedSlot();
            if (saveFile.Load(slot)) {
                saveEditor->SetSaveFile(&saveFile);
                state = AppState::SAVE_EDITOR;
            } else {
                saveBrowser->SetStatus(saveFile.GetLastError());
            }
        }
    } else if (state == AppState::SAVE_EDITOR) {
        saveEditor->Update(input);
        
        if (saveEditor->WantsToGoBack()) {
            saveEditor->ResetBackFlag();
            state = AppState::SAVE_BROWSER;
            saveBrowser->Rescan();
        }
    }
}

void App::Render() {
    // Clear screen
    SDL_Color bg = Colors::Background();
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);
    
    // Render current state
    if (state == AppState::SAVE_BROWSER) {
        saveBrowser->Render();
    } else if (state == AppState::SAVE_EDITOR) {
        saveEditor->Render();
    }
    
    // Present
    SDL_RenderPresent(renderer);
}