# Handle external libraries.

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
add_subdirectory(Assimp)
set(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Assimp/include ${CMAKE_CURRENT_BINARY_DIR}/Assimp/include PARENT_SCOPE)
set(ASSIMP_LIBS assimp PARENT_SCOPE)

# GLEW
set(BUILD_UTILS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(glew-1.13.0/build/cmake)
set(GLEW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glew-1.13.0/include)
set(GLEW_INCLUDE_DIR "${GLEW_INCLUDE_DIR}" PARENT_SCOPE)
set(GLEW_LIBS glew_s PARENT_SCOPE)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(GLFW)
set(GLFW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GLFW/include PARENT_SCOPE)
set(GLFW_LIBS glfw PARENT_SCOPE)

# GLM
set(GLM_TEST_ENABLE OFF CACHE INTERNAL "" FORCE)
add_subdirectory(GLM)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GLM PARENT_SCOPE)

# imgui
add_subdirectory(imgui)
set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui PARENT_SCOPE)
set(IMGUI_LIBS imgui PARENT_SCOPE)

# SOIL
add_subdirectory(SOIL)
set(SOIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SOIL/src/SOIL2 PARENT_SCOPE)
set(SOIL_LIBS soil PARENT_SCOPE)
