#pragma once
#define MAX_NR_OF_CITIES 5

typedef struct City {
  bool exists;
  int condition; // icon
  int temperature;
  int id; // cityid
  char name[1][30]; 
} City;


int main_window_save_cities();
int main_window_load_cities();
  
void main_window_create();
void main_window_destroy();
Window *main_window_get_window();