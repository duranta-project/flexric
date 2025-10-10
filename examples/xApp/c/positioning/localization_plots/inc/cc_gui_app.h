// App: Plot SRS CIR in Real time

#include "App.h"
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <mutex>
#include <unordered_map>
#include <tuple>

void Demo_Config() {
    ImGui::ShowFontSelector("Font");
    ImGui::ShowStyleSelector("ImGui Style");
    ImPlot::ShowStyleSelector("ImPlot Style");
    ImPlot::ShowColormapSelector("ImPlot Colormap");
    ImPlot::ShowInputMapSelector("Input Map");
}

struct ChannelApp : App {
    // Add channel buffers as members

    std::vector<std::vector<float>> m_srs_cfr;
    std::mutex m_cfr_mutex;

    // Add setters to update the data and plot it in real-time
 
    void UpdateCFR(const std::vector<std::vector<float>>& srs_cfr) {
        std::lock_guard<std::mutex> lk(m_cfr_mutex);
        m_srs_cfr = srs_cfr;
    } 


    // Constructor
    ChannelApp(const std::string& title, int argc, char const* argv[]) 
     : App(title, 1280, 720, argc, argv),
       m_srs_cfr(8*272)
      {

      }

    void Update() override {


        std::vector<std::vector<float>> local_cfr;
        {
          std::lock_guard<std::mutex> lk(m_cfr_mutex);
          local_cfr = m_srs_cfr;
        }


        ImGui::Begin("SRS Channel");
          if (ImGui::BeginTabBar("SRS Channel")) {
            if (ImGui::BeginTabItem("Channel Plots")) {

            if (ImPlot::BeginPlot("Subsampled SRS estimates", ImGui::GetContentRegionAvail(), ImPlotFlags_None)) {
              ImPlot::SetupAxis(ImAxis_X1, "Subcarrier Index");
              ImPlot::SetupAxis(ImAxis_Y1, "|H|");
              ImPlot::PlotLine("CFR", local_cfr[0].data(), local_cfr[0].size());
             ImPlot::EndPlot();
          }

    ImGui::EndTabItem();
    }
             if (ImGui::BeginTabItem("Config")) {
                Demo_Config();
                ImGui::EndTabItem();
             }
             ImGui::EndTabBar();
      }
        ImGui::End();

    };
};

