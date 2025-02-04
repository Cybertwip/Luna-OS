#include <luna/engine.h>
#include <luna/graphics.h>
#include <lvgl.h>
#include <eastl/unique_ptr.h>
#include <eastl/allocator_malloc.h>

#include <sys/time.h>

#include <src/display/lv_display_private.h>
#include <src/misc/lv_event_private.h>

extern "C" {
    void wait(dword_t milliseconds);
    extern dword_t mouse_cursor_x;
    extern dword_t mouse_cursor_y;
    extern byte_t mouse_click_button_state;
    #define MOUSE_CLICK 1
}

// Double buffering support
static lv_color32_t* draw_buf1 = nullptr;
static lv_color32_t* draw_buf2 = nullptr;

static void disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    Graphics* gfx = (Graphics*)disp->user_data;

    uint32_t w = gfx->getWidth();
    uint32_t h = gfx->getHeight();
    uint32_t bpp = gfx->getBitsPerPixel();

    uint32_t* back_buffer = static_cast<uint32_t*>(gfx->getBackBuffer());
    uint32_t* src = (uint32_t*)px_map;

    // Copy only the dirty area
    for(int y = area->y1; y <= area->y2; y++) {
        uint32_t src_offset = (y * w) + area->x1;
        uint32_t dst_offset = (y * w) + area->x1;
        memcpy(&back_buffer[dst_offset], &src[src_offset], (area->x2 - area->x1 + 1) * sizeof(uint32_t));
    }

    lv_display_flush_ready(disp);
}

static uint32_t tick_get() {
    return mtime();
}

static void close_cb(lv_event_t* e) {
    lv_obj_t* win = (lv_obj_t*)e->user_data;
    lv_obj_del(win);
}
static void create_background() {
    lv_obj_t* desktop = lv_screen_active();
    
    // Night sky gradient
    lv_obj_set_style_bg_opa(desktop, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(desktop, lv_color_hex(0x0F0C29), 0);

    // Moon glow effect
    lv_obj_t* glow = lv_obj_create(desktop);
    lv_obj_remove_style_all(glow);
    lv_obj_set_size(glow, 250, 250);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(glow, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_10, 0);
    lv_obj_align(glow, LV_ALIGN_TOP_RIGHT, -80, -30);

    // Main moon body
    lv_obj_t* moon = lv_obj_create(desktop);
    lv_obj_remove_style_all(moon);
    lv_obj_set_size(moon, 180, 180);
    lv_obj_set_style_radius(moon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(moon, lv_color_hex(0xF4F6F0), 0);
    lv_obj_set_style_bg_opa(moon, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_color(moon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_shadow_opa(moon, LV_OPA_40, 0);
    lv_obj_set_style_shadow_spread(moon, 20, 0);
    lv_obj_align(moon, LV_ALIGN_TOP_RIGHT, -50, 50);

    // Moon craters
    const lv_point_t craters[] = {
        {30, 40}, {70, 20}, {110, 35}, {40, 90}, {90, 110}
    };
    
    for(int i = 0; i < sizeof(craters)/sizeof(craters[0]); i++) {
        lv_obj_t* crater = lv_obj_create(moon);
        lv_obj_remove_style_all(crater);
        lv_obj_set_size(crater, 25 + (i * 3), 25 + (i * 3));
        lv_obj_set_style_radius(crater, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(crater, lv_color_hex(0xE8EAE3), 0);
        lv_obj_set_style_bg_opa(crater, LV_OPA_COVER, 0);
        lv_obj_set_pos(crater, craters[i].x, craters[i].y);
    }


    lv_obj_t* title = lv_label_create(lv_layer_top());
    lv_label_set_text(title, "LUNA OS");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_opa(title, LV_OPA_COVER, 0);
    lv_obj_set_style_transform_zoom(title, 400, 0);  // Initial zoomed state
    
    // Create animation for subtle zoom effect
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, title);
    lv_anim_set_values(&a, 400, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
    lv_anim_start(&a);
    
    // Add text shadow
    lv_obj_t* title_shadow = lv_label_create(lv_layer_top());
    lv_label_set_text(title_shadow, "LUNA OS");
    lv_obj_set_style_text_color(title_shadow, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_opa(title_shadow, LV_OPA_20, 0);
    lv_obj_set_style_text_font(title_shadow, &lv_font_montserrat_14, 0);
    lv_obj_align(title_shadow, LV_ALIGN_CENTER, 3, 3);
    
    // Final alignment
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);
    

    // Add subtitle
    lv_obj_t* subtitle = lv_label_create(lv_layer_top());
    lv_label_set_text(subtitle, "stellar computing experience");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(subtitle, LV_OPA_70, 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, 0);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 60);

}

// Add toolbar creation function
static void create_toolbar() {
    lv_obj_t* toolbar = lv_obj_create(lv_layer_top());
    lv_obj_set_size(toolbar, lv_disp_get_hor_res(NULL), 40);
    lv_obj_align(toolbar, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_set_style_bg_color(toolbar, lv_color_hex(0xA0A0A0), 0);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_COVER, 0);

    // Toolbar style
    lv_obj_set_style_border_width(toolbar, 0, 0);
    lv_obj_set_style_radius(toolbar, 0, 0);
    
    // Toolbar content
    lv_obj_t* btn_container = lv_obj_create(toolbar);
    lv_obj_set_size(btn_container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);

    const char* btns[] = {LV_SYMBOL_HOME, LV_SYMBOL_IMAGE, LV_SYMBOL_SETTINGS, LV_SYMBOL_POWER};
    for(size_t i = 0; i < sizeof(btns)/sizeof(btns[0]); i++) {
        lv_obj_t* btn = lv_btn_create(btn_container);
        lv_obj_set_size(btn, 40, 30);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, btns[i]);
        lv_obj_center(label);
    }
}

static lv_obj_t* create_window() {
    lv_obj_t* win = lv_win_create(lv_layer_top());
    lv_win_add_title(win, "Application Window");
    
    // Window style
    lv_obj_t* close_btn = lv_win_add_button(win, LV_SYMBOL_CLOSE, 44);
    lv_obj_add_event_cb(close_btn, close_cb, LV_EVENT_CLICKED, win);

    // Window size and position
    lv_obj_set_size(win, lv_disp_get_hor_res(NULL) - 40, lv_disp_get_ver_res(NULL) - 60);
    lv_obj_align(win, LV_ALIGN_TOP_LEFT, 20, 20);

    lv_obj_t* content = lv_win_get_content(win);
    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_set_flex_grow(content, 1);

    lv_obj_t* label = lv_label_create(content);
    lv_label_set_text(label, "Welcome to the application!");
    lv_obj_center(label);

    lv_obj_move_foreground(win);
    return win;
}


static uint32_t last_click_time = 0;
static lv_obj_t* last_clicked_obj = NULL;

static void icon_event_cb(lv_event_t* e) {
    if (e->code == LV_EVENT_CLICKED) {
        uint32_t now = lv_tick_get();
        if (last_clicked_obj == e->current_target && (now - last_click_time) < 500) {
            create_window();
            last_clicked_obj = NULL;
            lv_obj_invalidate(lv_screen_active()); // Force redraw
        } else {
            last_click_time = now;
            last_clicked_obj = e->current_target;
        }
    }
}

static void create_desktop_icons() {
    lv_obj_t* desktop = lv_screen_active();

    lv_obj_set_layout(desktop, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(desktop, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(desktop, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(desktop, 20, 0);

    const char* apps[] = {"Text Editor", "Browser", "Settings"};
    for (size_t i = 0; i < sizeof(apps)/sizeof(apps[0]); i++) {
        lv_obj_t* icon_btn = lv_btn_create(desktop);
        lv_obj_set_size(icon_btn, 100, 100);
        lv_obj_add_event_cb(icon_btn, icon_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(icon_btn, lv_color_hex(0xA0A0A0), 0);
        lv_obj_set_style_bg_opa(icon_btn, LV_OPA_COVER, 0);

        lv_obj_t* icon_img = lv_img_create(icon_btn);
        lv_img_set_src(icon_img, LV_SYMBOL_FILE);
        lv_obj_align(icon_img, LV_ALIGN_TOP_MID, 0, 10);

        lv_obj_t* icon_label = lv_label_create(icon_btn);
        lv_label_set_text(icon_label, apps[i]);
        lv_obj_align(icon_label, LV_ALIGN_BOTTOM_MID, 0, -10);


    }
}

static void mouse_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    data->point.x = mouse_cursor_x;
    data->point.y = mouse_cursor_y;
    data->state = (mouse_click_button_state == MOUSE_CLICK) 
                  ? LV_INDEV_STATE_PRESSED 
                  : LV_INDEV_STATE_RELEASED;
}

extern "C" int main(int argc, char** argv) {
    mouse_cursor_x = screen_width / 2;
    mouse_cursor_y = screen_height / 2;

    Graphics gfx;
    lv_init();
    lv_tick_set_cb(tick_get);

    uint32_t w = gfx.getWidth();
    uint32_t h = gfx.getHeight();

    lv_display_t* disp = lv_display_create(w, h);
    disp->user_data = &gfx;

    const size_t buf_size = (w * h) * sizeof(lv_color32_t);
    draw_buf1 = static_cast<lv_color32_t*>(malloc(buf_size));
    draw_buf2 = static_cast<lv_color32_t*>(malloc(buf_size));
    
    lv_display_set_buffers(disp, draw_buf1, draw_buf2, buf_size, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, disp_flush);

    // Set up input device properly for LVGL 9
    lv_indev_t* mouse_indev = lv_indev_create();
    lv_indev_set_type(mouse_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(mouse_indev, mouse_read_cb);

    create_background();
    create_toolbar();
    create_desktop_icons();

    while (1) {
        lv_timer_handler();


        gfx.swapBuffers();
        wait_for_user_input();
        move_mouse_cursor();
    }

    free(draw_buf1);
    free(draw_buf2);
    return 0;
}