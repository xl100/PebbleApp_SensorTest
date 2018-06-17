#include <pebble.h>

#define KEY_BUTTON    0
#define KEY_VIBRATE   1

#define BUTTON_UP     0
#define BUTTON_SELECT 1
#define BUTTON_DOWN   2

static Window *s_main_window;
static TextLayer *s_text_layer;

/********************************* AppMessage *********************************/

static void send(int key, int message) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    dict_write_int(iter, key, &message, sizeof(int), true);
    
    app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
    // Get the first pair
    Tuple *t = dict_read_first(iterator);
    
    // Process all pairs present
    while(t != NULL) {
        // Process this pair's key
        switch(t->key) {
            case KEY_VIBRATE:
                // Trigger vibration
                text_layer_set_text(s_text_layer, "Vibrate!");
                vibes_short_pulse();
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_INFO, "Unknown key: %d", (int)t->key);
            break;
        }
        
        // Get next pair, if any
        t = dict_read_next(iterator);
    }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}

static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/********************************* Buttons ************************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    text_layer_set_text(s_text_layer, "Select");
    
    send(KEY_BUTTON, BUTTON_SELECT);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    text_layer_set_text(s_text_layer, "Up");
    
    send(KEY_BUTTON, BUTTON_UP);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    text_layer_set_text(s_text_layer, "Down");
    
    send(KEY_BUTTON, BUTTON_DOWN);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/***************************** Main_window ********************************/

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    // Create main TextLayer
    s_text_layer = text_layer_create(bounds);
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text(s_text_layer, "Open Android app and press any button");
    text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
#ifdef PBL_ROUND
    text_layer_enable_screen_text_flow_and_paging(s_text_layer, 5);
#endif
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init(void) {
    // Register collbacks
    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_inbox_dropped(inbox_dropped_handler);
    app_message_register_outbox_failed(outbox_failed_handler);
    app_message_register_outbox_sent(outbox_sent_handler);
    
    // Open AppMessage
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    
    // Create main Window
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(s_main_window, animated);
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);

  app_event_loop();
  deinit();
}