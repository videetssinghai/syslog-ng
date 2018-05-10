set(testdst_HEADERS
    "testdst-parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/testdst-grammar.h"
)

set(testdst_SOURCES
    "testdst-plugin.c"
    "testdst-parser.c"
    "${CMAKE_CURRENT_BINARY_DIR}/testdst-grammar.c"
)

generate_y_from_ym(modules/testdst/testdst-grammar)

bison_target(testdstGrammar
    ${CMAKE_CURRENT_BINARY_DIR}/testdst-grammar.y
    ${CMAKE_CURRENT_BINARY_DIR}/testdst-grammar.c
COMPILE_FLAGS ${BISON_FLAGS})

option(ENABLE_testdst "Enable testdst ON")

if (ENABLE_testdst)
  add_library(testdst MODULE ${testdst_SOURCES})
  target_include_directories(testdst
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(testdst PRIVATE syslog-ng)

  install(TARGETS testdst
  LIBRARY DESTINATION lib/syslog-ng/
  COMPONENT testdst)
endif()
