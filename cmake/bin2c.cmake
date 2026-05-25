# bin2c.cmake - Convert binary file to C byte array
# Called by embed_resource.cmake

file(READ "${INPUT_FILE}" FILE_CONTENT HEX)
string(LENGTH "${FILE_CONTENT}" FILE_SIZE_HEX)
math(EXPR FILE_SIZE "${FILE_SIZE_HEX} / 2")

set(C_NAME "${VAR_NAME}")

# Generate header
file(WRITE "${HEADER_FILE}" "#pragma once\n\nextern const unsigned char ${C_NAME}[];\nextern const unsigned int ${C_NAME}_size;\n")

# Generate source
file(WRITE "${OUTPUT_FILE}" "#include \"${C_NAME}_data.h\"\n\nconst unsigned char ${C_NAME}[] = {\n")

set(COUNTER 0)
string(LENGTH "${FILE_CONTENT}" CONTENT_LEN)
math(EXPR CONTENT_LEN "${CONTENT_LEN} - 1")

set(LINE "")
foreach(IDX RANGE 0 ${CONTENT_LEN} 2)
    string(SUBSTRING "${FILE_CONTENT}" ${IDX} 2 BYTE)
    if(COUNTER GREATER 0)
        string(APPEND LINE ",")
    endif()
    string(APPEND LINE "0x${BYTE}")
    math(EXPR COUNTER "${COUNTER} + 1")

    if(COUNTER EQUAL 16)
        file(APPEND "${OUTPUT_FILE}" "    ${LINE}\n")
        set(LINE "")
        set(COUNTER 0)
    endif()
endforeach()

if(NOT "${LINE}" STREQUAL "")
    file(APPEND "${OUTPUT_FILE}" "    ${LINE}\n")
endif()

file(APPEND "${OUTPUT_FILE}" "};\n\nconst unsigned int ${C_NAME}_size = ${FILE_SIZE};\n")
