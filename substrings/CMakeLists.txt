set(PROJECT_NAME substrings)

set(CMAKE_CXX_STANDARD 20)

################################################################################
# Source groups
################################################################################
set(Header_files
    "EntropyCache.hpp"
    "FastLog2.hpp"
    "Matcher.hpp"
    "Substrings.hpp"
    "timeit.hpp"
)
source_group("Header files" FILES ${Header_files})

set(Source_files
    "EntropyCache.cpp"
    "main.cpp"
    "Matcher.cpp"
    "Substrings.cpp"
)
source_group("Source files" FILES ${Source_files})

set(ALL_FILES
    ${Header_files}
    ${Source_files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE substrings)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdpts/parallel-hashmap/parallel_hashmap;"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdpts/difflib/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdpts/cxxopts/include;"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        "UNICODE;"
        "_UNICODE"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG;"
        "TIME_IT;"
        "_MBCS"
    ">"
    "_CONSOLE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /O2;
            /Ob2;
            /Oi;
            /GF;
            /GT;
            /arch:AVX2;
            /Oy;
            /Gy;
            /Ot;
            /fp:fast;
            /GS-
        >
        /permissive-;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /DEBUG
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )
endif()

find_package(absl CONFIG REQUIRED)

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "absl::base;"
    "absl::stacktrace;"
    "absl::symbolize;"
    "absl::malloc_internal;"
    "absl::log_internal_conditions;"
    "absl::throw_delegate;"
    "absl::raw_logging_internal;"
    "absl::int128;"
    "absl::strings;"
    "absl::string_view;"
    "absl::strings_internal;"
    "absl::time_zone;"
    "absl::time"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
