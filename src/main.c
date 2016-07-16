#include <pebble.h>
#include "splash_window.h"
#include "main_window.h"

void launch_main_window(void *data) {
  window_stack_push(main_window_get_window(), true); // animated
}

int main() {
  splash_window_create();
  main_window_create();
  window_stack_push(splash_window_get_window(), true); // animated
  AppTimer *timer = app_timer_register(1000, launch_main_window, NULL);
  app_event_loop();
  splash_window_destroy();
}