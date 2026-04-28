idf_component_register(SRCS "SquareLine_cb.c" 
                       INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}/main/ui"
                       REQUIRES driver lvgl esp_lvgl_port aw9523)