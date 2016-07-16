#include <pebble.h>
#include "error_window.h"

Window *errorWindow;
Layer *errorGraphicsLayer;
char currentErrorText[1][20]; // WAT?

void error_window_set_error(char *errorText) {
  strncpy(currentErrorText[0], errorText, sizeof(currentErrorText[0])); // WAT?
  if (errorGraphicsLayer != NULL) {
    layer_mark_dirty(errorGraphicsLayer);
  }
}

void error_window_show() {
  window_stack_push(errorWindow, true);
}

void error_graphics_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_text_color(ctx, GColorBulgarianRose);
  graphics_draw_text(ctx,
                     currentErrorText[0],
                     fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                     GRect(0, 0, 144, 50),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);
  
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_circle(ctx, GPoint(72, 109), 59);
  uint16_t cx = 144/2, cy = (168-50)/2+50, offset = 41.7193;
  graphics_draw_line(ctx, GPoint(cx-offset, cy+offset), GPoint(cx+offset, cy-offset));
  graphics_draw_line(ctx, GPoint(cx-offset, cy-offset), GPoint(cx+offset, cy+offset));
}

void error_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  errorGraphicsLayer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(errorGraphicsLayer, error_graphics_proc);
  layer_add_child(window_layer, errorGraphicsLayer); // WHY not get layer this time?
}

void error_window_unload(Window *window) {
  layer_destroy(errorGraphicsLayer);
  errorGraphicsLayer = NULL;
}

void error_window_create() {
  errorWindow = window_create();
  window_set_window_handlers(errorWindow, (WindowHandlers) {
    .load = error_window_load,
    .unload = error_window_unload
  });
}

void error_window_destroy() {
  window_destroy(errorWindow);
}

Window *error_window_get_window() {
  return errorWindow;
}