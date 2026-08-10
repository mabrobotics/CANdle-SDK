#include "GUI.hpp"

GraphicInterface::GraphicInterface(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

void GraphicInterface::init()
{
    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + generally GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + generally GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    m_window = glfwCreateWindow(1280, 960, "MD GUI", nullptr, nullptr);
    if (m_window == nullptr)
        return;

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void GraphicInterface::close()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    if (m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void GraphicInterface::loop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (m_data->candleAvailable)
        {
            // drawMenuTopBar();
            drawMenuLowerBar();
            drawTestMenuBar();
            drawLeftMenuBar();
            // drawRightMenuBar();
            drawErrorMenuBar();
            drawMainMenu();
        }
        else
        {
            drawErrorMenuPopup();

            ImGui::BeginDisabled();

            // drawMenuTopBar();
            drawMenuLowerBar();
            drawTestMenuBar();
            drawLeftMenuBar();
            // drawRightMenuBar();
            drawErrorMenuBar();
            drawMainMenu();

            ImGui::EndDisabled();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

/*

Main menu draw functions

*/

// void GraphicInterface::drawMenuTopBar()
// {
//     if (ImGui::BeginMainMenuBar())
//     {
//         if (ImGui::BeginMenu("File"))
//         {
//             if (ImGui::MenuItem("Undo", "Ctrl+Z"))
//             {
//             }
//             ImGui::EndMenu();
//         }
//         if (ImGui::BeginMenu("Edit"))
//         {
//             if (ImGui::MenuItem("Undo", "Ctrl+Z"))
//             {
//             }
//             if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
//             {
//             }  // Disabled item
//             ImGui::Separator();
//             if (ImGui::MenuItem("Cut", "Ctrl+X"))
//             {
//             }
//             if (ImGui::MenuItem("Copy", "Ctrl+C"))
//             {
//             }
//             if (ImGui::MenuItem("Paste", "Ctrl+V"))
//             {
//             }
//             ImGui::EndMenu();
//         }
//         ImGui::EndMainMenuBar();
//     }
// }

void GraphicInterface::drawMenuLowerBar()
{
    const ImGuiViewport* lowbarViewport = ImGui::GetMainViewport();

    ImVec2 lowbarPos =
        ImVec2(lowbarViewport->WorkPos.x,
               lowbarViewport->WorkPos.y + lowbarViewport->WorkSize.y - lowBarHeight);
    ImGui::SetNextWindowPos(lowbarPos, ImGuiCond_Always);

    ImVec2 mainSize = ImVec2(lowbarViewport->WorkSize.x, lowBarHeight);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Lower Bar", nullptr, flagsBackMenu))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / m_io.Framerate,
                    m_io.Framerate);
    }
    ImGui::End();
}

void GraphicInterface::drawTestMenuBar()
{
    const ImGuiViewport* testMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(testMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 testPos = ImVec2(testMenuViewport->WorkPos.x,
                            testMenuViewport->WorkPos.y + testMenuViewport->WorkSize.y -
                                testMenuBarHeight - lowBarHeight);
    ImGui::SetNextWindowPos(testPos, ImGuiCond_Always);

    ImVec2 windowSize = ImVec2(leftMenuBarWidth, testMenuBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Test menu", nullptr, flagsBackMenu))
    {
        drawTestManualButton();
        drawTestEndButton();
    }
    ImGui::End();
}

void GraphicInterface::drawErrorMenuBar()
{
    const ImGuiViewport* errorBarViewport = ImGui::GetMainViewport();

    ImVec2 errorBarPos =
        ImVec2(errorBarViewport->WorkPos.x + leftMenuBarWidth, errorBarViewport->WorkPos.y);
    ImGui::SetNextWindowPos(errorBarPos, ImGuiCond_Always);

    ImVec2 mainSize =
        ImVec2(errorBarViewport->WorkSize.x - leftMenuBarWidth /*- rightMenuBarWidth*/,
               errorMenuBarHeight);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Error Menu Bar", nullptr, flagsBackMenu))
    {
        bool errorOccured = m_data->errorOccured;
        ImGui::Text("Error status:");
        ImGui::SameLine();
        if (!errorOccured)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("No errors");
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void GraphicInterface::drawLeftMenuBar()
{
    const ImGuiViewport* leftMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(leftMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 windowSize =
        ImVec2(leftMenuBarWidth, leftMenuViewport->WorkSize.y - testMenuBarHeight - lowBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Main Menu", nullptr, flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
        {
            ImGui::BeginDisabled();
        }
        drawDiscoverMDButton();
        drawSelectMDButton();
        drawSelectModeButton();

        switch (m_data->currentMode)
        {
            case mab::MdMode_E::IDLE:
                break;
            case mab::MdMode_E::VELOCITY_PID:
                drawParametersVelocity();
                drawSetTargetVelocity();
                drawSetVelocityWindow();
                break;
            case mab::MdMode_E::POSITION_PID:
                drawParametersPosition();
                drawSetTargetPosition();
                drawSetPositionWindow();
                break;
            case mab::MdMode_E::IMPEDANCE:
                drawParametersImpedance();
                drawSetTargetPosition();
                break;
            case mab::MdMode_E::RAW_TORQUE:  // case unused
                drawSetTargetTorque();
                break;
            case mab::MdMode_E::VELOCITY_PROFILE:
                drawParametersVelocity();
                drawParametersPosition();
                drawSetTargetVelocity();
                drawSetTargetPosition();
                drawSetTargetAcceleration();
                drawSetTargetDeceleration();
                break;
            case mab::MdMode_E::POSITION_PROFILE:
                drawParametersVelocity();
                drawParametersPosition();
                drawSetTargetVelocity();
                drawSetTargetPosition();
                drawSetTargetAcceleration();
                drawSetTargetDeceleration();
                break;
            default:
                break;
        }
        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::End();
}

// void GraphicInterface::drawRightMenuBar()
// {
//     const ImGuiViewport* rightMenuViewport = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(rightMenuViewport->WorkPos, ImGuiCond_Always);

//     ImVec2 rightPos =
//         ImVec2(rightMenuViewport->WorkSize.x - rightMenuBarWidth, rightMenuViewport->WorkPos.y);
//     ImGui::SetNextWindowPos(rightPos, ImGuiCond_Always);

//     ImVec2 windowSize = ImVec2(rightMenuBarWidth, rightMenuViewport->WorkSize.y - lowBarHeight);
//     ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

//     if (ImGui::Begin("Right Menu", nullptr, flagsBackMenu))
//     {
//     }
//     ImGui::End();
// }

void GraphicInterface::drawMainMenu()
{
    const ImGuiViewport* mainMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(mainMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 mainPos = ImVec2(mainMenuViewport->WorkPos.x + leftMenuBarWidth,
                            mainMenuViewport->WorkPos.y + errorMenuBarHeight);
    ImGui::SetNextWindowPos(mainPos, ImGuiCond_Always);

    ImVec2 mainSize =
        ImVec2(mainMenuViewport->WorkSize.x - leftMenuBarWidth /*- rightMenuBarWidth*/,
               mainMenuViewport->WorkSize.y - lowBarHeight - errorMenuBarHeight);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Main menu", nullptr, flagsBackMenu))
    {
        mab::MdMode_E currentModeLocal = m_data->currentMode;
        updatePlotData();

        if (currentModeLocal == mab::MdMode_E::VELOCITY_PID ||
            currentModeLocal == mab::MdMode_E::VELOCITY_PROFILE)
        {
            drawVelocityPlot();
            drawPositionPlot();
            ImGui::SameLine();
            drawTorquePlot();
        }
        else if (currentModeLocal == mab::MdMode_E::POSITION_PID ||
                 currentModeLocal == mab::MdMode_E::POSITION_PROFILE ||
                 currentModeLocal == mab::MdMode_E::IMPEDANCE)
        {
            drawPositionPlot();
            drawVelocityPlot();
            ImGui::SameLine();
            drawTorquePlot();
        }
        else
        {
            drawVelocityPlot();
            drawPositionPlot();
            drawTorquePlot();
        }
    }

    ImGui::End();
}

void GraphicInterface::drawErrorMenuPopup()
{
    const char* popupTitle = "CANdle Error##ErrorPopup";

    ImGui::OpenPopup(popupTitle);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, mabColor);

    if (ImGui::BeginPopupModal(popupTitle, nullptr, flags))
    {
        ImGui::SetWindowFontScale(1.3f);

        if (!m_data->updatedVersion)
            ImGui::Text("Update CANdle drivers.");
        else
            ImGui::Text("You didn't light your CANdle!");
        ImGui::Separator();

        if (!m_data->updatedVersion)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                               "Install new USB drivers by runing candlesdk-win-driver.exe!");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                               "Continue by connecting CANdle via USB!");

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(1);
}

/*

BUTTONS

*/

void GraphicInterface::drawTestManualButton()
{
    ImVec4        colorNormal, colorHovered, colorActive;
    float         borderSize;
    bool          selectedMode               = m_data->selectedMode;
    mab::MdMode_E currentMode                = m_data->currentMode;
    bool          buttonAutomaticTestPressed = m_data->buttonAutomaticTestPressed;
    bool          buttonManualTestPressed    = m_data->buttonManualTestPressed;

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed)
    {
        ImGui::BeginDisabled();
    }

    if (buttonManualTestPressed)
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

    ImGui::SetCursorPosX(paddingButtons);

    ImGui::PushButtonRepeat(true);

    if (ImGui::Button("Manual Test", ImVec2(leftMenuBarWidth - (paddingButtons * 2.0f), 40.0f)))
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->buttonManualTestPressed = true;
    }

    bool is_held = ImGui::IsItemActive();
    if (!buttonAutomaticTestPressed)
    {
        if (is_held)
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->testStarted                = true;
            m_data->buttonAutomaticTestPressed = false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->testStarted             = false;
            m_data->buttonManualTestPressed = false;
        }
    }

    ImGui::PopButtonRepeat();

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed)
    {
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawTestEndButton()
{
    ImVec4        colorNormal, colorHovered, colorActive;
    float         borderSize;
    bool          testStarted             = m_data->testStarted;
    bool          selectedMode            = m_data->selectedMode;
    bool          buttonManualTestPressed = m_data->buttonManualTestPressed;
    mab::MdMode_E currentMode             = m_data->currentMode;

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonManualTestPressed)
    {
        ImGui::BeginDisabled();
    }

    if (testStarted && !buttonManualTestPressed)
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

    ImGui::SetCursorPosX(paddingButtons);
    if (ImGui::Button(m_data->buttonAutomaticTestPressed ? "End test" : "Automatic Test",
                      ImVec2(leftMenuBarWidth - (paddingButtons * 2.0f), 40.0f)))
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->testStarted                = !m_data->testStarted;
        m_data->buttonManualTestPressed    = false;
        m_data->buttonAutomaticTestPressed = true;
    }

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonManualTestPressed)
    {
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawDiscoverMDButton()
{
    ImGui::Spacing();
    std::string chosenIDname = "";

    if (m_data->discoverOngoing)
    {
        ImGui::BeginDisabled();
        double  time       = ImGui::GetTime();
        uint8_t dotCounter = (uint8_t)(time * 2.5) % 4;

        chosenIDname = "Discover ongoing" + std::string("...").substr(0, dotCounter);
    }
    else
    {
        chosenIDname = "Discover MD";
    }

    buttonStyle();
    ImGui::SetCursorPosX(paddingButtons);
    if (ImGui::Button(chosenIDname.c_str(),
                      ImVec2(leftMenuBarWidth - (paddingButtons * 2.0f), mediumButtonHeight)))
    {
        m_data->buttonDiscoverMdPressed = true;
    }
    endButtonStyle();

    if (m_data->discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

void GraphicInterface::drawSelectMDButton()
{
    ImGui::Spacing();

    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID     = 0;
    std::string               chosenIDname = "";

    bool selectedMD = m_data->selectedMD;

    mdIDs    = m_data->mdIDs;
    chosenID = m_data->chosenID;

    if (mdIDs.empty())
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->selectedMD   = false;
        m_data->selectedMode = false;
    }

    if (mdIDs.empty())
    {
        chosenIDstr = "No MDs available.";
        ImGui::BeginDisabled();
    }
    else if (!selectedMD)
        chosenIDstr = "Select Your MD";

    std::string comboText = "MD Select";
    comboStyle(comboText.c_str());
    if (ImGui::BeginCombo("##MD Select", chosenIDstr.c_str()))
    {
        if (ImGui::Selectable("None"))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->selectedMD = false;
        }
        for (const mab::canId_t& id : mdIDs)
        {
            chosenIDname = "MD" + std::to_string(uint16_t((id)));

            bool selectedID = (chosenID == id);

            if (ImGui::Selectable(chosenIDname.c_str()))
            {
                std::lock_guard<std::mutex> lock(m_data->mtx);
                m_data->chosenID              = id;
                m_data->buttonSelectMdPressed = true;
                m_data->selectedMD            = true;
                chosenIDstr                   = chosenIDname;
                m_data->discoverOngoing       = false;
            }
            if (selectedID)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    endComboStyle();

    if (m_data->discoverOngoing && mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
    else if (mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
}

void GraphicInterface::drawSelectModeButton()
{
    ImGui::Spacing();

    bool selectedMD = m_data->selectedMD;

    if (!selectedMD)
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->currentMode = mab::MdMode_E::IDLE;
        ImGui::BeginDisabled();
    }

    std::string comboText = "MD Motion Mode";
    comboStyle(comboText.c_str());
    if (ImGui::BeginCombo("##MD Motion Mode", getModeName(m_data->currentMode)))
    {
        if (ImGui::Selectable("None", m_data->currentMode == mab::MdMode_E::IDLE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::IDLE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Velocity PID", m_data->currentMode == mab::MdMode_E::VELOCITY_PID))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::VELOCITY_PID;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Position PID", m_data->currentMode == mab::MdMode_E::POSITION_PID))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::POSITION_PID;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Impedance PD", m_data->currentMode == mab::MdMode_E::IMPEDANCE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::IMPEDANCE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Velocity Profile",
                              m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::VELOCITY_PROFILE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Position Profile",
                              m_data->currentMode == mab::MdMode_E::POSITION_PROFILE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::POSITION_PROFILE;
            m_data->selectedMode = true;
        }

        ImGui::EndCombo();
    }
    endComboStyle();

    if (!selectedMD)
    {
        ImGui::EndDisabled();
    }
}

/*

Parameters setters etc.

*/

void GraphicInterface::drawParametersVelocity()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Velocity loop - PID tuning parameters");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTableVelocity", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kp_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Ki_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Ki_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->integralMax_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->integralMax_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Velocity Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        m_data->Kp_velSlider          = 0.0f;
        m_data->Ki_velSlider          = 0.0f;
        m_data->Kd_velSlider          = 0.0f;
        m_data->integralMax_velSlider = 0.0f;
    }
    endButtonStyle();

    ImGui::Spacing();
}

void GraphicInterface::drawParametersPosition()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kp_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Ki_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Ki_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->integralMax_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->integralMax_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Position Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        m_data->Kp_posSlider          = 0.0f;
        m_data->Ki_posSlider          = 0.0f;
        m_data->Kd_posSlider          = 0.0f;
        m_data->integralMax_posSlider = 0.0f;
    }
    endButtonStyle();
    ImGui::Spacing();
}

void GraphicInterface::drawParametersImpedance()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Impedance PD tuner");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kp_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Impedance Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        m_data->Kp_impSlider = 0.0f;
        m_data->Kd_impSlider = 0.0f;
    }
    endButtonStyle();
    ImGui::Spacing();
}

void GraphicInterface::drawSetTargetVelocity()
{
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Velocity");
    if (drawBigInputFloat("##Target Velocity",
                          &m_data->targetVelocitySlider,
                          1.0f,
                          2.0f,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s"))
    {
        m_data->targetVelocitySlider = std::clamp(
            m_data->targetVelocitySlider, -m_data->maxVelocityClamp, m_data->maxVelocityClamp);
    }
}

void GraphicInterface::drawSetTargetPosition()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Position");
    if (drawBigInputFloat("##Target Position",
                          &m_data->targetPositionSlider,
                          1.0f,
                          2.0f,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad"))
    {
        m_data->targetPositionSlider = std::clamp(
            m_data->targetPositionSlider, m_data->minPositionClamp, m_data->maxPositionClamp);
    }
}

void GraphicInterface::drawSetTargetTorque()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Torque");
    if (drawBigInputFloat("##Target Torque",
                          &m_data->targetTorqueSlider,
                          1.0f,
                          2.0f,
                          "%.2f",
                          leftMenuBarWidth,
                          "Nm"))
    {
        m_data->targetTorqueSlider =
            std::clamp(m_data->targetTorqueSlider, -m_data->maxTorqueClamp, m_data->maxTorqueClamp);
    }
}

void GraphicInterface::drawSetTargetAcceleration()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Acceleration");
    if (drawBigInputFloat("##Acceleration",
                          &m_data->targetAccelerationSlider,
                          step,
                          step_fast,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s^2"))
    {
        m_data->targetAccelerationSlider =
            std::clamp(m_data->targetAccelerationSlider, 0.0f, m_data->maxAccelerationClamp);
    }
}

void GraphicInterface::drawSetTargetDeceleration()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Deceleration");
    if (drawBigInputFloat("##Deceleration",
                          &m_data->targetDecelerationSlider,
                          step,
                          step_fast,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s^2"))
    {
        m_data->targetDecelerationSlider =
            std::clamp(m_data->targetDecelerationSlider, 0.0f, m_data->maxDecelerationClamp);
    }
}

void GraphicInterface::drawSetPositionWindow()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Position Window");
    drawBigInputFloat("##Position Window",
                      &m_data->positionWindowSlider,
                      step,
                      step_fast,
                      "%.2f",
                      leftMenuBarWidth,
                      "rad");
}

void GraphicInterface::drawSetVelocityWindow()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Velocity Window");
    drawBigInputFloat("##Velocity Window",
                      &m_data->velocityWindowSlider,
                      step,
                      step_fast,
                      "%.2f",
                      leftMenuBarWidth,
                      "rad/s");
}

/*

Draw plots

*/

void GraphicInterface::updatePlotData()
{
    static bool  lastTestStarted    = false;
    static float timeInTargetWindow = 0.0f;
    static float lastHardwareTime   = 0.0f;

    bool testStarted = m_data->testStarted;

    if (testStarted && !lastTestStarted)
    {
        m_data->reset();
        m_data->readData       = m_data->plotWriteData.load(std::memory_order_acquire);
        timeInTargetWindow     = 0.0f;
        lastHardwareTime       = 0.0f;
        m_data->guiElapsedTime = 0.0f;
    }

    lastTestStarted = testStarted;

    if (testStarted)
    {
        m_data->guiElapsedTime += m_io.DeltaTime;
        uint32_t currentDataWrite = m_data->plotWriteData.load(std::memory_order_acquire);

        while (m_data->readData != currentDataWrite)
        {
            uint32_t bufferIndex = m_data->readData % commonMemory_S::PLOT_BUFFER_SIZE;

            float time      = m_data->plotTime[bufferIndex];
            float vel       = m_data->plotVelocity[bufferIndex];
            float pos       = m_data->plotPosition[bufferIndex];
            float trq       = m_data->plotTorque[bufferIndex];
            float targetVel = m_data->plotTargetVelocity[bufferIndex];
            float targetPos = m_data->plotTargetPosition[bufferIndex];

            float dt_point   = time - lastHardwareTime;
            lastHardwareTime = time;

            if (vel > m_data->maxVel)
                m_data->maxVel = vel;
            if (vel < m_data->minVel)
                m_data->minVel = vel;
            if (pos > m_data->maxPos)
                m_data->maxPos = pos;
            if (pos < m_data->minPos)
                m_data->minPos = pos;
            if (trq > m_data->maxTrq)
                m_data->maxTrq = trq;
            if (trq < m_data->minTrq)
                m_data->minTrq = trq;

            bool inPosWindow = std::abs(pos - targetPos) <= m_data->positionWindow;
            bool inVelWindow = std::abs(vel - targetVel) <= m_data->velocityWindow;

            switch (m_data->currentMode)
            {
                case mab::MdMode_E::VELOCITY_PID:
                    timeInTarget(inVelWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::VELOCITY_PROFILE:
                    timeInTarget(inVelWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::POSITION_PID:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::IMPEDANCE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::POSITION_PROFILE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::IDLE:
                    break;
                case mab::MdMode_E::RAW_TORQUE:
                    break;
                default:
                    break;
            }

            m_data->readData++;
        }

        m_data->offset = currentDataWrite % commonMemory_S::PLOT_BUFFER_SIZE;
    }
}

void GraphicInterface::timeInTarget(bool& inTimeWindow, float& timeInTargetWindow, float& dt)
{
    if (inTimeWindow)
    {
        timeInTargetWindow += dt;

        if (timeInTargetWindow >= targetHoldTime && !m_data->buttonManualTestPressed)
        {
            {
                std::lock_guard<std::mutex> lock(m_data->mtx);
                m_data->testStarted = false;
                m_data->testOngoing = false;
            }
        }
    }
    else
    {
        timeInTargetWindow = 0.0f;
    }
}

void GraphicInterface::drawVelocityPlot()
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);

    bool testStarted = m_data->testStarted;

    ImVec2 windowSize;

    mab::MdMode_E currentModeLocal = m_data->currentMode;

    if (currentModeLocal == mab::MdMode_E::VELOCITY_PID ||
        currentModeLocal == mab::MdMode_E::VELOCITY_PROFILE)
    {
        windowSize = ImVec2(-1, ImGui::GetContentRegionAvail().y / 2);
    }
    else if (currentModeLocal == mab::MdMode_E::POSITION_PID ||
             currentModeLocal == mab::MdMode_E::POSITION_PROFILE ||
             currentModeLocal == mab::MdMode_E::IMPEDANCE)
    {
        windowSize = ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y);
    }
    else
    {
        windowSize = ImVec2(-1, ImGui::GetContentRegionAvail().y / 3);
    }

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##VelocityPlot", windowSize))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        float bottomY =
            (m_data->minVel < 0.0f) ? (m_data->minVel - 0.2f * std::abs(m_data->minVel)) : 0.0f;
        float topY = (m_data->maxVel > 0.0f) ? (m_data->maxVel + 0.2f * m_data->maxVel) : 0.0f;

        if (m_data->targetVelocity > topY)
            topY = m_data->targetVelocity + 0.2f * m_data->targetVelocity;
        if (m_data->targetVelocity < bottomY)
            bottomY = m_data->targetVelocity - 0.2f * std::abs(m_data->targetVelocity);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Velocity [rad/s]");
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Velocity(t)", m_data->plotTime, m_data->plotVelocity, pointsCount, spec);

        if (m_data->currentMode == mab::MdMode_E::VELOCITY_PID ||
            m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE ||
            m_data->currentMode == mab::MdMode_E::POSITION_PROFILE)
        {
            ImPlot::PlotStairs(
                "Target Velocity", m_data->plotTime, m_data->plotTargetVelocity, pointsCount, spec);
        }

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

void GraphicInterface::drawPositionPlot()
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);

    bool   testStarted = m_data->testStarted;
    ImVec2 windowSize;

    mab::MdMode_E currentModeLocal = m_data->currentMode;

    if (currentModeLocal == mab::MdMode_E::VELOCITY_PID ||
        currentModeLocal == mab::MdMode_E::VELOCITY_PROFILE)
    {
        windowSize = ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y);
    }
    else if (currentModeLocal == mab::MdMode_E::POSITION_PID ||
             currentModeLocal == mab::MdMode_E::POSITION_PROFILE ||
             currentModeLocal == mab::MdMode_E::IMPEDANCE)
    {
        windowSize = ImVec2(-1, ImGui::GetContentRegionAvail().y / 2);
    }
    else
    {
        windowSize = ImVec2(-1, ImGui::GetContentRegionAvail().y / 2);
    }

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##PositionPlot", windowSize))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        float bottomY =
            (m_data->minPos < 0.0f) ? (m_data->minPos - 0.2f * std::abs(m_data->minPos)) : 0.0f;
        float topY = (m_data->maxPos > 0.0f) ? (m_data->maxPos + 0.2f * m_data->maxPos) : 0.0f;

        if (m_data->targetPosition > topY)
            topY = m_data->targetPosition + 0.2f * m_data->targetPosition;
        if (m_data->targetPosition < bottomY)
            bottomY = m_data->targetPosition - 0.2f * std::abs(m_data->targetPosition);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Position [rad]");
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Position(t)", m_data->plotTime, m_data->plotPosition, pointsCount, spec);
        if (m_data->currentMode == mab::MdMode_E::POSITION_PID ||
            m_data->currentMode == mab::MdMode_E::POSITION_PROFILE ||
            m_data->currentMode == mab::MdMode_E::IMPEDANCE ||
            m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE)
            ImPlot::PlotStairs(
                "Target Position", m_data->plotTime, m_data->plotTargetPosition, pointsCount, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

void GraphicInterface::drawTorquePlot()
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset      = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);
    bool testStarted = m_data->testStarted;

    ImVec2 windowSize = ImVec2(-1, -1);

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##TorquegPlot", windowSize))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        float bottomY =
            (m_data->minTrq < 0.0f) ? (m_data->minTrq - 0.2f * std::abs(m_data->minTrq)) : 0.0f;
        float topY = (m_data->maxTrq > 0.0f) ? (m_data->maxTrq + 0.2f * m_data->maxTrq) : 0.0f;

        if (m_data->targetTorque > topY)
            topY = m_data->targetTorque + 0.2f * m_data->targetTorque;
        if (m_data->targetTorque < bottomY)
            bottomY = m_data->targetTorque - 0.2f * std::abs(m_data->targetTorque);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Torque [Nm]");
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Torque(t)", m_data->plotTime, m_data->plotTorque, pointsCount, spec);

        if (m_data->currentMode == mab::MdMode_E::IMPEDANCE)
            ImPlot::PlotStairs(
                "Target Torque", m_data->plotTime, m_data->plotTargetTorque, pointsCount, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

/*

Style

*/

void GraphicInterface::comboStyle(const char* text)
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

    ImVec2 currentPadding = ImGui::GetStyle().FramePadding;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(currentPadding.x, roundingFrameButton));

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("%s", text);

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::SetNextItemWidth(leftMenuBarWidth - 2 * paddingButtons);
}

void GraphicInterface::endComboStyle()
{
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(7);
}

void GraphicInterface::buttonStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
}

void GraphicInterface::endButtonStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

void GraphicInterface::centerText(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth   = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}

const char* GraphicInterface::getModeName(mab::MdMode_E mode)
{
    switch (mode)
    {
        case mab::MdMode_E::IDLE:
            return "None";
        case mab::MdMode_E::VELOCITY_PID:
            return "Velocity PID";
        case mab::MdMode_E::POSITION_PID:
            return "Position PID";
        case mab::MdMode_E::IMPEDANCE:
            return "Impedance PD";
        case mab::MdMode_E::RAW_TORQUE:  // case unused
            return "Raw Torque";
        case mab::MdMode_E::VELOCITY_PROFILE:
            return "Velocity Profile";
        case mab::MdMode_E::POSITION_PROFILE:
            return "Position Profile";
        default:
            return "Unknown mode";
    }
}

bool GraphicInterface::drawBigInputFloat(const char* label,
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

bool GraphicInterface::buttonColorInputFloat(const char* label,
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