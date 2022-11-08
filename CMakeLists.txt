cmake_minimum_required(VERSION 2.8)
project( lan_vid_pseudostream )
find_package( OpenCV REQUIRED )
add_executable( server Server.cpp PracticalSocket.cpp )
target_compile_options(server PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wno-deprecated >
     $<$<CXX_COMPILER_ID:MSVC>:
          /W2>)
target_link_libraries( server ${OpenCV_LIBS} )
add_executable( client Client.cpp PracticalSocket.cpp )
target_compile_options(client PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wno-deprecated >
     $<$<CXX_COMPILER_ID:MSVC>:
          /W2>)
target_link_libraries( client ${OpenCV_LIBS} )
