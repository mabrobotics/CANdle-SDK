#include "imgui.h"
#include <string>

class GuiStyle
{
  protected:
    // Roundings
    float roundingFrameButton   = 12.0f;
    float roundingFrameCheckbox = 8.0f;

    // Paddings
    float insideWindowPadding = 10.0f;
    float windowsPadding      = 20.0f;
    float leftMenuPadding     = 30.f;

    // Windows dimensions
    float leftMenuBarWidth  = 325.0f;
    float middleWindowWidth = 1200.0f;

    // Button sizes
    float smallButtonHeight  = 20.0f;
    float mediumButtonHeight = 30.0f;
    float largeButtonHeight  = 40.0f;

    float decisionButtonWidth     = 120.0f;
    float leftMenuWideButtonWidth = leftMenuBarWidth - (leftMenuPadding * 2.0f);

    // Colors
    ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 clear_color = ImVec4(0.055f, 0.059f, 0.067f, 1.00f);
    ImVec4 buttonColor = ImVec4(0.167f, 0.165f, 0.196f, 1.0f);

    const ImVec4 redColor   = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 greenColor = ImVec4(0.30f, 0.80f, 0.40f, 1.0f);

    const ImVec4 mabColor        = ImVec4(1.0f, 0.468f, 0.0f, 1.0f);
    const ImVec4 mabColorHovered = ImVec4(1.0f, 0.468f, 0.0f, 0.5f);

    const ImVec4 colA = mabColorHovered;
    const ImVec4 colB = mabColorHovered;

    // Flags
    ImGuiWindowFlags flagsBackMenu = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Style functions
    void comboStyle(const char* text);
    void buttonStyle();
    void buttonSelectStyle(bool flag);
    void buttonImportantStyle(bool& flag);
    void buttonInvisibleStyle();
    void checkboxStyle();

    void endComboStyle();
    void endButtonStyle();
    void endButtonSelectStyle();
    void endButtonImportantStyle();
    void endButtonInvisibleStyle();
    void endCheckboxStyle();

    void centerText(const char* text);
    bool drawBigInputFloat(const char* label,
                           float*      v,
                           float       step,
                           float       step_fast,
                           const char* format,
                           float       windowWidth,
                           const char* unit = nullptr);
    bool buttonColorInputFloat(
        const char* label, float* v, float step, float step_fast, const char* format);
};