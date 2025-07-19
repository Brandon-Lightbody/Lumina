# Converts binary file to C header
function(embed_binary input output)
    file(READ ${input} hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex ${hex})
    string(REGEX REPLACE ",$" "" hex ${hex})
    
    get_filename_component(name ${input} NAME_WE)
    set(array_name "${name}_bytes")
    
    file(WRITE ${output} "#pragma once\n\n")
    file(APPEND ${output} "namespace Lumina::Embedded {\n")
    file(APPEND ${output} "    extern const unsigned char ${array_name}[] = {${hex}};\n")
    file(APPEND ${output} "    extern const size_t ${name}_length = sizeof(${array_name});\n")
    file(APPEND ${output} "}\n")
endfunction()

embed_binary(${INPUT_FILE} ${OUTPUT_FILE})