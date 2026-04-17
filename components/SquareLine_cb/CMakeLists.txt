idf_component_register(SRCS "SquareLine_cb.c" 
                       INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}/main/ui"
                       REQUIRES driver lvgl aw9523)