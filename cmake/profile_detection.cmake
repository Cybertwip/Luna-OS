# Function to load platform and ISA-specific sources and flags
function(LOAD_PROFILE ISA PLATFORM)
    # Validate inputs
    if(NOT ISA)
        message(FATAL_ERROR "ISA must be specified.")
    endif()
    if(NOT PLATFORM)
        message(FATAL_ERROR "PLATFORM must be specified.")
    endif()

    # Define paths for ISA and platform
    set(ISA_DIR "${CMAKE_SOURCE_DIR}/kernel/arch/${ISA}")
    set(PLATFORM_DIR "${ISA_DIR}/${PLATFORM}")

    # Check if directories exist
    if(NOT EXISTS "${ISA_DIR}")
        message(FATAL_ERROR "ISA directory not found: ${ISA_DIR}")
    endif()
    if(NOT EXISTS "${PLATFORM_DIR}")
        message(FATAL_ERROR "Platform directory not found: ${PLATFORM_DIR}")
    endif()

    # Gather source files for the ISA and platform
    file(GLOB ISA_SOURCES
        "${ISA_DIR}/*.c"
        "${ISA_DIR}/*.[sS]"
    )
    file(GLOB PLATFORM_SOURCES
        "${PLATFORM_DIR}/*.c"
        "${PLATFORM_DIR}/*.[sS]"
    )

    # Load ISA and platform-specific flags
    set(ISA_FLAGS_FILE "${ISA_DIR}/flags.cmake")
    set(PLATFORM_FLAGS_FILE "${PLATFORM_DIR}/flags.cmake")

    if(EXISTS "${ISA_FLAGS_FILE}")
        include("${ISA_FLAGS_FILE}")
    else()
        message(WARNING "ISA flags file not found: ${ISA_FLAGS_FILE}")
    endif()

    if(EXISTS "${PLATFORM_FLAGS_FILE}")
        include("${PLATFORM_FLAGS_FILE}")
    else()
        message(WARNING "Platform flags file not found: ${PLATFORM_FLAGS_FILE}")
    endif()

    # Set the linker script path
    set(PLATFORM_LINKER_SCRIPT "${PLATFORM_DIR}/linker.ld")
    if(NOT EXISTS "${PLATFORM_LINKER_SCRIPT}")
        message(FATAL_ERROR "Linker script not found: ${PLATFORM_LINKER_SCRIPT}")
    endif()

    # Export variables to the parent scope
    set(PLATFORM_LAYOUT "${PLATFORM_LINKER_SCRIPT}" PARENT_SCOPE)
    set(ISA_SRCS "${ISA_SOURCES}" PARENT_SCOPE)
    set(PLATFORM_SRCS "${PLATFORM_SOURCES}" PARENT_SCOPE)
    set(ISA_C_FLAGS "${ISA_C_FLAGS}" PARENT_SCOPE)
    set(ISA_ASM_FLAGS "${ISA_ASM_FLAGS}" PARENT_SCOPE)
endfunction()