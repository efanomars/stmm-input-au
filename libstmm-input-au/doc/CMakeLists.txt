# stmm-input-au: doc/CMakeLists.txt

include(FindDoxygen)

if (DOXYGEN_FOUND)
    if (NOT DOXYGEN_DOT_FOUND) 
        message(WARNING "Graphviz doesn't seem to be installed. Doxygen will not be able to generate graphs. Consider installing this package.")
    endif(NOT DOXYGEN_DOT_FOUND) 

    include(CommonDoxy)
    set(STMMI_TEMP_DOC_LIB_LIST "stmm-input")
    CreateLibDoxy(stmm-input-au  ${STMM_INPUT_AU_VERSION}  "${STMMI_TEMP_DOC_LIB_LIST}")
else(DOXYGEN_FOUND)
    add_custom_target(doc COMMAND echo "Doxygen not found")
endif(DOXYGEN_FOUND)
