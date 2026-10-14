#include "MonitorMenu.hpp"
#include "commonMemory.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "mab_types.hpp"

MonitorMenu::MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

/*

Monitor Menu Draw Functions

*/

void MonitorMenu::drawMonitorMenu()
{
    if (ImGui::BeginChild("Left Column Monitor",
                          ImVec2(actuatorsMenuBarWidth, 0),
                          ImGuiChildFlags_None,
                          m_data->flagsBackMenu))
    {
        refreshMDStatus();
        drawActuatorMenu();
        drawCanBusInfoMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("Middle Column Monitor",
                          ImVec2(plotsMenuWidth, 0),
                          ImGuiChildFlags_None,
                          m_data->flagsBackMenu))
    {
        refreshMDStatus();
        drawPlotsMenu();
        drawLiveValuesMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    drawPlaceHolderMenu();
}

void MonitorMenu::drawActuatorMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));
    if (ImGui::BeginChild("Actuators Menu",
                          ImVec2(actuatorsMenuBarWidth, -canBusInfoBarHeight),
                          ImGuiChildFlags_Borders,
                          m_data->flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::Text("Actuators");

        ImGui::PushFont(nullptr, 18.0f);
        float statusX = ImGui::GetFontSize() * 8.0f;

        if (!MonitorMenuMDStatus.empty())
        {
            for (size_t i = 0; i < MonitorMenuMDStatus.size(); i++)
            {
                const MDStatus_S& status = MonitorMenuMDStatus[i];

                if (std::isnan(status.driverTemp))
                    ImGui::Text("ID %u   --", (unsigned)status.id);
                else
                    ImGui::Text("ID %u   %.1f C", (unsigned)status.id, status.driverTemp);

                ImGui::SameLine(statusX);

                if (!status.valid)
                    ImGui::TextDisabled("--");
                else if (status.ok)
                    ImGui::TextColored(ImVec4(0.30f, 0.80f, 0.40f, 1.0f), "OK");
                else
                    ImGui::TextColored(ImVec4(0.90f, 0.30f, 0.30f, 1.0f), "ERR");
            }
        }
        ImGui::PopFont();

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawCanBusInfoMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));
    if (ImGui::BeginChild(
            "Can Bus Info Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, m_data->flagsBackMenu))
    {
        ImGui::Text("CAN Bus Info");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawPlotsMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding, m_data->insideWindowPadding));

    if (ImGui::BeginChild("##Plots Menu",
                          ImVec2(plotsMenuWidth, -liveValuesMenuHeight),
                          ImGuiChildFlags_None,
                          m_data->flagsBackMenu))
    {
        const ImVec2 avail   = ImGui::GetContentRegionAvail();
        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        const ImVec2 cellSize((avail.x - spacing.x) * 0.5f, (avail.y - spacing.y) * 0.5f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                            ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));

        if (ImGui::BeginChild("##Plot1", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::Text("Plot 1");
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot2", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::Text("Plot 2");
        }
        ImGui::EndChild();

        if (ImGui::BeginChild("##Plot3", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::Text("Plot 3");
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot4", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::Text("Plot 4");
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawLiveValuesMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));

    if (ImGui::BeginChild(
            "##Live Values Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, m_data->flagsBackMenu))
    {
        ImGui::Text("Live Values");

        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        ImVec2 tableSize = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);

        if (ImGui::BeginTable("##LiveValuesTable", 7, flags, tableSize))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Pos rad");
            ImGui::TableSetupColumn("Vel rad/status");
            ImGui::TableSetupColumn("Torque Nm");
            ImGui::TableSetupColumn("T motor C");
            ImGui::TableSetupColumn("T MOSFET C");
            ImGui::TableSetupColumn("Status");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < MonitorMenuMDStatus.size(); i++)
            {
                const MDStatus_S& status = MonitorMenuMDStatus[i];

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%u", (unsigned)status.id);

                ImGui::TableNextColumn();
                cellFloat(status.position, "%.3f");
                ImGui::TableNextColumn();
                cellFloat(status.velocity, "%.2f");
                ImGui::TableNextColumn();
                cellFloat(status.torque, "%.2f");
                ImGui::TableNextColumn();
                cellFloat(status.motorTemp, "%.1f");
                ImGui::TableNextColumn();
                cellFloat(status.driverTemp, "%.1f");

                ImGui::TableNextColumn();
                if (!status.valid)
                    ImGui::TextDisabled("--");
                else if (status.ok)
                    ImGui::TextColored(greenColor, "OK");
                else
                    ImGui::TextColored(redColor, "ERR");
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawPlaceHolderMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));
    if (ImGui::BeginChild(
            "##Placeholder Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, m_data->flagsBackMenu))
    {
        ImGui::Text("Placeholders");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

/*

 Draw Plots

*/

// void MonitorMenu::drawVelocityPlot(ImVec2 size, ImPlotFlags plotFlag)
// {
//     int processedSamples = static_cast<int>(m_data->readData);
//     int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
//     int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

//     ImPlotSpec spec;
//     spec.Offset = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);

//     bool testStarted = m_data->testStarted;

//     ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

//     if (ImPlot::BeginPlot("##VelocityPlot", size, plotFlag))

//     {
//         ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
//         ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

//         float currentTime =
//             m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
//                                                    : m_data->offset - 1];

//         ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
//         ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

//         timeElapsedOnPlot = currentTime;

//         float bottomY =
//             (m_data->minVel < 0.0f) ? (m_data->minVel - 0.2f * std::abs(m_data->minVel)) : 0.0f;
//         float topY = (m_data->maxVel > 0.0f) ? (m_data->maxVel + 0.2f * m_data->maxVel) : 0.0f;

//         if (m_data->targetVelocity > topY)
//             topY = m_data->targetVelocity + 0.2f * m_data->targetVelocity;
//         if (m_data->targetVelocity < bottomY)
//             bottomY = m_data->targetVelocity - 0.2f * std::abs(m_data->targetVelocity);

//         if (topY == 0.0f && bottomY == 0.0f)
//         {
//             topY    = marginPlot;
//             bottomY = -marginPlot;
//         }

//         ImPlot::SetupAxis(ImAxis_Y1, "Velocity [rad/s]", ImPlotAxisFlags_NoHighlight);
//         ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

//         ImPlot::EndPlot();
//     }
//     ImPlot::PopStyleColor();
// }

/*

 Help functions

*/

void MonitorMenu::refreshMDStatus()
{
    std::lock_guard<std::mutex> lock(m_data->mtx);
    MonitorMenuMDStatus = m_data->mdStatus;
}

void MonitorMenu::cellFloat(float v, const char* fmt)
{
    if (std::isnan(v))
        ImGui::TextDisabled("--");
    else
        ImGui::Text(fmt, v);
}