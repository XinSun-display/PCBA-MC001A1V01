file(GLOB_RECURSE SRC_UI "${CMAKE_SOURCE_DIR}/main/ui/*.c")
set(srcs "display.c" ${SRC_UI})
set(includes "."  "${IDF_PATH}/components/esp_lcd/include" "${IDF_PATH}/components/esp_lcd/interface" "${CMAKE_SOURCE_DIR}/main/ui")
set(priv_requires "esp_mm" "esp_psram" "esp_timer" "esp_lcd" "ft5x46")

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES ${priv_requires}
                       INCLUDE_DIRS  ${includes}
                       REQUIRES driver lvgl esp_lvgl_port aw9523)