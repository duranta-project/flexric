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

void Demo_Config() {
    ImGui::ShowFontSelector("Font");
    ImGui::ShowStyleSelector("ImGui Style");
    ImPlot::ShowStyleSelector("ImPlot Style");
    ImPlot::ShowColormapSelector("ImPlot Colormap");
    ImPlot::ShowInputMapSelector("Input Map");
}

struct ChannelApp : App {
    // Add channel buffers as members
    size_t m_ofdm_symbol_size;
    std::vector<float> m_srs_cir;
    std::mutex m_cir_mutex;

    std::vector<float> m_srs_cfr;
    std::mutex m_cfr_mutex;

    std::vector<float> rx_x, rx_y;
    std::vector<float> tp_x, tp_y;
    bool show_antennas    = true;
    bool show_test_points = true;

    std::mutex                     m_cc_mutex;

    std::unordered_map<uint32_t, std::vector<std::pair<float,float>>> m_ue_map;
    // Add setters to update the data and plot it in real-time

    void UpdateCIR(const std::vector<float>& srs_cir) {
        std::lock_guard<std::mutex> lk(m_cir_mutex);
        m_srs_cir = srs_cir;
    }
 
    void UpdateCFR(const std::vector<float>& srs_cfr) {
        std::lock_guard<std::mutex> lk(m_cfr_mutex);
        m_srs_cfr = srs_cfr;
    } 

    void UpdateCC(const std::unordered_map<uint32_t, std::vector<float>>& ue_map) {
        std::lock_guard<std::mutex> lk(m_cc_mutex);
 
      for(const auto& key_value: ue_map) {
//        auto& history = m_ue_map[key_value.first];
        m_ue_map[key_value.first].emplace_back(key_value.second [0], key_value.second[1]);

        if (m_ue_map[key_value.first].size() > 5000) {
            m_ue_map[key_value.first].erase(m_ue_map[key_value.first].begin());
        }
      }
    }

    // Constructor
    ChannelApp(const std::string& title, int argc, char const* argv[], size_t Nfft) 
     : App(title, 1280, 720, argc, argv),
       m_ofdm_symbol_size(Nfft),
       m_srs_cir(Nfft),
       m_srs_cfr(Nfft)
      {
       // Load CSV data once
       {
          std::ifstream in("/home/bouknana/srs_data/rx_positions.csv");
          std::string line;
          while (std::getline(in, line)) {
            std::stringstream ss(line);
            float x, y;
            char comma;
            if (ss >> x >> comma >> y) {
              rx_x.push_back(x);
              rx_y.push_back(y);
            }
          }
         
       }
       {
          std::ifstream in("/home/bouknana/srs_data/test_points.csv");
          std::string line;
          while (std::getline(in, line)) {
            std::stringstream ss(line);
            float x, y;
            char comma;
            if (ss >> x >> comma >> y) {
              tp_x.push_back(x);
              tp_y.push_back(y);
            }
          }
       }

      }

    void Update() override {

        // pull the new data for plotting
        std::vector<float> local_cir;
        {
          std::lock_guard<std::mutex> lk(m_cir_mutex);
          local_cir = m_srs_cir;
        }

        std::vector<float> local_cfr;
        {
          std::lock_guard<std::mutex> lk(m_cfr_mutex);
          local_cfr = m_srs_cfr;
        }

        std::unordered_map<uint32_t, std::vector<std::pair<float, float>>> local_map_cp;
        std::vector<float> local_cc_0;
        std::vector<float> local_cc_1;
        {
            std::lock_guard<std::mutex> lk(m_cc_mutex);
            local_map_cp = m_ue_map;
        }

        ImGui::Begin("Localization with Channel Charting");
          if (ImGui::BeginTabBar("xAppDemoTabs")) {
            if (ImGui::BeginTabItem("Plots")) {
               // Create section for CIR plots
               if (ImGui::CollapsingHeader("Localization with Channel Charting", ImGuiTreeNodeFlags_DefaultOpen)) {
		  ImGui::Columns(2, nullptr, false);
		  // Plot CIR
		  if (ImPlot::BeginPlot("SRS Channel Impulse Response", ImVec2(-1,300), ImPlotFlags_None)) {
		    //ImPlot::SetNextPlotLimitsX(2060, 2150, ImGuiCond_Always);
		    ImPlot::SetupAxis(ImAxis_X1, "Sample Index");
		    ImPlot::SetupAxis(ImAxis_Y1, "|h|");
		    ImPlot::PlotLine("CIR", local_cir.data(), (int)m_ofdm_symbol_size);
		    ImPlot::EndPlot();
		  }
		  ImGui::NextColumn();
		  // Plot CFR
		  if (ImPlot::BeginPlot("SRS Channel Frequency Response", ImVec2(-1,300), ImPlotFlags_None)) {
		    ImPlot::SetupAxis(ImAxis_X1, "Subcarrier Index");
		    ImPlot::SetupAxis(ImAxis_Y1, "|H|");
		    ImPlot::PlotLine("CFR", local_cfr.data(), (int)m_ofdm_symbol_size);
		    ImPlot::EndPlot();
		  }
		  ImGui::Columns(1);  // back to single column
	       }
	      // Create section for Testbed map
	       if (ImGui::CollapsingHeader("Testbed - UE position tracking", ImGuiTreeNodeFlags_DefaultOpen)) {
		  ImGui::Checkbox("RX Antennas",    &show_antennas);
		  ImGui::Checkbox("Test Points", &show_test_points);
		  if (ImPlot::BeginPlot("##map", ImVec2(-1,450), ImPlotFlags_None)) {
		    ImPlot::SetupAxisLimits(ImAxis_X1, -10, 60);
		    ImPlot::SetupAxisLimits(ImAxis_Y1, -10, 40, ImPlotAxisFlags_Invert);
		    if (show_antennas){
		       ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 6.0f, ImVec4(0.314f, 0.980f, 0.482f, 1.0f), IMPLOT_AUTO, ImVec4(0.314f, 0.980f, 0.482f, 1.0f));
		       ImPlot::PlotScatter("RX Antennas", rx_x.data(), rx_y.data(), rx_x.size());
		    }
		    if (show_test_points){
                       //ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 6.0f, ImVec4(0,0,1,1), IMPLOT_AUTO, ImVec4(0,0,0,0));
		       ImPlot::PlotScatter("Test Points", tp_x.data(), tp_y.data(), tp_x.size());
		    }

        for (const auto& [ue_id, history] : local_map_cp) {

          for (const auto& [x, y] : history) {
              local_cc_0.push_back(x);
              local_cc_1.push_back(y);
          }

          std::string label = "UE " + std::to_string(ue_id);
          ImPlot::PlotScatter(label.c_str(), local_cc_0.data(), local_cc_1.data(), (int)local_cc_0.size());
/*
          if (!local_cc_0.empty()) {
              //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f, ImVec4(0.314f, 0.980f, 0.482f, 1.0f));
              std::string latest_label = "Latest position for UE" + std::to_string(ue_id);
              ImPlot::PlotScatter(latest_label.c_str(), &local_cc_0.back(), &local_cc_1.back(), 1);
          }
*/
        }
        ImPlot::EndPlot();
		  }
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

