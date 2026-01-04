// keyboard.cpp - NON-BLOCKING keyboard implementation
#include "keyboard.h"
#include <psp2/kernel/threadmgr.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Static buffers for IME dialog
static uint16_t g_ime_title[128];
static uint16_t g_ime_initial[128];
static uint16_t g_ime_input_text[128];

VitaKeyboard::VitaKeyboard() : active(false), hasResult(false), resultValue(0), minValue(0), maxValue(0) {
}

VitaKeyboard::~VitaKeyboard() {
    if (active) {
        sceImeDialogTerm();
        active = false;
    }
}

void VitaKeyboard::InitParams(SceImeDialogParam& param, const std::string& title, const std::string& initialText) {
    sceImeDialogParamInit(&param);
    
    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH;
    param.languagesForced = SCE_TRUE;
    param.type = SCE_IME_TYPE_NUMBER;
    param.option = 0;
    param.maxTextLength = 10;
    
    // Convert title to UTF-16
    for (size_t i = 0; i < title.length() && i < 127; i++) {
        g_ime_title[i] = (uint16_t)title[i];
    }
    g_ime_title[std::min(title.length(), size_t(127))] = 0;
    
    // Convert initial text to UTF-16
    for (size_t i = 0; i < initialText.length() && i < 127; i++) {
        g_ime_initial[i] = (uint16_t)initialText[i];
        g_ime_input_text[i] = (uint16_t)initialText[i];
    }
    g_ime_initial[std::min(initialText.length(), size_t(127))] = 0;
    g_ime_input_text[std::min(initialText.length(), size_t(127))] = 0;
    
    param.title = g_ime_title;
    param.initialText = g_ime_initial;
    param.inputTextBuffer = g_ime_input_text;
}

bool VitaKeyboard::ShowNumberInput(const std::string& title, const std::string& initialText, int& result, int minVal, int maxVal) {
    if (active) {
        printf("Keyboard already active!\n");
        return false;
    }
    
    printf("Showing keyboard for: %s\n", title.c_str());
    
    SceImeDialogParam param;
    InitParams(param, title, initialText);
    
    minValue = minVal;
    maxValue = maxVal;
    hasResult = false;
    
    int ret = sceImeDialogInit(&param);
    if (ret < 0) {
        printf("Failed to init IME dialog: 0x%08X\n", ret);
        return false;
    }
    
    printf("IME dialog initialized successfully\n");
    active = true;
    
    // DON'T BLOCK! Just set active and return
    // The Update() method will be called each frame to check status
    
    // Wait for dialog to finish (but don't block forever)
    int timeout = 0;
    const int MAX_TIMEOUT = 300; // 30 seconds at 10 updates per second
    
    while (active && timeout < MAX_TIMEOUT) {
        Update();
        
        if (!active) {
            // Dialog finished
            break;
        }
        
        sceKernelDelayThread(100000); // 0.1 second
        timeout++;
    }
    
    if (timeout >= MAX_TIMEOUT) {
        printf("Keyboard timeout!\n");
        sceImeDialogTerm();
        active = false;
        return false;
    }
    
    if (hasResult) {
        result = resultValue;
        printf("Keyboard result: %d\n", result);
        return true;
    }
    
    printf("Keyboard cancelled\n");
    return false;
}

void VitaKeyboard::Update() {
    if (!active) return;
    
    SceCommonDialogStatus status = sceImeDialogGetStatus();
    
    if (status == SCE_COMMON_DIALOG_STATUS_RUNNING) {
        // Still running, nothing to do
        return;
    }
    
    if (status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
        printf("IME dialog finished\n");
        
        SceImeDialogResult dialogResult;
        memset(&dialogResult, 0, sizeof(dialogResult));
        sceImeDialogGetResult(&dialogResult);
        
        if (dialogResult.button == SCE_IME_DIALOG_BUTTON_CLOSE) {
            printf("User cancelled keyboard\n");
            hasResult = false;
        } else {
            // Get text from input buffer
            char textBuf[128];
            int i = 0;
            
            // Convert UTF-16 result back to ASCII
            while (i < 127 && g_ime_input_text[i]) {
                textBuf[i] = (char)g_ime_input_text[i];
                i++;
            }
            textBuf[i] = '\0';
            
            printf("User entered: %s\n", textBuf);
            
            // Parse number
            int value = atoi(textBuf);
            
            // Clamp to range
            if (value < minValue) value = minValue;
            if (value > maxValue) value = maxValue;
            
            resultValue = value;
            hasResult = true;
            
            printf("Parsed value: %d (clamped to %d-%d)\n", resultValue, minValue, maxValue);
        }
        
        sceImeDialogTerm();
        active = false;
    }
}