file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/ncnn_centernet.h
)

message(${SRC_FILES})

# list(APPEND SRCS ${LAYER_ARCH_SRC})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fPIC -std=c++11 -fopenmp")
add_library(centernet-ncnn STATIC ${SRC_FILES})

if(MSVC)
	message("windows")
	include_directories(${CMAKE_SOURCE_DIR}/include/win)
	link_directories(${CMAKE_SOURCE_DIR}/lib/win)
	target_link_libraries(centernet-ncnn PUBLIC opencv_world344 ncnn)
else()
	find_package(OpenCV REQUIRED)
	message("Opencv Version: ${OPENCV_VERSION}" )
	target_link_libraries(centernet-ncnn PUBLIC ${OpenCV_LIBS} ncnn)
endif()

if(MIRROR_OPENMP)
    find_package(OpenMP)
    if(NOT TARGET OpenMP::OpenMP_CXX AND (OpenMP_CXX_FOUND OR OPENMP_FOUND))
        target_compile_options(centernet-ncnn PRIVATE ${OpenMP_CXX_FLAGS})
    endif()
endif()

if(MIRROR_OPENMP AND OpenMP_CXX_FOUND)
    message("Building with OpenMP")
    target_link_libraries(centernet-ncnn PUBLIC OpenMP::OpenMP_CXX)
endif()

if(MIRROR_VULKAN)
    find_package(Vulkan REQUIRED)
    target_link_libraries(centernet-ncnn PUBLIC Vulkan::Vulkan)
endif()

target_include_directories(centernet-ncnn
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include/vision>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mnist>        
)

if(MIRROR_INSTALL_SDK)
    install(TARGETS centernet-ncnn EXPORT centernet-ncnn ARCHIVE DESTINATION ${PROJECT_BINARY_DIR}/lib)
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/common/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/face/face_engine.h
        ${CMAKE_CURRENT_SOURCE_DIR}/object/object_engine.h
        ${CMAKE_CURRENT_SOURCE_DIR}/classifier/classifier_engine.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/ncnn_centernet.h
        DESTINATION ${PROJECT_BINARY_DIR}/include
    )
endif()

# Add Python
if(EXISTS /usr/include/python3.6)
    include_directories(/usr/include/python3.6)
else()
    message(SEND_ERROR "NOT EXISTS /usr/include/python3.6")
endif()

# Add numpy
if(EXISTS /usr/local/lib/python3.6/dist-packages/numpy/core/include)
    include_directories(/usr/local/lib/python3.6/dist-packages/numpy/core/include)
elseif(EXISTS $ENV{HOME}/.local/lib/python3.6/site-packages/numpy/core/include)
    include_directories($ENV{HOME}/.local/lib/python3.6/site-packages/numpy/core/include)
elseif(EXISTS /usr/lib/python3/dist-packages/numpy/core/include)
    include_directories(/usr/lib/python3/dist-packages/site-packages/numpy/core/include)
else()
    message(SEND_ERROR "NOT EXISTS numpy lib")
endif()

file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/cpp/*.cpp
                   ${CMAKE_CURRENT_LIST_DIR}/demo.cpp)
set(TEST_COMPILE_CODE ${TEST_SRC})
# add_executable(centernet-ncnn-bin ${SRC_FILES})
# target_link_libraries(centernet-ncnn-bin PUBLIC centernet-ncnn)

add_executable(demo ${TEST_COMPILE_CODE})
target_link_libraries(demo 
                      ncnn
                      ${OpenCV_LIBS})
