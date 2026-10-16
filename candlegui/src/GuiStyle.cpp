#include "GuiStyle.hpp"
#include "imgui.h"

/*

Style

*/

void GuiStyle::comboStyle(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, mabColorHovered);
    ImGui::PushStyleColor(ImGuiCol_Header, mabColorHovered);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));

    ImVec2 currentPadding = ImGui::GetStyle().FramePadding;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(currentPadding.x, roundingFrameButton));

    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("%s", text);

    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::SetNextItemWidth(leftMenuBarWidth - 2 * leftMenuPadding);
}

void GuiStyle::endComboStyle()
{
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(7);
}

void GuiStyle::buttonStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
}

void GuiStyle::buttonSelectStyle(bool flag)
{
    ImVec4 colorNormal, colorHovered, colorActive;

    if (flag)
    {
        colorNormal  = buttonColor;
        colorHovered = mabColorHovered;
        colorActive  = buttonColor;
    }
    else
    {
        colorNormal  = clear_color;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushStyleColor(ImGuiCol_Button, colorNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);
}

void GuiStyle::endButtonStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void GuiStyle::endButtonSelectStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void GuiStyle::buttonImportantStyle(bool& flag)
{
    ImVec4 colorNormal, colorHovered, colorActive;
    float  borderSize;

    if (flag)
    {
        borderSize   = 0.0f;
        colorNormal  = mabColor;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }
    else
    {
        borderSize   = 1.0f;
        colorNormal  = buttonColor;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushStyleColor(ImGuiCol_Border, mabColor);
    ImGui::PushStyleColor(ImGuiCol_Button, colorNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);
}

void GuiStyle::endButtonImportantStyle()
{
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void GuiStyle::buttonInvisibleStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button, transparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
}

void GuiStyle::endButtonInvisibleStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

void GuiStyle::checkboxStyle()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark, mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColorHovered);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameCheckbox);
}

void GuiStyle::endCheckboxStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

void GuiStyle::centerText(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth   = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}

bool GuiStyle::drawBigInputFloat(const char* label,
                                 float*      v,
                                 float       step,
                                 float       step_fast,
                                 const char* format,
                                 float       windowWidth,
                                 const char* unit)
{
    bool valueChanged = false;

    std::string formatStr = format;
    if (unit != nullptr && unit[0] != '\0')
    {
        formatStr += " ";
        formatStr += unit;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, mabColor);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, mabColor);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(roundingFrameButton, roundingFrameButton));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushID(label);
    float frameHeight = ImGui::GetFrameHeight();
    float buttonSize  = frameHeight;
    float spacing     = ImGui::GetStyle().ItemInnerSpacing.x;

    float interactableWidth = ImGui::CalcItemWidth();
    float inputWidth        = interactableWidth - (buttonSize + spacing) * 2.0f;

    float labelWidth = 0.0f;

    float totalWidgetWidth = interactableWidth + labelWidth;

    if (windowWidth > totalWidgetWidth)
    {
        float offsetX = (windowWidth - totalWidgetWidth) / 2.0f;
        ImGui::SetCursorPosX(offsetX);
    }

    ImGui::PushButtonRepeat(true);

    if (ImGui::Button("-", ImVec2(buttonSize, 0)))
    {
        *v -= ImGui::GetIO().KeyCtrl ? step_fast : step;
        valueChanged = true;
    }
    ImGui::SameLine(0, spacing);

    ImGui::SetNextItemWidth(inputWidth);
    if (ImGui::InputFloat("##input", v, 0.0f, 0.0f, formatStr.c_str(), ImGuiInputTextFlags_None))
    {
        valueChanged = true;
    }
    ImGui::SameLine(0, spacing);

    if (ImGui::Button("+", ImVec2(buttonSize, 0)))
    {
        *v += ImGui::GetIO().KeyCtrl ? step_fast : step;
        valueChanged = true;
    }

    ImGui::PopButtonRepeat();

    ImGui::PopID();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(6);

    ImGui::SetWindowFontScale(1.0f);

    return valueChanged;
}

bool GuiStyle::buttonColorInputFloat(const char* label,
                                     float*      v,
                                     float       step      = 0.0f,
                                     float       step_fast = 0.0f,
                                     const char* format    = "%.3f")
{
    ImVec4 bgColor = ImVec4(0.167f, 0.165f, 0.196f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bgColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bgColor);
    bool valueChanged = ImGui::InputFloat(label, v, step, step_fast, format);
    ImGui::PopStyleColor(3);
    return valueChanged;
}