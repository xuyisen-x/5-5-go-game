cmake_minimum_required(VERSION 3.15)
project(RenjuAI LANGUAGES CXX)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ONNXRUNTIME_LIBRARY "/home/xuyisen/download/onnxruntime-linux-x64-static_lib-1.17.0-gcc-11/lib/libonnxruntime.a")

# Console test
add_executable(test
                test.cpp
                GoGame/GoGame.cpp
                Model/ONNXEngine.cpp
                AI/RandomAI.cpp
                AI/MCTSAI.cpp
                AI/ExhaustiveTree.cpp)

add_executable(selfplay
                selfplay.cpp
                GoGame/GoGame.cpp
                Model/ONNXEngine.cpp
                AI/RandomAI.cpp
                AI/MCTSAI.cpp
                AI/ExhaustiveTree.cpp)

add_executable(humanplay
                human.cpp
                GoGame/GoGame.cpp
                Model/ONNXEngine.cpp
                AI/RandomAI.cpp
                AI/MCTSAI.cpp
                AI/ExhaustiveTree.cpp)

add_executable(gtp
                GTPengine.cpp
                GoGame/GoGame.cpp
                Model/ONNXEngine.cpp
                AI/RandomAI.cpp
                AI/MCTSAI.cpp
                AI/ExhaustiveTree.cpp)

# Library test
add_library(get_input SHARED 
            get_input.cpp
            GoGame/GoGame.cpp
            Model/ONNXEngine.cpp
            AI/RandomAI.cpp
            AI/MCTSAI.cpp
            AI/ExhaustiveTree.cpp)

target_link_libraries(test PRIVATE
                     ${ONNXRUNTIME_LIBRARY})

target_link_libraries(humanplay PRIVATE
                     ${ONNXRUNTIME_LIBRARY})

target_link_libraries(gtp PRIVATE
                     ${ONNXRUNTIME_LIBRARY})

target_link_libraries(selfplay PRIVATE
                     ${ONNXRUNTIME_LIBRARY}
                     /home/xuyisen/download/hdf/HDF5-1.14.3-Linux/HDF_Group/HDF5/1.14.3/lib/libhdf5.so
                     /home/xuyisen/download/hdf/HDF5-1.14.3-Linux/HDF_Group/HDF5/1.14.3/lib/libhdf5_cpp.so)

target_include_directories(selfplay PRIVATE 
                          /home/xuyisen/download/hdf/HDF5-1.14.3-Linux/HDF_Group/HDF5/1.14.3/include)

target_link_libraries(get_input PRIVATE
                     ${ONNXRUNTIME_LIBRARY})