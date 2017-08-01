cmake_minimum_required(VERSION 2.8)
# Project Name
PROJECT(opengl_videoPlayer)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
        MESSAGE("CXXFLAGS: ${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        MESSAGE("CXXFLAGS: ${CMAKE_CXX_FLAGS}")

endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR};/sw/config/mlib/cmake/Modules")
set(project_LIBRARIES)
set(project_INCLUDES)
set(project_DEFINITIONS)


set(synch_DIR "/sw/synch/cmake")

#########################################################
# ADD PTHREAD
#########################################################
list(APPEND project_LIBRARIES pthread)

find_package(synch REQUIRED)
list(APPEND project_LIBRARIES synch )


 #########################################################
 # ADD FFMPEG
 #########################################################
 find_package(FFMPEG REQUIRED)
 list(APPEND project_LIBRARIES ${FFMPEG_LIBRARIES})
 list(APPEND project_INCLUDES ${FFMPEG_INCLUDE_DIR})




#########################################################
# FIND X11
#########################################################
find_package(X11 REQUIRED)
list(APPEND project_INCLUDES ${X11_INCLUDE_DIR})
list(APPEND project_LIBRARIES ${X11_LIBRARIES})
list(APPEND project_DEFINITIONS ${X11_DEFINITIONS})



#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
list(APPEND project_INCLUDES ${OPENGL_INCLUDE_DIRS})
list(APPEND project_LIBRARIES ${OPENGL_LIBRARIES})
list(APPEND project_DEFINITIONS ${OPENGL_DEFINITIONS})



if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)



#########################################################
# FIND GLEW
#########################################################
find_package(GLEW REQUIRED)
#list(APPEND project_LIBRARIES GLEW)

list(APPEND project_INCLUDES ${GLEW_INCLUDE_DIRS})
list(APPEND project_LIBRARIES ${GLEW_LIBRARIES})
list(APPEND project_DEFINITIONS ${GLEW_DEFINITIONS})



#########################################################
# FIND GLUT
#########################################################

find_package(GLUT REQUIRED)

list(APPEND project_INCLUDES ${GLUT_INCLUDE_DIRS})
list(APPEND project_LIBRARIES ${GLUT_LIBRARIES})
list(APPEND project_DEFINITIONS ${GLUT_DEFINITIONS})


#########################################################
# FIND PULSEAUDIO
#########################################################

find_package(PulseAudio REQUIRED)

list(APPEND project_INCLUDES ${PULSEAUDIO_INCLUDE_DIRS})
list(APPEND project_LIBRARIES ${PULSEAUDIO_LIBRARIES})





#########################################################
# Include Files
#########################################################
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${project_INCLUDES})


set(HEADERS
  include/Renderer.h
  include/Decoder.h
  include/Pulseplayer.h
)
set(SOURCES
       src/main.cpp
       src/Renderer.cpp
       src/Decoder.cpp
       src/Pulseplayer.cpp
)




set(HEADERS_SERVER
    include/Renderer.h
    include/Decoder.h
    include/Pulseplayer.h

)

set(SOURCES_SERVER
    src/main_server.cpp
    src/Pulseplayer.cpp
    src/Renderer.cpp
    src/Decoder.cpp
)


set(HEADERS_LOCAL
    include/Renderer.h
    include/Decoder.h
    include/Pulseplayer.h
)

set(SOURCES_LOCAL
    src/main_local.cpp
    src/Pulseplayer.cpp
    src/Renderer.cpp
    src/Decoder.cpp
)


MESSAGE("libs: ${project_LIBRARIES}")
########################################################
# Add definitions
#########################################################
add_definitions(${project_DEFINITIONS})

########################################################
# Linking & stuff
#########################################################


#add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
#target_link_libraries(${PROJECT_NAME} ${project_LIBRARIES})
#add_executable(${PROJECT_NAME}_server ${SOURCES_SERVER} ${HEADERS_SERVER})
#target_link_libraries(${PROJECT_NAME}_server ${project_LIBRARIES})
add_executable(${PROJECT_NAME}_local ${SOURCES_LOCAL} ${HEADERS_LOCAL})
target_link_libraries(${PROJECT_NAME}_local ${project_LIBRARIES})
