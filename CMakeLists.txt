cmake_minimum_required(VERSION 3.16)
project(megaApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Test)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/logger/logger.cpp
    src/utils/utils.cpp
    src/worker.cpp
)

set(HEADERS
    src/mainwindow.h
    src/config.h
    src/worker.h
    src/logger/logger.h
    src/logger/loggerConfig.h
    src/utils/utils.h
    src/models/taskModel.h
    src/models/workerModel.h
)

set(UIS
    ui/mainwindow.ui
)

qt_add_executable(megaApp ${SOURCES} ${HEADERS} ${UIS})

target_include_directories(megaApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

target_link_libraries(megaApp PRIVATE Qt6::Widgets Qt6::Core)

enable_testing()

set(XOR_TEST_SOURCES
    tests/testWorkerXor.cpp
    src/worker.cpp
)

set(XOR_TEST_HEADERS
    tests/testWorkerXor.h
)

qt_add_executable(TestWorker ${XOR_TEST_SOURCES} ${XOR_TEST_HEADERS})

target_include_directories(TestWorker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

target_link_libraries(TestWorker PRIVATE Qt6::Widgets Qt6::Core Qt6::Test)

add_test(NAME WorkerXorTests COMMAND TestWorker)

set(LOGGER_TEST_SOURCES
    src/logger/logger.cpp
    tests/testLogger.cpp
)

set(LOGGER_TEST_HEADERS
    src/logger/loggerConfig.h
    tests/testLogger.h
)

qt_add_executable(TestLogger ${LOGGER_TEST_SOURCES} ${LOGGER_TEST_HEADERS})

target_include_directories(TestLogger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

target_link_libraries(TestLogger PRIVATE Qt6::Widgets Qt6::Core Qt6::Test)

add_test(NAME LoggerTest COMMAND TestLogger)
