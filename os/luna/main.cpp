#include <sys/system.h>
#include <luna/engine.h>
#include <luna/graphics.h>
#include <lvgl.h>
#include <drivers/timer.h>
#include <eastl/unique_ptr.h>
#include <eastl/allocator_malloc.h>

#include <src/display/lv_display_private.h>

static void disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    Graphics* gfx = (Graphics*)disp->user_data;
    uint32_t w = gfx->getWidth();
    uint32_t h = gfx->getHeight();
    uint32_t bpp = gfx->getBitsPerPixel();

    uint32_t* back_buffer = static_cast<uint32_t*>(gfx->getBackBuffer());
    uint32_t* src = (uint32_t*)px_map;

    memcpy((void*)back_buffer, (void*)src, w * h * (bpp / 8));
    lv_display_flush_ready(disp);
}

static uint32_t tick_get() {
    return ticks(); // Return milliseconds
}

// Stub callbacks for window buttons
static void min_cb(lv_event_t* e) {
    // Stub action for minimize
}

static void max_cb(lv_event_t* e) {
    // Stub action for maximize
}

static void close_cb(lv_event_t* e) {
    // Stub action for close
}

int main(int argc, char** argv) {
    // eastl::allocator_malloc ma;
    eastl::unique_ptr<Graphics> gfx = 
    eastl::make_unique<Graphics>();

    lv_init();
    lv_tick_set_cb(tick_get);

    uint32_t w = gfx->getWidth();
    uint32_t h = gfx->getHeight();

    lv_display_t* disp = lv_display_create(w, h);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
    disp->user_data = gfx.get();

    const size_t buf_size = (w * h) * sizeof(lv_color32_t);
    lv_color32_t* draw_buf = static_cast<lv_color32_t*>(malloc(buf_size));
    lv_display_set_buffers(disp, draw_buf, nullptr, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, disp_flush);

    // Create window with title
    lv_obj_t* win = lv_win_create(lv_screen_active()); 
    lv_win_add_title(win, "Luna Example Window");

    // Add buttons (order: close, maximize, minimize to appear left-to-right as min, max, close)
    lv_obj_t* min_btn = lv_win_add_button(win, LV_SYMBOL_MINUS, 44);
    lv_obj_t* max_btn = lv_win_add_button(win, LV_SYMBOL_PLUS, 44);
    lv_obj_t* close_btn = lv_win_add_button(win, LV_SYMBOL_CLOSE, 44);

    // Add content to the window
    lv_obj_t* content = lv_win_get_content(win);
    lv_obj_t* label = lv_label_create(content);
    lv_label_set_text(label, "Hello, I'm Luna's Graphic User Interface!");
    lv_obj_center(label);

    while (1) {
        lv_timer_handler();
        gfx->swapBuffers();
    }

    free(draw_buf);
    return 0;
}