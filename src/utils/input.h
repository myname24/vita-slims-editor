// ============================================================
// src/utils/input.h - FIXED: extern instead of defining in header
// ============================================================
#pragma once
#include <psp2/ctrl.h>
#include <psp2/touch.h>

struct InputState {
    SceCtrlData pad;
    uint32_t pressed;
    uint32_t held;
    SceTouchData touch;
    bool touchPressed;
    bool touchReleased;
    int touchX;
    int touchY;
    
    // DECLARE static members (don't define them here!)
    static uint32_t oldButtons;
    static bool oldTouchPressed;
    
    void Update() {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        pressed = pad.buttons & ~oldButtons;
        held = pad.buttons;
        oldButtons = pad.buttons;
        
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
        bool currentTouchPressed = (touch.reportNum > 0);
        
        touchPressed = currentTouchPressed && !oldTouchPressed;
        touchReleased = !currentTouchPressed && oldTouchPressed;
        
        if (currentTouchPressed) {
            touchX = touch.report[0].x * 960 / 1920;
            touchY = touch.report[0].y * 544 / 1088;
        }
        
        oldTouchPressed = currentTouchPressed;
    }
    
    bool IsPressed(uint32_t button) const { return pressed & button; }
    bool IsHeld(uint32_t button) const { return held & button; }
    bool IsTouchInRect(int x, int y, int w, int h) const {
        return touchPressed && touchX >= x && touchX < x + w && 
               touchY >= y && touchY < y + h;
    }
};

// NOTE: These are DECLARED here but DEFINED in a .cpp file
// (we'll define them in app.cpp since it's included everywhere)