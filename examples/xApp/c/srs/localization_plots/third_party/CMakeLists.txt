###############################################################################
# IMGUI
###############################################################################

FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(ImGui)

set(IMGUI_SRC
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp")

add_library(ImGui ${IMGUI_SRC})
add_library(ImGui::ImGui ALIAS ImGui)
target_link_libraries(ImGui PUBLIC glfw glad OpenGL::GL)
target_include_directories(ImGui PUBLIC "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/examples" "${imgui_SOURCE_DIR}/examples/libs/gl3w" "${imgui_SOURCE_DIR}/backends" "${imgui_SOURCE_DIR}/misc/cpp")

###############################################################################
# ImPlot
###############################################################################
FetchContent_Declare(ImPlot
    GIT_REPOSITORY https://github.com/epezent/implot
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(ImPlot)

add_library(ImPlot
    "${implot_SOURCE_DIR}/implot.cpp"
    "${implot_SOURCE_DIR}/implot_items.cpp"
    "${implot_SOURCE_DIR}/implot_demo.cpp")
 
add_library(ImPlot::ImPlot ALIAS ImPlot)
target_include_directories(ImPlot PUBLIC "${implot_SOURCE_DIR}" "${imgui_SOURCE_DIR}")
target_link_libraries(ImPlot PUBLIC ImGui::ImGui)

