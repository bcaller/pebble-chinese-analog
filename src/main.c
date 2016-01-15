#include <pebble.h>
#include "characters.h"

static Window *main_window;
static Layer *time_layer;
static GFont font_numbers;

static void tick_minute_handler(struct tm *tick_time, TimeUnits units_changed) {
	// Write the current hours and minutes into a buffer
    static char time_buffer[2];
    time_buffer[1] = '\0';
    time_buffer[0] = chinese_numbers[tick_time->tm_hour % 12];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour is: %s", time_buffer);
}

static GRect margin_bounds(GRect bounds, int margin_x, int margin_y) {
    return (GRect) {
        .origin = {bounds.origin.x + margin_x, bounds.origin.y + margin_y},
        .size = {bounds.size.w - margin_x*2, bounds.size.h - margin_y * 2}
    };
}

static void draw_numbers(struct Layer *layer, GContext *ctx) {
    GRect bounds = margin_bounds(layer_get_bounds(layer), 14, 16);
    graphics_context_set_text_color(ctx, GColorYellow);
    for(int i=0; i<12; i++) {
        GRect rect = grect_centered_from_polar(bounds, GOvalScaleModeFitCircle, TRIG_MAX_ANGLE*i/12, (GSize){32,35});
        graphics_draw_text(ctx, chinese_numbers_null+(i*2), font_numbers, rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}

static void main_window_load(Window *window) {  
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    font_numbers = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FZKATJW_SIMP_32));
	
	//Make layer
	time_layer = layer_create(bounds);
	layer_set_update_proc(time_layer, draw_numbers);
    
	//Add layer
	layer_add_child(window_layer, time_layer);
    
    window_set_background_color(window, GColorDarkCandyAppleRed);
}

static void main_window_unload(Window *w) {
    tick_timer_service_unsubscribe();
    layer_destroy(time_layer);
    fonts_unload_custom_font(font_numbers);
}

static void init() {
    // Create main Window element and assign to pointer
    main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
    });

    // Show the Window on the watch, with animated=true
    window_stack_push(main_window, true);

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_minute_handler);
}

static void deinit() {
    window_destroy(main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}