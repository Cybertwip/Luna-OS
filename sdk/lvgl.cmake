# Copy lvgl top directory headers (non-recursive)
file(COPY ../external/lvgl/lv_version.h
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl
)
file(COPY ../external/lvgl/lvgl_private.h
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl
)
file(COPY ../external/lvgl/lvgl.h
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl
)

file(COPY ../external/lvgl/lvgl.h
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl
)

file(COPY ../os/lv_conf.h
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl
)

# Copy lvgl/src headers recursively
file(COPY ../external/lvgl/src/
     DESTINATION ${CMAKE_BINARY_DIR}/sdk/include/lvgl/src
     FILES_MATCHING
     PATTERN "*.h"
)