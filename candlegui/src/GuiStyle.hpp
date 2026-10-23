#pragma once
#include "imgui.h"
#include <string>

namespace GuiStyle
{
    // Roundings
    inline float roundingFrameButton   = 12.0f;
    inline float roundingFrameCheckbox = 8.0f;

    // Paddings
    inline float insideWindowPadding_x = 5.0f;
    inline float insideWindowPadding_y = 10.0f;
    inline float windowsPadding        = 20.0f;
    inline float leftMenuPadding       = 30.f;

    // Windows dimensions
    inline float leftMenuBarWidth  = 325.0f;
    inline float middleWindowWidth = 1200.0f;

    // Button sizes
    inline float smallButtonHeight  = 20.0f;
    inline float mediumButtonHeight = 30.0f;
    inline float largeButtonHeight  = 40.0f;

    inline float decisionButtonWidth     = 120.0f;
    inline float leftMenuWideButtonWidth = leftMenuBarWidth - (leftMenuPadding * 2.0f);

    // Colors
    inline ImVec4 transparent       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    inline ImVec4 clear_color       = ImVec4(0.055f, 0.059f, 0.067f, 1.00f);
    inline ImVec4 buttonColor       = ImVec4(0.167f, 0.165f, 0.196f, 1.0f);
    inline ImVec4 buttonColorDimmed = ImVec4(0.167f, 0.165f, 0.196f, 0.5f);

    inline const ImVec4 redColor   = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    inline const ImVec4 greenColor = ImVec4(0.30f, 0.80f, 0.40f, 1.0f);

    inline const ImVec4 mabColor        = ImVec4(1.0f, 0.468f, 0.0f, 1.0f);
    inline const ImVec4 mabColorHovered = ImVec4(1.0f, 0.468f, 0.0f, 0.5f);

    inline const ImVec4 colA = mabColorHovered;
    inline const ImVec4 colB = mabColorHovered;

    // Flags
    inline ImGuiWindowFlags flagsBackMenu =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    inline ImGuiTableFlags flagsTable = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX |
                                        ImGuiTableFlags_NoSavedSettings;

    // Style functions
    void comboStyle(const char* text = nullptr, float framePaddingY = roundingFrameButton);
    void buttonStyle();
    void buttonSelectStyle(bool flag);
    void buttonImportantStyle(bool& flag);
    void buttonInvisibleStyle();
    void checkboxStyle();
    void popupStyle();
    void tableStyle();
    void childInnerStyle();
    void childOuterStyle();

    void endComboStyle();
    void endButtonStyle();
    void endButtonSelectStyle();
    void endButtonImportantStyle();
    void endButtonInvisibleStyle();
    void endCheckboxStyle();
    void endPopupStyle();
    void endTableStyle();
    void endChildInnerStyle();
    void endChildOuterStyle();

    // Helper functions
    void centerText(const char* text);
    bool drawBigInputFloat(const char* label,
                           float*      v,
                           float       step,
                           float       step_fast,
                           const char* format,
                           float       windowWidth,
                           const char* unit = nullptr);
    bool buttonColorInputFloat(const char* label,
                               float*      v,
                               float       step      = 0.0f,
                               float       step_fast = 0.0f,
                               const char* format    = "%.3f");
}  // namespace GuiStyle