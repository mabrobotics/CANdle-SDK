#include "TestScript.hpp"
#include <cstddef>
#include <string>
#include "GuiStyle.hpp"
#include "imgui.h"
#include "mab_types.hpp"

TestScript::TestScript(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

/*

Main Window display

*/

void TestScript::drawTestScriptMenu()
{
    drawSelectBlocksMenu();
    ImGui::SameLine();

    drawMainScriptMenu();
    ImGui::SameLine();

    if (selected != -1)
        drawCodeDetailsMenu();
}

/*

Test Script Draw Functions

*/

void TestScript::drawSelectBlocksMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("##Code Blocks Selection",
                          ImVec2(GuiStyle::leftMenuBarWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;
        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, GuiStyle::headerFontSize);
        ImGui::Text("Code Blocks Selection");
        ImGui::PopFont();

        for (int group = 0; group < CodeGroupType_E::GROUP_COUNTER; group++)
        {
            ImGui::Text("%s", groupNames[group]);

            for (int blockType = 0; blockType < CodeBlockType_E::COUNT; blockType++)
            {
                if (blockGroup[blockType] != group)
                    continue;

                chooseButtonStyle(blockGroup[blockType]);
                ImGui::Button(blockNames[blockType], ImVec2(-1, GuiStyle::mediumButtonHeight));
                GuiStyle::endButtonStyle();

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("NEW_BLOCK", &blockType, sizeof(int));
                    ImGui::Text("%s", blockNames[blockType]);
                    ImGui::EndDragDropSource();
                }
            }
        }

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void TestScript::drawMainScriptMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("##Main Script Menu",
                          ImVec2(GuiStyle::middleWindowWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing    = m_data->testOngoing;
        bool clickedOnBlock = false;
        int  moveFrom = -1, moveTo = -1, insertType = -1, insertAt = -1;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, GuiStyle::headerFontSize);
        ImGui::Text("Create Your Instruction");
        ImGui::PopFont();

        if (ImGui::BeginChild("##ScriptGrid", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None))
        {
            ImVec2 windowPos  = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            GuiStyle::scriptSpaceStyle(
                windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                ImGui::GetScrollY());

            ImVec2 gridPos             = ImGui::GetCursorScreenPos();
            float  nextBlockPosition_y = gridPos.y + GuiStyle::insideWindowPadding_y;

            for (int blockNumber = 0; blockNumber < (int)script.size(); blockNumber++)
            {
                ImGui::SetCursorScreenPos(
                    ImVec2(gridPos.x + GuiStyle::insideWindowPadding_y, nextBlockPosition_y));
                ImGui::PushID(blockNumber);

                CodeBlock_S& block = script[blockNumber];

                ImVec2 blockPos = ImGui::GetCursorScreenPos();

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->ChannelsSplit(2);
                dl->ChannelsSetCurrent(1);

                ImVec2 dragSize = ImVec2(block.guiSize.x + 2 * GuiStyle::insideWindowPadding_x,
                                         block.guiSize.y + 2 * GuiStyle::insideWindowPadding_x);
                if (dragSize.x < 1.0f)
                    dragSize.x = 1.0f;
                if (dragSize.y < 1.0f)
                    dragSize.y = 1.0f;

                ImGui::SetCursorScreenPos(ImVec2(blockPos.x - GuiStyle::insideWindowPadding_x,
                                                 blockPos.y - GuiStyle::insideWindowPadding_x));
                ImGui::SetNextItemAllowOverlap();
                ImGui::InvisibleButton("##blockDrag", dragSize);

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("MOVE_BLOCK", &blockNumber, sizeof(int));
                    ImGui::Text("%s", blockNames[block.type]);
                    ImGui::EndDragDropSource();
                }

                ImGui::SetCursorScreenPos(blockPos);
                ImGui::BeginGroup();
                switch (block.type)
                {
                    case CodeBlockType_E::MOVE_TO_POSITION:
                        drawMoveFunction(block);
                        break;
                    case CodeBlockType_E::ADD_MDS:
                        drawAddMDsFunction(block);
                        break;
                    case CodeBlockType_E::SET_MODE:
                        drawSetModeFunction(block);
                        break;
                    case CodeBlockType_E::SET_GAINS:
                        drawSetGainsFunction(block);
                        break;
                    default:
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("%s", blockNames[block.type]);
                        break;
                }
                ImGui::EndGroup();

                ImVec2 minimumPos = ImGui::GetItemRectMin();
                ImVec2 maximumPos = ImGui::GetItemRectMax();
                block.guiSize     = ImGui::GetItemRectSize();

                nextBlockPosition_y = maximumPos.y + 2 * GuiStyle::insideWindowPadding_x + spacing;

                bool hovered =
                    ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
                    ImGui::IsMouseHoveringRect(
                        ImVec2(minimumPos.x - GuiStyle::insideWindowPadding_x,
                               minimumPos.y - GuiStyle::insideWindowPadding_x),
                        ImVec2(maximumPos.x + GuiStyle::insideWindowPadding_x,
                               maximumPos.y + GuiStyle::insideWindowPadding_x));
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    selected       = blockNumber;
                    clickedOnBlock = true;
                }

                dl->ChannelsSetCurrent(0);
                drawBlockBackground(
                    dl, minimumPos, maximumPos, blockGroup[block.type], selected == blockNumber);
                dl->ChannelsMerge();

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_BLOCK"))
                    {
                        moveFrom = *(const int*)payload->Data;
                        moveTo   = blockNumber;
                    }
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NEW_BLOCK"))
                    {
                        insertType = *(const int*)payload->Data;
                        insertAt   = blockNumber;
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }

            ImGui::SetCursorScreenPos(ImVec2(gridPos.x, nextBlockPosition_y));
            ImGui::Dummy(ImVec2(1.0f, GuiStyle::insideWindowPadding_y));

            if (!clickedOnBlock && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
                selected = -1;
        }
        ImGui::EndChild();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NEW_BLOCK"))
            {
                insertType = *(const int*)payload->Data;
                insertAt   = (int)script.size();
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_BLOCK"))
            {
                moveFrom = *(const int*)payload->Data;
                moveTo   = (int)script.size() - 1;
            }
            ImGui::EndDragDropTarget();
        }

        if (moveFrom >= 0)
        {
            CodeBlock_S block = script[moveFrom];
            script.erase(script.begin() + moveFrom);
            script.insert(script.begin() + moveTo, block);
        }
        if (insertType >= 0)
            script.insert(script.begin() + insertAt, CodeBlock_S{(CodeBlockType_E)insertType});

        if (moveFrom >= 0 && selected == moveFrom)
            selected = moveTo;
        if (insertType >= 0)
            selected = insertAt;

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void TestScript::drawCodeDetailsMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild(
            "##CodeBlockDetail", ImVec2(-1, -1), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, GuiStyle::headerFontSize);
        ImGui::Text("Code Block Details");
        ImGui::PopFont();

        if (selected < (int)script.size())
        {
            CodeBlock_S& block = script[selected];

            switch (block.type)
            {
                case CodeBlockType_E::MOVE_TO_POSITION:
                    drawMoveDetails(block);
                    break;
                default:
                    ImGui::Text("%s", blockNames[block.type]);
                    break;
            }
        }

        if (testOngoing)
            ImGui::EndDisabled();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

/*

Block Draw Functions

*/

void TestScript::drawMoveFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Move MD to position");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##pos", &block.movePosition, 0.0f, 0.0f, "%.2f");
}

void TestScript::drawAddMDsFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Use MDs");

    int removeAt = -1;

    for (int id = 0; id < (int)block.mdIds.size(); id++)
    {
        ImGui::PushID(id);

        ImGui::SameLine();
        drawMDSelectCombo(block.mdIds[id]);

        ImGui::SameLine();
        GuiStyle::buttonStyle();
        if (ImGui::SmallButton("x"))
            removeAt = id;
        GuiStyle::endButtonStyle();

        ImGui::PopID();
    }

    if (removeAt >= 0)
        block.mdIds.erase(block.mdIds.begin() + removeAt);

    ImGui::SameLine();
    GuiStyle::buttonStyle();
    if (ImGui::Button("+ add"))
        block.mdIds.push_back(0);
    GuiStyle::endButtonStyle();

    ImGui::SameLine();
    ImGui::Text("for the test");
}

void TestScript::drawSetModeFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Set MD mode to");

    ImGui::SameLine();
    GuiStyle::comboStyle(nullptr, blockComboHeight);
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo("##Select MD mode", getModeName(block.setMode).c_str()))
    {
        for (mab::MdMode_E mode : modes)
        {
            if (ImGui::Selectable(getModeName(mode).c_str(), block.setMode == mode))
                block.setMode = mode;
        }

        ImGui::EndCombo();
    }
    GuiStyle::endComboStyle();
}

void TestScript::drawSetGainsFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Set kp");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##kp_block", &block.setKp, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("ki");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##ki_block", &block.setKi, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("kd");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##kd_block", &block.setKd, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("gains");
}

/*

 Block Details

*/

void TestScript::drawMoveDetails(CodeBlock_S& block)
{
    ImGui::Text("%s", std::to_string(block.movePosition).c_str());
}

/*

Helper Functions

*/

void TestScript::chooseButtonStyle(CodeGroupType_E group)
{
    switch (group)
    {
        case CodeGroupType_E::GROUP_MOTION:
            GuiStyle::buttonMotionStyle();
            break;
        case CodeGroupType_E::GROUP_CONTROL:
            GuiStyle::buttonControlStyle();
            break;
        case CodeGroupType_E::GROUP_SETTERS:
            GuiStyle::buttonSetterStyle();
            break;
        default:
            break;
    }
}

ImVec4 TestScript::getGroupColor(CodeGroupType_E& group)
{
    switch (group)
    {
        case CodeGroupType_E::GROUP_MOTION:
            return GuiStyle::buttonMotionColor;
            break;
        case CodeGroupType_E::GROUP_CONTROL:
            return GuiStyle::buttonControlColor;
            break;
        case CodeGroupType_E::GROUP_SETTERS:
            return GuiStyle::buttonSetterColor;
            break;
        default:
            return GuiStyle::transparent;
            break;
    }
}

std::string TestScript::getModeName(mab::MdMode_E& modeName)
{
    std::string modeNameStr = "None";
    switch (modeName)
    {
        case mab::MdMode_E::VELOCITY_PID:
            modeNameStr = "Velocity PID";
            break;
        case mab::MdMode_E::POSITION_PID:
            modeNameStr = "Position PID";
            break;
        case mab::MdMode_E::VELOCITY_PROFILE:
            modeNameStr = "Velocity PROFILE";
            break;
        case mab::MdMode_E::POSITION_PROFILE:
            modeNameStr = "Position PROFILE";
            break;
        case mab::MdMode_E::IMPEDANCE:
            modeNameStr = "Impedance";
            break;
        case mab::MdMode_E::RAW_TORQUE:
            modeNameStr = "Raw Torque";
            break;
        default:
            break;
    }
    return modeNameStr;
}

void TestScript::drawBlockBackground(
    ImDrawList* dl, ImVec2 minimumPos, ImVec2 maximumPos, CodeGroupType_E group, bool isSelected)
{
    minimumPos = ImVec2(minimumPos.x - GuiStyle::insideWindowPadding_x,
                        minimumPos.y - GuiStyle::insideWindowPadding_x);
    maximumPos = ImVec2(maximumPos.x + GuiStyle::insideWindowPadding_x,
                        maximumPos.y + GuiStyle::insideWindowPadding_x);

    ImU32 col      = ImGui::ColorConvertFloat4ToU32(getGroupColor(group));
    float rounding = 0.0f;

    switch (group)
    {
        case CodeGroupType_E::GROUP_MOTION:
            rounding = 6.0f;
            dl->AddRectFilled(minimumPos, maximumPos, col, rounding);
            break;

        case CodeGroupType_E::GROUP_CONTROL:
            rounding = 4.0f;
            dl->AddRectFilled(minimumPos, maximumPos, IM_COL32(40, 40, 45, 255), rounding);
            dl->AddRectFilled(minimumPos,
                              ImVec2(minimumPos.x + 6.0f, maximumPos.y),
                              col,
                              rounding,
                              ImDrawFlags_RoundCornersLeft);
            break;

        case CodeGroupType_E::GROUP_SETTERS:
            rounding = (maximumPos.y - minimumPos.y) * 0.5f;
            dl->AddRectFilled(minimumPos, maximumPos, col, rounding);
            break;

        default:
            dl->AddRectFilled(minimumPos, maximumPos, col);
            break;
    }

    if (isSelected)
        dl->AddRect(minimumPos, maximumPos, IM_COL32(255, 255, 255, 255), rounding, 0, 2.0f);
}

void TestScript::drawMDSelectCombo(mab::canId_t& id)
{
    GuiStyle::comboStyle(nullptr, blockComboHeight);
    ImGui::SetNextItemWidth(70);
    if (ImGui::BeginCombo("##Select MD or Group", std::to_string(id).c_str()))
    {
        for (MDStatus_S md : m_data->mdStatus)  // TODO risky
        {
            if (ImGui::Selectable(std::to_string(md.id).c_str(), id == md.id))
                id = md.id;
        }

        // TODO add groups list
        ImGui::EndCombo();
    }
    GuiStyle::endComboStyle();
}
