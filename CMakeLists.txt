cmake_minimum_required(VERSION 2.8)
project( BadmintonEvolution )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)


add_executable( bg src/bg/bg.cpp )
target_link_libraries( bg ${OpenCV_LIBS} )

add_executable( show-bg src/bg/show-bg.cpp )
target_link_libraries( show-bg ${OpenCV_LIBS} )


add_executable( tag src/tag/tag.cpp )
target_link_libraries( tag ${OpenCV_LIBS} )

add_executable( show-tagged-vid src/tag/show-tagged-vid.cpp )
target_link_libraries( show-tagged-vid ${OpenCV_LIBS} )


add_executable( court src/court/court.cpp )
target_link_libraries( court ${OpenCV_LIBS} )

add_executable( show-court src/court/show-court.cpp )
target_link_libraries( show-court ${OpenCV_LIBS} )


add_executable( player src/player/player.cpp )
target_link_libraries( player ${OpenCV_LIBS} )

add_executable( show-player-vid src/player/show-player-vid.cpp )
target_link_libraries( show-player-vid ${OpenCV_LIBS} )


add_executable( crop src/crop/crop.cpp )
target_link_libraries( crop ${OpenCV_LIBS} )

add_executable( show-crop-vid src/crop/show-crop-vid.cpp )
target_link_libraries( show-crop-vid ${OpenCV_LIBS} )


add_executable( show-pose-vid src/pose/show-pose-vid.cpp )
target_link_libraries( show-pose-vid ${OpenCV_LIBS} )