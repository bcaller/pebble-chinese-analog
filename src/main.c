#include <pebble.h>
#include "characters.h"

static Window *main_window;
static Layer *number_layer;
static Layer *hand_layer;
static GFont font_numbers;

static GPath* hand_path_min;
static GPath* hand_path_hr;
static const GPathInfo HAND_PATHINFO_MIN = {
        4,
        (GPoint[]) {{-4, 0}, {0, 4}, {4, 0}, {0, -50}}
};
static const GPathInfo HAND_PATHINFO_HR = {
        4,
        (GPoint[]) {{-4, 0}, {0, 4}, {4, 0}, {0, -35}}
};

static void tick_minute_handler(struct tm *tick_time, TimeUnits units_changed) {
	// Write the current hours and minutes into a buffer
    static char time_buffer[2];
    time_buffer[1] = '\0';
    time_buffer[0] = chinese_numbers[tick_time->tm_hour % 12];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour is: %s", time_buffer);
    
    gpath_rotate_to(hand_path_hr, TRIG_MAX_ANGLE * ((tick_time->tm_hour % 12)*60 + tick_time->tm_min) / (12 * 60));
    gpath_rotate_to(hand_path_min, TRIG_MAX_ANGLE * tick_time->tm_min / 60);
    layer_mark_dirty(hand_layer);
}

static GRect margin_bounds(GRect bounds, int margin_x, int margin_y) {
    return (GRect) {
        .origin = {bounds.origin.x + margin_x, bounds.origin.y + margin_y},
        .size = {bounds.size.w - margin_x*2, bounds.size.h - margin_y * 2}
    };
}

static GRect move_origin(GRect bounds, int margin_x, int margin_y) {
    return (GRect) {
        .origin = {bounds.origin.x + margin_x, bounds.origin.y + margin_y},
        .size = bounds.size
    };
}

static void draw_hands(struct Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 4);
    gpath_draw_outline(ctx, hand_path_hr);
    gpath_draw_outline(ctx, hand_path_min);
    gpath_draw_filled(ctx, hand_path_min);
    gpath_draw_filled(ctx, hand_path_hr);
}

static void draw_numbers(struct Layer *layer, GContext *ctx) {
    GRect bounds = margin_bounds(layer_get_bounds(layer), 14, PBL_IF_ROUND_ELSE(21,18));
    graphics_context_set_text_color(ctx, GColorYellow);
    for(int i=0; i<12; i++) {
        #ifndef PBL_ROUND
        GRect shifted; //A little bit less round on RECT displays
        if(i == 1 || i == 11) shifted = move_origin(bounds, i==1?2:-2, -2);
        else if(i == 2 || i == 4) shifted = move_origin(bounds, 2, i==4?2:-2);
        else if(i == 5 || i == 7) shifted = move_origin(bounds, i==5?2:-2, 2);
        else if(i == 8 || i == 10) shifted = move_origin(bounds, -2, i==10?2:-2);
        else shifted = bounds;
        #endif
        GRect rect = grect_centered_from_polar(PBL_IF_ROUND_ELSE(bounds, shifted), GOvalScaleModeFitCircle, TRIG_MAX_ANGLE*i/12, (GSize){32,35});
        graphics_draw_text(ctx, chinese_daxie_null+(i*2), font_numbers, rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}

static void main_window_load(Window *window) {  
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    font_numbers = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FZKATJW_31));
	
	//Make layer
	number_layer = layer_create(bounds);
	layer_set_update_proc(number_layer, draw_numbers);
    
    hand_layer = layer_create(bounds);
	layer_set_update_proc(hand_layer, draw_hands);
    
    hand_path_hr = gpath_create(&HAND_PATHINFO_HR);
    hand_path_min = gpath_create(&HAND_PATHINFO_MIN);
    gpath_move_to(hand_path_min, (GPoint){bounds.size.w/2, bounds.size.h/2});
    gpath_move_to(hand_path_hr, (GPoint){bounds.size.w/2, bounds.size.h/2});
    
	//Add layer
	layer_add_child(window_layer, hand_layer);
    layer_add_child(window_layer, number_layer);
    
    window_set_background_color(window, GColorDarkCandyAppleRed);
}

static void main_window_unload(Window *w) {
    tick_timer_service_unsubscribe();
    layer_destroy(number_layer);
    layer_destroy(hand_layer);
    gpath_destroy(hand_path_min);
    gpath_destroy(hand_path_hr);
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