# Embed binary resource as C byte array
# Usage: embed_resource(VAR_NAME path/to/file)
# Sets ${VAR_NAME}_OUTPUT to the generated source file path
function(embed_resource VAR_NAME INPUT_FILE)
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${VAR_NAME}_data.cpp")
    set(HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${VAR_NAME}_data.h")

    # Convert path separators
    file(TO_CMAKE_PATH "${INPUT_FILE}" INPUT_FILE_CMAKE)

    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND} -DINPUT_FILE="${INPUT_FILE_CMAKE}"
                -DOUTPUT_FILE="${OUTPUT_FILE}"
                -DHEADER_FILE="${HEADER_FILE}"
                -DVAR_NAME="${VAR_NAME}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bin2c.cmake"
        DEPENDS "${INPUT_FILE_CMAKE}"
        COMMENT "Embedding resource: ${INPUT_FILE_CMAKE}"
    )

    set(${VAR_NAME}_OUTPUT "${OUTPUT_FILE}" PARENT_SCOPE)
endfunction()
