// keyboard.h
#pragma once
#include <string>
#include <psp2/ime_dialog.h>

class VitaKeyboard {
public:
    VitaKeyboard();
    ~VitaKeyboard();
    
    bool ShowNumberInput(const std::string& title, const std::string& initialText, int& result, int minVal, int maxVal);
    bool IsActive() const { return active; }
    void Update();
    
private:
    bool active;
    bool hasResult;
    int resultValue;
    int minValue;
    int maxValue;
    
    static void InitParams(SceImeDialogParam& param, const std::string& title, const std::string& initialText);
};