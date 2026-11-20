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

                if (blockType == CodeBlockType_E::REPEAT_END)
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
        int  hoverSlot = -1;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, GuiStyle::headerFontSize);
        ImGui::Text("Create Your Instruction");
        ImGui::PopFont();

        if (ImGui::BeginChild(
                "##ScriptBackground", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None))
        {
            const float pad_x = GuiStyle::insideWindowPadding_x;

            bool gridHovered =
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

            ImVec2 windowPos  = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();

            GuiStyle::scriptSpaceStyle(
                windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y));

            ImVec2 gridPos             = ImGui::GetCursorScreenPos();
            float  nextBlockPosition_y = gridPos.y + GuiStyle::insideWindowPadding_y;

            std::vector<OpenRepeat>  openRepeats;
            std::vector<BlockLayout> blockLayout;

            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->ChannelsSplit(2);
            dl->ChannelsSetCurrent(1);

            for (int blockNumber = 0; blockNumber < (int)script.size(); blockNumber++)
            {
                CodeBlock_S& block = script[blockNumber];

                int depth     = (int)openRepeats.size();
                int slotDepth = depth;
                if (block.type == CodeBlockType_E::REPEAT_END && depth > 0)
                    depth--;

                ImGui::SetCursorScreenPos(
                    ImVec2(gridPos.x + gridMargin_x + pad_x + depth * indentInRepeatLoop,
                           nextBlockPosition_y));
                ImGui::PushID(blockNumber);

                ImVec2 blockPos = ImGui::GetCursorScreenPos();

                ImVec2 dragSize = ImVec2(std::max(1.0f, block.guiSize.x + 2 * pad_x),
                                         std::max(1.0f, block.guiSize.y + 2 * pad_x));

                ImGui::SetCursorScreenPos(ImVec2(blockPos.x - pad_x, blockPos.y - pad_x));
                ImGui::SetNextItemAllowOverlap();
                ImGui::InvisibleButton("##blockDrag", dragSize);

                if (block.type != CodeBlockType_E::REPEAT_END &&
                    ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
                {
                    ImGui::SetDragDropPayload("MOVE_BLOCK", &blockNumber, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                float footerWidth = openRepeats.empty() ? 60.0f
                                                        : openRepeats.back().headerMax.x -
                                                              openRepeats.back().headerMin.x;
                ImGui::SetCursorScreenPos(blockPos);
                ImGui::BeginGroup();
                drawBlockContent(block, footerWidth);
                ImGui::EndGroup();

                ImVec2 minimumPos    = ImGui::GetItemRectMin();
                ImVec2 maximumPos    = ImGui::GetItemRectMax();
                ImVec2 backgroundMin = ImVec2(minimumPos.x - pad_x, minimumPos.y - pad_x);
                ImVec2 backgroundMax = ImVec2(maximumPos.x + pad_x, maximumPos.y + pad_x);
                block.guiSize        = ImGui::GetItemRectSize();

                blockLayout.push_back({backgroundMin.y, backgroundMax.y, slotDepth});

                if (block.type == CodeBlockType_E::REPEAT_N_TIMES)
                    openRepeats.push_back({blockNumber, minimumPos, maximumPos});

                nextBlockPosition_y = maximumPos.y + 2 * pad_x + spacingBetweenBlocks;

                bool hovered =
                    gridHovered && ImGui::IsMouseHoveringRect(backgroundMin, backgroundMax);
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    if (block.type == CodeBlockType_E::REPEAT_END && !openRepeats.empty())
                        selected = openRepeats.back().startIndex;
                    else
                        selected = blockNumber;
                    clickedOnBlock = true;
                }

                dl->ChannelsSetCurrent(0);
                if (block.type == CodeBlockType_E::REPEAT_END && !openRepeats.empty())
                {
                    OpenRepeat& repeatBlockHeader = openRepeats.back();
                    drawRepeatShape(dl,
                                    repeatBlockHeader.headerMin,
                                    repeatBlockHeader.headerMax,
                                    minimumPos,
                                    maximumPos,
                                    selected == repeatBlockHeader.startIndex);
                    openRepeats.pop_back();
                }
                else if (block.type != CodeBlockType_E::REPEAT_N_TIMES)
                {
                    drawBlockBackground(dl,
                                        minimumPos,
                                        maximumPos,
                                        blockGroup[block.type],
                                        selected == blockNumber);
                }
                dl->ChannelsSetCurrent(1);

                ImGui::PopID();
            }

            int endSlotDepth = (int)openRepeats.size();

            dl->ChannelsSetCurrent(0);
            for (OpenRepeat& repeatBlockHeader : openRepeats)
                drawBlockBackground(dl,
                                    repeatBlockHeader.headerMin,
                                    repeatBlockHeader.headerMax,
                                    CodeGroupType_E::GROUP_CONTROL,
                                    selected == repeatBlockHeader.startIndex);
            dl->ChannelsSetCurrent(1);

            dl->ChannelsMerge();

            const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload();
            bool draggingBlock = dragPayload && (dragPayload->IsDataType("NEW_BLOCK") ||
                                                 dragPayload->IsDataType("MOVE_BLOCK"));

            if (draggingBlock && gridHovered)
            {
                float mouseY = ImGui::GetMousePos().y;

                hoverSlot = (int)blockLayout.size();

                for (int blockIndex = 0; blockIndex < (int)blockLayout.size(); blockIndex++)
                {
                    float blockMiddle =
                        (blockLayout[blockIndex].top + blockLayout[blockIndex].bottom) * 0.5f;
                    if (mouseY < blockMiddle)
                    {
                        hoverSlot = blockIndex;
                        break;
                    }
                }

                if (dragPayload->IsDataType("MOVE_BLOCK"))
                {
                    int dragFrom = *(const int*)dragPayload->Data;
                    int dragEnd  = findLoopEnd(dragFrom);
                    if (hoverSlot >= dragFrom && hoverSlot <= dragEnd + 1)
                        hoverSlot = -1;
                }
            }

            if (hoverSlot >= 0)
            {
                float line_y;
                int   blocksNumber = (int)blockLayout.size();

                if (blocksNumber == 0)
                    line_y = gridPos.y + GuiStyle::insideWindowPadding_y;
                else if (hoverSlot == 0)
                    line_y = blockLayout[0].top - spacingBetweenBlocks * 0.5f;
                else if (hoverSlot == blocksNumber)
                    line_y = blockLayout[blocksNumber - 1].bottom + spacingBetweenBlocks * 0.5f;
                else
                    line_y =
                        (blockLayout[hoverSlot - 1].bottom + blockLayout[hoverSlot].top) * 0.5f;

                int lineDepth =
                    (hoverSlot < blocksNumber) ? blockLayout[hoverSlot].slotDepth : endSlotDepth;
                float line_x = gridPos.x + gridMargin_x + lineDepth * indentInRepeatLoop;

                ImU32 lineColor = IM_COL32(255, 255, 255, 255);
                dl->AddLine(
                    ImVec2(line_x, line_y), ImVec2(line_x + 200.0f, line_y), lineColor, 3.0f);
                dl->AddCircleFilled(ImVec2(line_x, line_y), 4.0f, lineColor);
            }

            ImGui::SetCursorScreenPos(ImVec2(gridPos.x, nextBlockPosition_y));
            ImGui::Dummy(ImVec2(1.0f, GuiStyle::insideWindowPadding_y));

            if (!clickedOnBlock && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && gridHovered)
                selected = -1;
        }
        ImGui::EndChild();

        if (ImGui::BeginDragDropTarget())
        {
            ImGuiDragDropFlags flags = ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NEW_BLOCK", flags))
            {
                insertType = *(const int*)payload->Data;
                insertAt   = (hoverSlot >= 0) ? hoverSlot : (int)script.size();
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_BLOCK", flags))
            {
                if (hoverSlot >= 0)
                {
                    moveFrom = *(const int*)payload->Data;
                    moveTo   = hoverSlot;
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (moveFrom >= 0)
        {
            int moveEnd = findLoopEnd(moveFrom);
            int count   = moveEnd - moveFrom + 1;

            bool intoItself = (moveTo >= moveFrom && moveTo <= moveEnd + 1);
            if (!intoItself)
            {
                std::vector<CodeBlock_S> part(script.begin() + moveFrom,
                                              script.begin() + moveEnd + 1);
                script.erase(script.begin() + moveFrom, script.begin() + moveEnd + 1);

                if (moveTo > moveEnd)
                    moveTo -= count;

                script.insert(script.begin() + moveTo, part.begin(), part.end());
                selected = moveTo;
            }
        }

        if (insertType >= 0)
        {
            script.insert(script.begin() + insertAt, CodeBlock_S{(CodeBlockType_E)insertType});
            if (insertType == CodeBlockType_E::REPEAT_N_TIMES)
                script.insert(script.begin() + insertAt + 1,
                              CodeBlock_S{CodeBlockType_E::REPEAT_END});
            selected = insertAt;
        }

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

        if (selected >= 0 && selected < (int)script.size())
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

void TestScript::drawBlockContent(CodeBlock_S& block, float footerWidth)
{
    switch (block.type)
    {
        case CodeBlockType_E::MOVE_TO_POSITION:
            drawMoveToPosFunction(block);
            break;
        case CodeBlockType_E::MOVE_VELOCITY:
            drawMoveWithVelFunction(block);
            break;
        case CodeBlockType_E::MOVE_TORQUE:
            drawMoveWithTrqFunction(block);
            break;
        case CodeBlockType_E::REPEAT_N_TIMES:
            drawRepeatNTimesFunction(block);
            break;
        case CodeBlockType_E::REPEAT_END:
            ImGui::Dummy(ImVec2(footerWidth, footerHeight));
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
}

void TestScript::drawMoveToPosFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Move MD to position");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##pos", &block.movePosition, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("rad");
}

void TestScript::drawMoveWithVelFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Move MD with velocity");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##vel", &block.moveVelocity, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("rad/s");
}

void TestScript::drawMoveWithTrqFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Move MD with torque");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("##trq", &block.moveTorque, 0.0f, 0.0f, "%.2f");

    ImGui::SameLine();
    ImGui::Text("Nm");
}

void TestScript::drawRepeatNTimesFunction(CodeBlock_S& block)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Repeat");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("##n_times", &block.n, 0.0f, 0.0f);
    if (block.n < 1)
        block.n = 1;

    ImGui::SameLine();
    ImGui::Text("times");
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
        if (ImGui::Button("x"))
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
    ImGui::SetNextItemWidth(130);
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

ImVec4 TestScript::getGroupColor(CodeGroupType_E group)
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

int TestScript::findLoopEnd(int start)
{
    if (script[start].type != CodeBlockType_E::REPEAT_N_TIMES)
        return start;

    int openedRepeatLoops = 0;
    for (int blockIndex = start; blockIndex < (int)script.size(); blockIndex++)
    {
        if (script[blockIndex].type == CodeBlockType_E::REPEAT_N_TIMES)
            openedRepeatLoops++;
        else if (script[blockIndex].type == CodeBlockType_E::REPEAT_END)
        {
            openedRepeatLoops--;
            if (openedRepeatLoops == 0)
                return blockIndex;
        }
    }
    return (int)script.size() - 1;
}

void TestScript::drawBlockBackground(
    ImDrawList* dl, ImVec2 minimumPos, ImVec2 maximumPos, CodeGroupType_E group, bool isSelected)
{
    const float pad_x = GuiStyle::insideWindowPadding_x;

    minimumPos = ImVec2(minimumPos.x - pad_x, minimumPos.y - pad_x);
    maximumPos = ImVec2(maximumPos.x + pad_x, maximumPos.y + pad_x);

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

void TestScript::drawRepeatShape(ImDrawList* dl,
                                 ImVec2      headerMin,
                                 ImVec2      headerMax,
                                 ImVec2      footerMin,
                                 ImVec2      footerMax,
                                 bool        isSelected)
{
    const float pad_x    = GuiStyle::insideWindowPadding_x;
    const float barWidth = 12.0f;
    ImU32       col = ImGui::ColorConvertFloat4ToU32(getGroupColor(CodeGroupType_E::GROUP_CONTROL));

    ImVec2 top0 = ImVec2(headerMin.x - pad_x, headerMin.y - pad_x);
    ImVec2 top1 = ImVec2(headerMax.x + pad_x, headerMax.y + pad_x);
    ImVec2 bot0 = ImVec2(footerMin.x - pad_x, footerMin.y - pad_x);
    ImVec2 bot1 = ImVec2(footerMax.x + pad_x, footerMax.y + pad_x);

    dl->AddRectFilled(
        top0, top1, col, 4.0f, ImDrawFlags_RoundCornersTop | ImDrawFlags_RoundCornersBottomRight);
    dl->AddRectFilled(ImVec2(top0.x, top1.y - 1.0f), ImVec2(top0.x + barWidth, bot0.y + 1.0f), col);
    dl->AddRectFilled(
        bot0, bot1, col, 4.0f, ImDrawFlags_RoundCornersBottom | ImDrawFlags_RoundCornersTopRight);

    if (isSelected)
        dl->AddRect(top0, top1, IM_COL32(255, 255, 255, 255), 4.0f, 0, 2.0f);
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
