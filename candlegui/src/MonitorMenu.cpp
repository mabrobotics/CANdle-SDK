#include "MonitorMenu.hpp"
#include "BusActuatorsMenu.hpp"
#include "GuiStyle.hpp"
#include "commonMemory.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "mab_types.hpp"
#include <cfloat>
#include <filesystem>
#include "Recorder.hpp"

MonitorMenu::MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
    exportDir = defaultExportDir();
}

/*

Monitor Menu Draw Functions

*/

void MonitorMenu::drawMonitorMenu()
{
    if (ImGui::BeginChild("Left Column Monitor",
                          ImVec2(GuiStyle::leftMenuBarWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        refreshMDStatus();
        drawActuatorMenu();
        drawCanBusInfoMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("Middle Column Monitor",
                          ImVec2(GuiStyle::middleWindowWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        refreshMDStatus();
        drawPlotsMenu();
        drawLiveValuesMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    drawRecordDataMenu();
}

void MonitorMenu::drawActuatorMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("Actuators Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, -canBusInfoBarHeight),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
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
                else if (status.quickStatusIsOK)
                    ImGui::TextColored(GuiStyle::greenColor, "OK");
                else
                    ImGui::TextColored(GuiStyle::redColor, "ERR");
            }
        }
        ImGui::PopFont();

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void MonitorMenu::drawCanBusInfoMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild(
            "Can Bus Info Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::Text("CAN Bus Info");
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void MonitorMenu::drawPlotsMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("##Plots Menu",
                          ImVec2(GuiStyle::middleWindowWidth, -liveValuesMenuHeight),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        const ImVec2 avail   = ImGui::GetContentRegionAvail();
        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        const ImVec2 cellSize((avail.x - spacing.x) * 0.5f, (avail.y - spacing.y) * 0.5f);

        GuiStyle::childOuterStyle();
        if (ImGui::BeginChild("##Plot1", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Velocity");
            ImGui::PopFont();
            drawVelocityPlots();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot2", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Position");
            ImGui::PopFont();
            drawPositionPlots();
        }
        ImGui::EndChild();

        if (ImGui::BeginChild("##Plot3", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Torque");
            ImGui::PopFont();
            drawTorquePlots();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot4", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("MOSFET temperature");
            ImGui::PopFont();
            drawMosfetTempPlots();
        }
        ImGui::EndChild();

        GuiStyle::endChildOuterStyle();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void MonitorMenu::drawLiveValuesMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild(
            "##Live Values Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::Text("Live Values");

        ImVec2 tableSize = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);

        GuiStyle::tableStyle();
        if (ImGui::BeginTable("##LiveValuesTable", 7, GuiStyle::flagsTable, tableSize))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Pos rad");
            ImGui::TableSetupColumn("Vel rad/s");
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
                else if (status.quickStatusIsOK)
                    ImGui::TextColored(GuiStyle::greenColor, "OK");
                else
                    ImGui::TextColored(GuiStyle::redColor, "ERR");
            }
            ImGui::EndTable();
        }
        GuiStyle::endTableStyle();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void MonitorMenu::drawRecordDataMenu()
{
    bool availableToRecord = m_data->mdStatus.empty();

    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("##Record Data Menu",
                          ImVec2(0, 0),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY,
                          GuiStyle::flagsBackMenu))
    {
        ImGui::PushFont(nullptr, 18.0f);
        ImGui::Text("Record Data");
        ImGui::PopFont();

        if (availableToRecord)
            ImGui::BeginDisabled();

        drawRecordingButtons();
        drawExportButtons();

        if (availableToRecord)
            ImGui::EndDisabled();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void MonitorMenu::drawRecordingButtons()
{
    bool recording = m_data->recording.load();

    ImVec2 tableSize = ImVec2(-FLT_MIN, GuiStyle::mediumButtonHeight);

    GuiStyle::buttonImportantStyle(recording);
    if (ImGui::Button(
            recording ? "Stop recording###RecordButton" : "Start recording###RecordButton",
            tableSize))
    {
        m_data->recording.store(!recording);
    }
    GuiStyle::endButtonImportantStyle();

    float spacing     = ImGui::GetStyle().ItemSpacing.x;
    float columnWidth = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;

    int seconds = static_cast<int>(
        std::chrono::duration<double>(std::chrono::steady_clock::now() - recordTimeStart).count());
    double megabytes = m_data->recSamplesWritten.load() * sizeof(RecordSample_S) / 1e6;

    ImGui::BeginGroup();
    ImGui::Text("Time");
    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild("##TimeValue",
                          ImVec2(columnWidth, 0.0f),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        if (recording)
            ImGui::Text("%02d:%02d:%02d", seconds / 3600, (seconds / 60) % 60, seconds % 60);
        else
            ImGui::TextDisabled("Record not started");
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("Memory");
    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild("##MemoryValue",
                          ImVec2(columnWidth, 0.0f),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        if (recording)
            ImGui::Text("%.1f MB", megabytes);
        else
            ImGui::TextDisabled("--");
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();
    ImGui::EndGroup();
}

void MonitorMenu::drawExportButtons()
{
    const float browseWidth = ImGui::CalcTextSize("Browse...").x + 16.0f;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Save CSV to directory:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - browseWidth);

    GuiStyle::buttonStyle();
    if (ImGui::Button("Browse...", ImVec2(browseWidth, GuiStyle::smallButtonHeight)))
        ImGui::OpenPopup("Choose export folder");
    GuiStyle::endButtonStyle();

    drawFolderBrowserPopup("Choose export folder", exportDir);

    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild(
            "##Path", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        ImGui::TextWrapped("%s", exportDir.string().c_str());
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();

    const char* preview =
        (selectedRecording >= 0) ? recordingFiles[selectedRecording].c_str() : "None";

    ImGui::Text("Select Recording");
    GuiStyle::comboStyle();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##Recording", preview))
    {
        if (ImGui::IsWindowAppearing())
            refreshRecordingList();

        for (int i = 0; i < (int)recordingFiles.size(); ++i)
        {
            if (ImGui::Selectable(recordingFiles[i].c_str(), i == selectedRecording))
            {
                selectedRecording = i;
            }
        }
        ImGui::EndCombo();
    }
    GuiStyle::endComboStyle();

    ImGui::BeginDisabled(selectedRecording < 0);
    GuiStyle::buttonStyle();
    if (ImGui::Button("Export to CSV", ImVec2(-FLT_MIN, GuiStyle::smallButtonHeight)))
    {
        std::filesystem::path bin(recordingFiles[selectedRecording]);
        std::filesystem::path csv = exportDir / bin.filename();
        csv.replace_extension("");

        std::error_code error;
        std::filesystem::create_directories(exportDir, error);

        exportOk = !error && exportToCsv(bin.string(), csv.string());
    }
    GuiStyle::endButtonStyle();
    ImGui::EndDisabled();
}

bool MonitorMenu::drawFolderBrowserPopup(const char* popupId, std::filesystem::path& result)
{
    bool picked = false;

    GuiStyle::popupStyle();
    ImGui::SetNextWindowSize(ImVec2(500.0f, 400.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(popupId, nullptr, ImGuiWindowFlags_NoSavedSettings))
    {
        if (ImGui::IsWindowAppearing())
        {
            std::error_code error;
            if (std::filesystem::is_directory(result, error))
                setBrowsePath(result);
            else if (std::filesystem::is_directory(result.parent_path(), error))
                setBrowsePath(result.parent_path());
            else
                setBrowsePath(std::filesystem::current_path());
        }
        GuiStyle::buttonStyle();
        if (ImGui::Button("Up", ImVec2(GuiStyle::decisionButtonWidth * 0.5f, 0.0f)) &&
            browsePath.has_parent_path())
            setBrowsePath(browsePath.parent_path());
        GuiStyle::endButtonStyle();

        ImGui::SameLine();

        GuiStyle::comboStyle(nullptr, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, GuiStyle::buttonColor);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##path", pathBuf, sizeof(pathBuf));
        ImGui::PopStyleColor();
        GuiStyle::endComboStyle();

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            std::string typed = pathBuf;
            if (!typed.empty() && typed[0] == '~')
                if (const char* home = std::getenv("HOME"))
                    typed = home + typed.substr(1);

            std::error_code       error;
            std::filesystem::path target = std::filesystem::weakly_canonical(typed, error);
            if (!error && std::filesystem::is_directory(target, error))
                setBrowsePath(target);
            else
                pathError = true;
        }

        if (pathError)
            ImGui::TextColored(GuiStyle::redColor, "Folder not found");

        std::vector<std::filesystem::path> directories;
        std::error_code                    error;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(
                 browsePath, std::filesystem::directory_options::skip_permission_denied, error))
            if (entry.is_directory(error) && entry.path().filename().string()[0] != '.')
                directories.push_back(entry.path());
        std::sort(directories.begin(), directories.end());

        const float footerHeight = GuiStyle::mediumButtonHeight + ImGui::GetStyle().ItemSpacing.y;

        GuiStyle::tableStyle();
        if (ImGui::BeginTable(
                "##directories", 1, GuiStyle::flagsTable, ImVec2(0.0f, -footerHeight)))
        {
            for (const std::filesystem::path& directory : directories)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(directory.filename().string().c_str(),
                                      false,
                                      ImGuiSelectableFlags_SpanAllColumns))
                    setBrowsePath(directory);
            }

            if (directories.empty())
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextDisabled("(no subfolders)");
            }
            ImGui::EndTable();
        }
        GuiStyle::endTableStyle();

        const ImVec2 decisionSize(GuiStyle::decisionButtonWidth, GuiStyle::smallButtonHeight);
        const float  spacing = ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
                             decisionSize.x * 2.0f - spacing);

        GuiStyle::buttonStyle();
        if (ImGui::Button("Cancel", decisionSize))
            ImGui::CloseCurrentPopup();
        GuiStyle::endButtonStyle();

        ImGui::SameLine();

        bool primary = true;
        GuiStyle::buttonImportantStyle(primary);
        if (ImGui::Button("Select", decisionSize))
        {
            result = browsePath;
            picked = true;
            ImGui::CloseCurrentPopup();
        }
        GuiStyle::endButtonImportantStyle();

        ImGui::EndPopup();
    }
    GuiStyle::endPopupStyle();

    return picked;
}

/*

 Draw Plots

*/

void MonitorMenu::drawDevicePlot(PlotSignal_E signal, const char* plotId, const char* yLabel)
{
    const int devices = m_data->devicePlotsCount.load(std::memory_order_acquire);

    float startTime   = 0.0f;
    float currentTime = 0.0f;
    bool  first       = true;

    for (int i = 0; i < devices; ++i)
    {
        const DevicePlot_S& p = m_data->devicePlots[i];
        const uint32_t      n = p.writeData.load(std::memory_order_acquire);
        if (n == 0)
            continue;

        const float oldest = (n < DevicePlot_S::SIZE) ? p.time[0] : p.time[n % DevicePlot_S::SIZE];
        const float newest = p.time[(n - 1) % DevicePlot_S::SIZE];

        startTime   = first ? oldest : std::min(startTime, oldest);
        currentTime = first ? newest : std::max(currentTime, newest);
        first       = false;
    }
    if (currentTime <= startTime)
        currentTime = startTime + 1.0f;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (ImPlot::BeginPlot(plotId, ImGui::GetContentRegionAvail()))
    {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_X1, startTime, currentTime, ImPlotCond_Always);

        ImPlot::SetupAxis(ImAxis_Y1, yLabel, ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_AutoFit);

        for (int i = 0; i < devices; ++i)
        {
            const DevicePlot_S& p = m_data->devicePlots[i];
            const uint32_t      n = p.writeData.load(std::memory_order_acquire);
            if (n == 0)
                continue;

            const int pointsCount = static_cast<int>(std::min(n, DevicePlot_S::SIZE));

            ImPlotSpec spec;
            spec.Offset = (n < DevicePlot_S::SIZE) ? 0 : static_cast<int>(n % DevicePlot_S::SIZE);

            char label[16];
            std::snprintf(label, sizeof(label), "ID %u", static_cast<unsigned>(p.id));

            ImPlot::PlotLine(label, p.time, p.signal(signal), pointsCount, spec);
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleColor();
}

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

void MonitorMenu::refreshRecordingList()
{
    std::string previous;
    if (selectedRecording >= 0 && selectedRecording < (int)recordingFiles.size())
        previous = recordingFiles[selectedRecording];

    recordingFiles.clear();
    selectedRecording = -1;

    if (!std::filesystem::exists("recordings"))
        return;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator("recordings"))
    {
        if (entry.path().extension() == ".bin")
            recordingFiles.push_back(entry.path().string());
    }
    std::sort(recordingFiles.begin(), recordingFiles.end());

    for (int i = 0; i < (int)recordingFiles.size(); ++i)
    {
        if (recordingFiles[i] == previous)
            selectedRecording = i;
    }
}

std::filesystem::path MonitorMenu::defaultExportDir()
{
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / "Documents" / "recordings";
}

void MonitorMenu::setBrowsePath(const std::filesystem::path& path)
{
    browsePath = path;
    snprintf(pathBuf, sizeof(pathBuf), "%s", path.string().c_str());
    pathError = false;
}

void MonitorMenu::updateRecordingState()
{
    const bool recording = m_data->recording.load();

    if (recording && !guiWasRecording)
    {
        recordTimeStart           = std::chrono::steady_clock::now();
        m_data->recSamplesWritten = 0;
    }
    if (!recording && guiWasRecording)
    {
        refreshRecordingList();
        selectedRecording = (int)recordingFiles.size() - 1;
    }

    guiWasRecording = recording;
}