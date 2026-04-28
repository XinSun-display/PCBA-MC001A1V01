SET(SOURCES screens/ui_Screen1.c
    screens/ui_GPIO.c
    screens/ui_imgView.c
    screens/ui_backlight.c
    screens/ui_canClass.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_1079216036.c
    fonts/ui_font_tw1616.c)

add_library(ui ${SOURCES})
