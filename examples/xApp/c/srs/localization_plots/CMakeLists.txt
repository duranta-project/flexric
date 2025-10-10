include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

###################################################################
# Dependencies
###################################################################
# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 REQUIRED)

# GLAD
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

add_subdirectory(third_party)

###############################################################################
# APP FRAMEWORK
###############################################################################

add_library(app
  common/App.h
  common/App.cpp
  common/Shader.h
  common/Image.h
  common/Fonts/Fonts.h
  common/Fonts/FontAwesome5.cpp
  common/Fonts/RobotoRegular.cpp
  common/Fonts/RobotoBold.cpp
  common/Fonts/RobotoItalic.cpp
  common/Fonts/RobotoMonoRegular.cpp
  common/Fonts/RobotoMonoBold.cpp
  common/Fonts/RobotoMonoItalic.cpp
)
target_include_directories(app PUBLIC common third_party/glad/include)
target_link_libraries(app ImPlot::ImPlot glad)

###################################################################
# DEMO APP
###################################################################
# main ImPlot demo
add_executable(demo "demo/demo.cpp")

target_link_libraries(demo PRIVATE app)

######################################################################
# INTERFACE library for the localization GUI
######################################################################
add_library(gui_if INTERFACE)
target_include_directories(gui_if INTERFACE inc)

target_link_libraries(gui_if INTERFACE app)
