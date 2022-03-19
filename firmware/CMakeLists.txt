cmake_minimum_required()

project(zynq_development)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -ggdb -Wno-psabi -lpthread -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c++17 -ggdb -Wno-psabi -lpthread -pthread -Wno-unused-variable -Wno-unused-parameter")

include_directories(include)

if(XVC_SERVER)
    message("BUILD JTAG-XVCSERVER APP")
    file(GLOB SOURCE "src/xvcserver.c")
    add_executable(xvc_server ${SOURCE})
elseif(TEMPERATURE_UNITTEST)
    message("BUILD TEMPERATURE UNIT TEST")
    add_executable(temperature_unittest ${SOURCE})
endif(XVC_SERVER)


