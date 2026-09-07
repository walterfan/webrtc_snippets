cmake_minimum_required(VERSION 3.20)

project(cpp_rtc_practice_lab LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_compile_options(
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-g>
)

include(CTest)

option(PRACTICE_BUILD_WEB_SERVER "Build the local Crow learning website" OFF)

file(GLOB EXERCISE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/exercises/*/starter.cpp")

add_library(practice_core
    ${EXERCISE_SOURCES}
    src/sip_message.cpp
    src/catalog.cpp
    src/runner.cpp
)
target_include_directories(practice_core PUBLIC include)
target_compile_features(practice_core PUBLIC cxx_std_20)
target_compile_options(practice_core PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>
)

add_executable(cpp_rtc_practice src/main.cpp)
target_link_libraries(cpp_rtc_practice PRIVATE practice_core)

add_custom_target(practice_standalone)

function(add_practice_exercise day)
    add_executable(practice_${day} EXCLUDE_FROM_ALL src/standalone_main.cpp)
    target_link_libraries(practice_${day} PRIVATE practice_core)
    target_compile_definitions(practice_${day} PRIVATE PRACTICE_DAY=${day})
    add_dependencies(practice_standalone practice_${day})
endfunction()

foreach(day RANGE 1 30)
    if(day LESS 10)
        add_practice_exercise(0${day})
    else()
        add_practice_exercise(${day})
    endif()
endforeach()

if(PRACTICE_BUILD_WEB_SERVER)
    find_package(Crow CONFIG REQUIRED)

    add_library(practice_web_core
        src/exercise_repository.cpp
        src/test_runner.cpp
    )
    target_include_directories(practice_web_core PUBLIC include)
    target_link_libraries(practice_web_core PUBLIC practice_core)
    target_compile_features(practice_web_core PUBLIC cxx_std_20)
    target_compile_options(practice_web_core PRIVATE
        $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>
    )

    add_executable(practice_web_server
        src/web_main.cpp
        src/web_server.cpp
    )
    target_link_libraries(practice_web_server PRIVATE practice_web_core Crow::Crow)
    target_compile_features(practice_web_server PRIVATE cxx_std_20)
    target_compile_options(practice_web_server PRIVATE
        $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>
    )
    target_compile_definitions(practice_web_server PRIVATE
        PRACTICE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        PRACTICE_CMAKE_COMMAND="${CMAKE_COMMAND}"
        PRACTICE_CTEST_COMMAND="${CMAKE_CTEST_COMMAND}"
    )

endif()

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    add_executable(practice_protocol_test tests/sip_protocol_test.cpp)
    target_link_libraries(practice_protocol_test PRIVATE practice_core GTest::gtest_main)
    gtest_discover_tests(practice_protocol_test PROPERTIES LABELS protocol)

    add_executable(practice_framework_test tests/framework_test.cpp)
    target_link_libraries(practice_framework_test PRIVATE practice_core)
    add_test(NAME practice_framework COMMAND practice_framework_test)
    add_test(NAME practice_cli_list COMMAND cpp_rtc_practice --list)
    add_test(NAME practice_cli_info COMMAND cpp_rtc_practice --info 01)
    add_test(NAME practice_cli_unknown COMMAND cpp_rtc_practice --info 99)
    set_tests_properties(practice_cli_unknown PROPERTIES WILL_FAIL TRUE)

    add_executable(practice_acceptance_test tests/acceptance_test.cpp)
    target_link_libraries(practice_acceptance_test PRIVATE practice_core GTest::gtest_main)
    gtest_discover_tests(practice_acceptance_test PROPERTIES LABELS acceptance)

    if(PRACTICE_BUILD_WEB_SERVER)
        add_executable(practice_web_test tests/web_test.cpp)
        target_link_libraries(practice_web_test PRIVATE practice_web_core GTest::gtest_main)
        target_compile_definitions(practice_web_test PRIVATE
            PRACTICE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        )
        gtest_discover_tests(practice_web_test)
    endif()
endif()
