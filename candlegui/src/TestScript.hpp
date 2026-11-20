#pragma once
#include <vector>
#include "GuiStyle.hpp"
#include "candle_types.hpp"
#include "commonMemory.hpp"
#include "imgui.h"
#include "mab_types.hpp"

class TestScript
{
  public:
    TestScript(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawTestScriptMenu();

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    // Blocks
    enum CodeGroupType_E
    {
        GROUP_MOTION,   // 0
        GROUP_CONTROL,  // 1
        GROUP_SETTERS,  // 2
        GROUP_COUNTER
    };
    const char* groupNames[CodeGroupType_E::GROUP_COUNTER] = {"Motion",    // 1
                                                              "Control",   // 2
                                                              "Setters"};  // 3

    enum CodeBlockType_E
    {
        MOVE_TO_POSITION,  // 0
        MOVE_VELOCITY,     // 1
        MOVE_TORQUE,       // 2
        REPEAT_N_TIMES,    // 3
        REPEAT_END,        // 4
        WAIT,              // 5
        ADD_MDS,           // 6
        SET_MODE,          // 7
        SET_VALUE,         // 8
        SET_GAINS,         // 9
        COUNT
    };
    const char* blockNames[CodeBlockType_E::COUNT] = {"Move to position",    // 0
                                                      "Move with velocity",  // 1
                                                      "Move with torque",    // 2
                                                      "Repeat N Times",      // 3
                                                      "End repeat",          // 4
                                                      "Wait",                // 5
                                                      "Add MDs",             // 6
                                                      "Set mode",            // 7
                                                      "Set value",           // 8
                                                      "Set gains"};          // 9

    CodeGroupType_E blockGroup[CodeBlockType_E::COUNT] = {
        GROUP_MOTION,   // MOVE_TO_POSITION
        GROUP_MOTION,   // MOVE_VELOCITY
        GROUP_MOTION,   // MOVE_TORQUE
        GROUP_CONTROL,  // REPEAT_N_TIMES
        GROUP_CONTROL,  // REPEAT_END
        GROUP_CONTROL,  // WAIT
        GROUP_SETTERS,  // ADD_MDS
        GROUP_SETTERS,  // SET_MODE
        GROUP_SETTERS,  // SET_VALUE
        GROUP_SETTERS   // SET_GAINS
    };

    struct CodeBlock_S
    {
        CodeBlockType_E type;

        mab::canId_t              id;
        MDGroup_S                 group;
        std::vector<mab::canId_t> mdIds;

        // SETTER PARAMETERS
        mab::MdMode_E setMode = mab::MdMode_E::IDLE;
        float         setKp   = 0.0f;
        float         setKi   = 0.0f;
        float         setKd   = 0.0f;

        // MOVE PARAMETERS
        float movePosition = 0.0f;
        float moveVelocity = 0.0f;
        float moveTorque   = 0.0f;

        // REPEAT PARAMETERS
        int n = 1;

        ImVec2 guiSize = ImVec2(0, 0);
    };

    struct OpenRepeat
    {
        int    startIndex;
        ImVec2 headerMin;
        ImVec2 headerMax;
    };

    struct BlockLayout
    {
        float top;
        float bottom;
        int   slotDepth;
    };

    static constexpr mab::MdMode_E modes[] = {
        mab::MdMode_E::IDLE,
        mab::MdMode_E::POSITION_PID,
        mab::MdMode_E::VELOCITY_PID,
        mab::MdMode_E::RAW_TORQUE,
        mab::MdMode_E::IMPEDANCE,
        mab::MdMode_E::POSITION_PROFILE,
        mab::MdMode_E::VELOCITY_PROFILE,
    };

    std::vector<CodeBlock_S> script;
    int                      selected = -1;

    const float spacingBetweenBlocks = 8.0f;
    const float blockComboHeight     = 6.0f;
    const float footerHeight         = 8.0f;
    const float indentInRepeatLoop   = 20.0f;
    const float gridMargin_x         = 10.0f;

    // Test Script Draw Functions
    void drawSelectBlocksMenu();
    void drawMainScriptMenu();
    void drawCodeDetailsMenu();

    // Block Draw Functions
    void drawBlockContent(CodeBlock_S& block, float footerWidth);

    void drawMoveToPosFunction(CodeBlock_S& block);
    void drawMoveWithVelFunction(CodeBlock_S& block);
    void drawMoveWithTrqFunction(CodeBlock_S& block);
    void drawRepeatNTimesFunction(CodeBlock_S& block);
    void drawControlFunction();
    void drawSetterFunction();
    void drawAddMDsFunction(CodeBlock_S& block);
    void drawSetModeFunction(CodeBlock_S& block);
    void drawSetGainsFunction(CodeBlock_S& block);

    // Block Details
    void drawMoveDetails(CodeBlock_S& block);

    // Helper Functions
    void        chooseButtonStyle(CodeGroupType_E group);
    ImVec4      getGroupColor(CodeGroupType_E group);
    std::string getModeName(mab::MdMode_E& modeName);
    int         findLoopEnd(int start);
    void        drawBlockBackground(
        ImDrawList* dl, ImVec2 mn, ImVec2 mx, CodeGroupType_E group, bool isSelected);
    void drawRepeatShape(ImDrawList* dl,
                         ImVec2      headerMin,
                         ImVec2      headerMax,
                         ImVec2      footerMin,
                         ImVec2      footerMax,
                         bool        isSelected);

    void drawMDSelectCombo(mab::canId_t& id);
};
