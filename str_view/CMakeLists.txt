add_library(${PROJECT_NAME})
add_library(${namespace}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# add sources to the library target
target_sources(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/source/${PROJECT_NAME}.c
    PUBLIC 
        FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}
            FILES ${PROJECT_NAME}.h
)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
if (BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SV_BUILD_DLL=1)
endif()

# set properties for the target. VERSION set the library version to the project
# version * SOVERSION set the compatibility  version for the library to the
# major number of the version
# note that ${public_headers} should be in quotes
set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        RELEASE_POSTFIX "_release"
        DEBUG_POSTFIX "_debug"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        C_VISIBILITY_PRESET "default"
        VISIBILITY_INLINES_HIDDEN TRUE
)

include(GenerateExportHeader)

# this command generates a header file in the CMAKE_CURRENT_BINARY_DIR which
# sets the visibility attributes according to the compiler settings
generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME export/export_${PROJECT_NAME}.h
)

# Add CMAKE_CURRENT_BINARY_DIR to the include path so the generated header can
# be found
target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
