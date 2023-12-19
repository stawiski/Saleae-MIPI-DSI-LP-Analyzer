cmake_minimum_required (VERSION 3.13)

project(MIPI_DSI_LP_Analyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
source/MIPI_DSI_LP_Analyzer.cpp
source/MIPI_DSI_LP_Analyzer.h
source/MIPI_DSI_LP_AnalyzerResults.cpp
source/MIPI_DSI_LP_AnalyzerResults.h
source/MIPI_DSI_LP_AnalyzerSettings.cpp
source/MIPI_DSI_LP_AnalyzerSettings.h
source/MIPI_DSI_LP_SimulationDataGenerator.cpp
source/MIPI_DSI_LP_SimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
