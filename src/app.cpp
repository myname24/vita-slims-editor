// app.cpp - WITH static member definitions
#include "app.h"
#include "utils/colors.h"
#include <psp2/touch.h>
#include <psp2/io/stat.h>

// DEFINE the static members here (only once in the entire program)
uint32_t InputState::oldButtons = 0;
bool InputState::oldTouchPressed = false;

App::App() 
    : window(nullptr), renderer(nullptr), font(nullptr),
      state(AppState::FILE_BROWSER), fileBrowser(nullptr), 
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
    sceIoMkdir("ux0:/data/slimseditor/saves", 0777);
    
    // Initialize UI components
    fileBrowser = new FileBrowser(renderer, font);
    saveEditor = new SaveEditor(renderer, font);
    
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
    if (saveEditor) delete saveEditor;
    if (fileBrowser) delete fileBrowser;
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
    if (input.IsPressed(SCE_CTRL_START) && state == AppState::FILE_BROWSER) {
        running = false;
    }
}

void App::Update() {
    if (state == AppState::FILE_BROWSER) {
        fileBrowser->Update(input);
        
        // Check if user selected a file
        if (input.IsPressed(SCE_CTRL_CROSS) && fileBrowser->HasSelection()) {
            std::string path = fileBrowser->GetSelectedPath();
            if (saveFile.Load(path)) {
                saveEditor->SetSaveFile(&saveFile);
                state = AppState::SAVE_EDITOR;
            }
        }
    } else if (state == AppState::SAVE_EDITOR) {
        saveEditor->Update(input);
        
        if (saveEditor->WantsToGoBack()) {
            saveEditor->ResetBackFlag();
            state = AppState::FILE_BROWSER;
        }
    }
}

void App::Render() {
    // Clear screen
    SDL_Color bg = Colors::Background();
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);
    
    // Render current state
    if (state == AppState::FILE_BROWSER) {
        fileBrowser->Render();
    } else if (state == AppState::SAVE_EDITOR) {
        saveEditor->Render();
    }
    
    // Present
    SDL_RenderPresent(renderer);
}