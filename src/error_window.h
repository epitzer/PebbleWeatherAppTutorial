#pragma once

void error_window_show();
void error_window_set_error(char *errorText);
void error_window_load(Window *window);
void error_window_unload(Window *window);
void error_window_create();
void error_window_destroy();
Window *error_window_get_window();
