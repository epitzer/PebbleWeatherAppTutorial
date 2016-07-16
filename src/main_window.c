#include <pebble.h>
#include "main_window.h"
#include "error_window.h"

City cities[MAX_NR_OF_CITIES];
int currentCityToWrite = -1;

Window *mainWindow;
MenuLayer *mainMenuLayer;

uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 2;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return 1;
    case 1:
      return 1;
    default:
      return 0;
  }
}

int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Cities");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Config");
      break;
  }
}

void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Demo City", "40°C", NULL); // null icon
          break;
      }
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Add City", NULL, NULL);
      break;
  }
}

char *get_readable_dictation_status(DictationSessionStatus status) {
  switch(status){
    case DictationSessionStatusSuccess:
      return "Success";
    case DictationSessionStatusFailureTranscriptionRejected:
      return "User rejected success";
    case DictationSessionStatusFailureTranscriptionRejectedWithError:
      return "User rejected error";
    case DictationSessionStatusFailureSystemAborted:
      return "Too many errors, UI gave up";
    case DictationSessionStatusFailureNoSpeechDetected:
      return "No speech, UI exited";
    case DictationSessionStatusFailureConnectivityError:
      return "No BT/internet connection";
    case DictationSessionStatusFailureDisabled:
      return "Voice dictation disabled";
    case DictationSessionStatusFailureInternalError:
      return "Internal error";
    case DictationSessionStatusFailureRecognizerError:
      return "Failed to transcribe speech";
  }
  return "Unknown";
}

void send_weather_request(char *city) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is null! Refusing to send");
    return;
  }
  
  dict_write_cstring(iter, MESSAGE_KEY_getWeather, city);
  dict_write_end(iter);
  
  app_message_outbox_send();
}

void dictation_session_callback(DictationSession *session,
                                DictationSessionStatus status,
                                char *transcription,
                                void *context) {
  switch (status) {
    case DictationSessionStatusSuccess:
      send_weather_request(transcription);
      break;
    case DictationSessionStatusFailureTranscriptionRejected:
      /* user cancled -> ignore */
      break;
    default:
      error_window_set_error(get_readable_dictation_status(status));
      error_window_show();
      break;
  }
}

void launch_dictation() {
  static char last_text[40];
  DictationSession *session =
    dictation_session_create(sizeof(last_text),
                             dictation_session_callback,
                             NULL); // context
  if (session == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Dictation session is null! Are you running aplite?");
    return;
  }
  dictation_session_start(session);
  // TODO: dictation session cleanup?
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0: break;
    case 1: launch_dictation(); break;
    default:
      error_window_set_error("Unknown menu item selected");
      error_window_show();
      break;
  }
}

void process_tuple(Tuple *t){
  uint32_t key = t->key;
  int value = t->value->int32; // make sure you get the right member according to type
  
  if (key == MESSAGE_KEY_icon) {
    cities[currentCityToWrite].condition = value;
    
  } else if (key == MESSAGE_KEY_temperature) {
    cities[currentCityToWrite].temperature = value;
    
  } else if (key == MESSAGE_KEY_cityid) {
    cities[currentCityToWrite].id = value;
    
  } else if (key == MESSAGE_KEY_cityname) {
    strncpy(cities[currentCityToWrite].name[0],
            t->value->cstring,
            sizeof(cities[currentCityToWrite]));
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Got key %d with value %d", (int)key, value);
}

void message_inbox(DictionaryIterator *iter, void *context){
  for (int i = 0; i<MAX_NR_OF_CITIES; i++) {
    if (!cities[i].exists) {
      currentCityToWrite = i;
      break;
    }
  }
  
  Tuple *t = dict_read_first(iter);
  while (t != NULL) {
    process_tuple(t);
    t = dict_read_next(iter);
  }
  
  cities[currentCityToWrite].exists = true;
}

void message_inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message dropped, reason %d.", reason);
  switch(reason) {
    case APP_MSG_OK:
      APP_LOG(APP_LOG_LEVEL_INFO, "(0) All good, operation was successful.");
      break;
    case APP_MSG_SEND_TIMEOUT:
      APP_LOG(APP_LOG_LEVEL_INFO, "(2) The other end did not confirm receiving the sent data with an (n)ack in time.");
      break;
    case APP_MSG_SEND_REJECTED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(4) The other end rejected the sent data, with a 'nack' reply.");
      break;
    case APP_MSG_NOT_CONNECTED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(8) The other end was not connected.");
      break;
    case APP_MSG_APP_NOT_RUNNING:
      APP_LOG(APP_LOG_LEVEL_INFO, "(16) The local application was not running.");
      break;
    case APP_MSG_INVALID_ARGS:
      APP_LOG(APP_LOG_LEVEL_INFO, "(32) The function was called with invalid arguments."); 
      break;
    case APP_MSG_BUSY:
      APP_LOG(APP_LOG_LEVEL_INFO, "(64) There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent."); 
      break;
    case APP_MSG_BUFFER_OVERFLOW:
      APP_LOG(APP_LOG_LEVEL_INFO, "(128) The buffer was too small to contain the incoming message."); 
      break;
    case APP_MSG_ALREADY_RELEASED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(512) The resource had already been released."); 
      break;
    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(1024) The callback was already registered."); 
      break;
    case APP_MSG_CALLBACK_NOT_REGISTERED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(2048) The callback could not be deregistered, because it had not been registered before."); 
      break;
    case APP_MSG_OUT_OF_MEMORY:
      APP_LOG(APP_LOG_LEVEL_INFO, "(4096) The system did not have sufficient application memory to perform the requested operation."); 
      break;
    case APP_MSG_CLOSED:
      APP_LOG(APP_LOG_LEVEL_INFO, "(8192) App message was closed."); 
      break;
    case APP_MSG_INTERNAL_ERROR:
      APP_LOG(APP_LOG_LEVEL_INFO, "(16384) An internal OS error prevented AppMessage from completing an operation."); 
      break;
    case APP_MSG_INVALID_STATE:
      APP_LOG(APP_LOG_LEVEL_INFO, "(32768) The function  was called while App Message was not in the appropriate state."); 
      break;
   }
}

void setup_menu_layer(Window *window) {
	Layer *window_layer = window_get_root_layer(window);

    mainMenuLayer = menu_layer_create(GRect(0, 0, 144, 168));
    menu_layer_set_callbacks(mainMenuLayer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });

    menu_layer_set_click_config_onto_window(mainMenuLayer, window);

    layer_add_child(window_layer, menu_layer_get_layer(mainMenuLayer));
}

void main_window_load(Window *window) {
  setup_menu_layer(window);
  
  app_message_register_inbox_received(message_inbox);
  app_message_register_inbox_dropped(message_inbox_dropped);
  app_message_open(256, 256);
}

void main_window_unload(Window *window) {
  menu_layer_destroy(mainMenuLayer);
}

void main_window_create() {
  mainWindow = window_create();
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
}

void main_window_destroy() {
  window_destroy(mainWindow);
}

Window *main_window_get_window() {
  return mainWindow;
}