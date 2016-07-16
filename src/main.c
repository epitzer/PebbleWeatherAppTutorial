#include <pebble.h>
#include "splash_window.h"
#include "main_window.h"
#include "error_window.h"

void launch_main_window(void *data) {
  window_stack_push(main_window_get_window(), true); // animated
}

void log_mem(const char *message) {
  size_t free_mem = heap_bytes_free();
  size_t used_mem = heap_bytes_used();
  APP_LOG(APP_LOG_LEVEL_INFO,
          "%s: %u bytes free, %u bytes used",
          message,
          (unsigned int)free_mem,
          (unsigned int)used_mem);
}

int main() {
  log_mem("app started");
  main_window_load_cities();
  
  splash_window_create();
  main_window_create();
  error_window_create();
  
  window_stack_push(splash_window_get_window(), true); // animated
  AppTimer *timer = app_timer_register(1500, launch_main_window, NULL);
  
  app_event_loop();
  splash_window_destroy();
  main_window_destroy();
  error_window_destroy();
  
  log_mem("app closing");
}