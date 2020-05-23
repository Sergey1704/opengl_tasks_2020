set(SRC_FILES  ShaderProgram.cpp 
    Surface.cpp)


MAKE_OPENGL_TASK(692Naumov 1 "${SRC_FILES}")

if (UNIX)
    target_link_libraries(692Naumov stdc++fs)
endif()
