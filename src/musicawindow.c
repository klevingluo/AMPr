#include "musicawindow.h"
#include "RTLHelper.h"
#include "GeneralUtils.h"
#include "ToastLayer.h"
#include <inttypes.h>
#include <pebble.h>

static bool bIsPlayingState = false;
static bool bIsVolumeState = false;
static GBitmap *s_res_media_volup_icon;
static GBitmap *s_res_media_voldown_icon;
static ToastLayer *toast = NULL;

// predicted volume level
static int volume_level = 100;
static int desired_volume_level = 100;

// has volume matched yet?
static int volume_matched = 0;

// diagnostic layer
TextLayer *diag_layer;

// number of accelerometer readings per second
uint32_t num_samples = 5;

// some buffer for things
static char buf[] = "____________";

// double keeps track of volume mode:
static double volume_mode = 0.0;

// keeps track if in volume adjust mode
static int volume_adjust = 0;

// double keeps track of volume mode:
static double skip_mode = 0.0;

// 1 if ready for skip, 0 otherwise
static int skip_cooldown = 0;
static int vibe_cooldown = 1;

#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;
#endif

enum AppKeys {
  META_ARTIST_KEY = 0x0,       // TUPLE_CSTRING
  META_TRACK_KEY = 0x1,        // TUPLE_CSTRING
  META_ALBUM_KEY = 0x2,        // TUPLE_CSTRING
  ACTION_PLAYPAUSE_KEY = 0x3,  // TUPLE_INT32
  ACTION_INIT_KEY = 0x4,       // TUPLE_INT32
  ACTION_BACKWARD_KEY = 0x5,   // TUPLE_INT32
  ACTION_FORWARD_KEY = 0x6,    // TUPLE_INT32
  ACTION_VOLDOWN_KEY = 0x7,    // TUPLE_INT32
  ACTION_VOLUP_KEY = 0x8,      // TUPLE_INT32
  META_EXTRA_ARTIST_KEY = 0x9, // TUPLE_CSTRING
  META_EXTRA_TRACK_KEY = 0xA,  // TUPLE_CSTRING
  META_EXTRA_ALBUM_KEY = 0xB,  // TUPLE_CSTRING
  META_VOLSTATE_KEY = 0xC,     // TUPLE_INT32
};

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_24;
static GFont s_res_gothic_28_bold;
static GBitmap *s_res_media_backward_icon;
static GBitmap *s_res_media_playpause_icon;
static GBitmap *s_res_media_forward_icon;
static TextLayer *s_AlbumTextLayer;
static TextLayer *s_TrackTextLayer;
static TextLayer *s_ArtistTextLayer;
static TextLayer *s_RTL_AlbumSecondTextLayer;
static TextLayer *s_RTL_AlbumFirstTextLayer;
static TextLayer *s_RTL_TrackSecondTextLayer;
static TextLayer *s_RTL_TrackFirstTextLayer;
static TextLayer *s_RTL_ArtistSecondTextLayer;
static TextLayer *s_RTL_ArtistFirstTextLayer;
static ActionBarLayer *s_ActionBarLayer;

static void initialise_ui(void) {
  s_window = window_create();
  
  #ifdef PBL_SDK_2
  window_set_fullscreen(s_window, 0);
  #endif
  
  s_res_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_media_backward_icon = gbitmap_create_with_resource(RESOURCE_ID_MEDIA_BACKWARD_ICON);
  s_res_media_playpause_icon = gbitmap_create_with_resource(RESOURCE_ID_MEDIA_PLAYPAUSE_ICON);
  s_res_media_forward_icon = gbitmap_create_with_resource(RESOURCE_ID_MEDIA_FORWARD_ICON);
  // s_AlbumTextLayer
  s_AlbumTextLayer = text_layer_create(GRect(20, 85, 115, 55));
  text_layer_set_background_color(s_AlbumTextLayer, GColorClear);
  text_layer_set_text(s_AlbumTextLayer, "Album Here");
  text_layer_set_font(s_AlbumTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_AlbumTextLayer);
  
  // s_TrackTextLayer
  s_TrackTextLayer = text_layer_create(GRect(7, 30, 115, 60));
  text_layer_set_background_color(s_TrackTextLayer, GColorClear);
  text_layer_set_text(s_TrackTextLayer, "Fetching Music..");
  text_layer_set_font(s_TrackTextLayer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_TrackTextLayer);
  
  // s_ArtistTextLayer
  s_ArtistTextLayer = text_layer_create(GRect(25, -3, 115, 55));
  text_layer_set_background_color(s_ArtistTextLayer, GColorClear);
  text_layer_set_text(s_ArtistTextLayer, "Artist Here");
  text_layer_set_font(s_ArtistTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_ArtistTextLayer);
  
  // s_RTL_AlbumSecondTextLayer
  s_RTL_AlbumSecondTextLayer = text_layer_create(GRect(32, 112, 115, 27));
  text_layer_set_background_color(s_RTL_AlbumSecondTextLayer, GColorClear);
  text_layer_set_text(s_RTL_AlbumSecondTextLayer, "אלבום שם");
  text_layer_set_text_alignment(s_RTL_AlbumSecondTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_AlbumSecondTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_AlbumSecondTextLayer);
  
  // s_RTL_AlbumFirstTextLayer
  s_RTL_AlbumFirstTextLayer = text_layer_create(GRect(13, 100, 115, 27));
  text_layer_set_background_color(s_RTL_AlbumFirstTextLayer, GColorClear);
  text_layer_set_text(s_RTL_AlbumFirstTextLayer, "אלבום פה");
  text_layer_set_text_alignment(s_RTL_AlbumFirstTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_AlbumFirstTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_AlbumFirstTextLayer);
  
  // s_RTL_TrackSecondTextLayer
  s_RTL_TrackSecondTextLayer = text_layer_create(GRect(3, 70, 115, 30));
  text_layer_set_background_color(s_RTL_TrackSecondTextLayer, GColorClear);
  text_layer_set_text(s_RTL_TrackSecondTextLayer, "למוזיקה");
  text_layer_set_text_alignment(s_RTL_TrackSecondTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_TrackSecondTextLayer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_TrackSecondTextLayer);
  
  // s_RTL_TrackFirstTextLayer
  s_RTL_TrackFirstTextLayer = text_layer_create(GRect(3, 40, 115, 31));
  text_layer_set_background_color(s_RTL_TrackFirstTextLayer, GColorClear);
  text_layer_set_text(s_RTL_TrackFirstTextLayer, "ממתין");
  text_layer_set_text_alignment(s_RTL_TrackFirstTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_TrackFirstTextLayer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_TrackFirstTextLayer);
  
  // s_RTL_ArtistSecondTextLayer
  s_RTL_ArtistSecondTextLayer = text_layer_create(GRect(2, 3, 115, 27));
  text_layer_set_background_color(s_RTL_ArtistSecondTextLayer, GColorClear);
  text_layer_set_text(s_RTL_ArtistSecondTextLayer, "אמן שם");
  text_layer_set_text_alignment(s_RTL_ArtistSecondTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_ArtistSecondTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_ArtistSecondTextLayer);
  
  // s_RTL_ArtistFirstTextLayer
  s_RTL_ArtistFirstTextLayer = text_layer_create(GRect(3, -5, 115, 27));
  text_layer_set_background_color(s_RTL_ArtistFirstTextLayer, GColorClear);
  text_layer_set_text(s_RTL_ArtistFirstTextLayer, "אמן פה");
  text_layer_set_text_alignment(s_RTL_ArtistFirstTextLayer, GTextAlignmentRight);
  text_layer_set_font(s_RTL_ArtistFirstTextLayer, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_RTL_ArtistFirstTextLayer);
  
  // s_ActionBarLayer
  s_ActionBarLayer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_ActionBarLayer, s_window);
  action_bar_layer_set_background_color(s_ActionBarLayer, GColorBlack);
  action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_UP, s_res_media_backward_icon);
  action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_SELECT, s_res_media_playpause_icon);
  action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_DOWN, s_res_media_forward_icon);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_ActionBarLayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_AlbumTextLayer);
  text_layer_destroy(s_TrackTextLayer);
  text_layer_destroy(s_ArtistTextLayer);
  text_layer_destroy(s_RTL_AlbumSecondTextLayer);
  text_layer_destroy(s_RTL_AlbumFirstTextLayer);
  text_layer_destroy(s_RTL_TrackSecondTextLayer);
  text_layer_destroy(s_RTL_TrackFirstTextLayer);
  text_layer_destroy(s_RTL_ArtistSecondTextLayer);
  text_layer_destroy(s_RTL_ArtistFirstTextLayer);
  action_bar_layer_destroy(s_ActionBarLayer);
  gbitmap_destroy(s_res_media_backward_icon);
  gbitmap_destroy(s_res_media_playpause_icon);
  gbitmap_destroy(s_res_media_forward_icon);
}
// END AUTO-GENERATED UI CODE

static void sendToPhone(Tuplet *data) {
  
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Preparing to send data to phone..");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "null iterator received from app_message_outbox_begin!");
    return;
  }

  dict_write_tuplet(iter, data);
  dict_write_end(iter);
  app_message_outbox_send();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (bIsVolumeState) {
    action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_UP, s_res_media_backward_icon);
    action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_DOWN, s_res_media_forward_icon);
  } else {
    action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_UP, s_res_media_volup_icon);
    action_bar_layer_set_icon(s_ActionBarLayer, BUTTON_ID_DOWN, s_res_media_voldown_icon);
  }
  
  bIsVolumeState = !bIsVolumeState;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  Tuplet tuple = TupletInteger(ACTION_PLAYPAUSE_KEY, (bIsPlayingState ? 0 : 1));
  sendToPhone(&tuple);
  
  // Toggle playing state.
  bIsPlayingState = !bIsPlayingState;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (bIsVolumeState) {
    // Handle volume up here.
    Tuplet tuple = TupletInteger(ACTION_VOLUP_KEY, 0);
    sendToPhone(&tuple);
  } else {
    // Handle prev track here.
    Tuplet tuple = TupletInteger(ACTION_BACKWARD_KEY, 0);
    sendToPhone(&tuple);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (bIsVolumeState) {
    // Handle volume down here.
    Tuplet tuple = TupletInteger(ACTION_VOLDOWN_KEY, 0);
    sendToPhone(&tuple);
  } else {
    // Handle next track here.
    Tuplet tuple = TupletInteger(ACTION_FORWARD_KEY, 0);
    sendToPhone(&tuple);
  }
}

static void s_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void resetLayers(int topOffset, int widthOffset) {
  AdjustTextLayerPosition(s_ArtistTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_ArtistTextLayer, "");
  text_layer_set_overflow_mode(s_ArtistTextLayer, GTextOverflowModeFill);
  layer_set_hidden((Layer*)s_ArtistTextLayer, true);

  AdjustTextLayerPosition(s_RTL_ArtistFirstTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_ArtistFirstTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_ArtistFirstTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_ArtistFirstTextLayer, true);

  AdjustTextLayerPosition(s_RTL_ArtistSecondTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_ArtistSecondTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_ArtistSecondTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_ArtistSecondTextLayer, true);

  AdjustTextLayerPosition(s_TrackTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_TrackTextLayer, "");
  text_layer_set_overflow_mode(s_TrackTextLayer, GTextOverflowModeFill);
  layer_set_hidden((Layer*)s_TrackTextLayer, true);

  AdjustTextLayerPosition(s_RTL_TrackFirstTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_TrackFirstTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_TrackFirstTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_TrackFirstTextLayer, true);

  AdjustTextLayerPosition(s_RTL_TrackSecondTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_TrackSecondTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_TrackSecondTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_TrackSecondTextLayer, true);
  
  AdjustTextLayerPosition(s_AlbumTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_AlbumTextLayer, "");
  text_layer_set_overflow_mode(s_AlbumTextLayer, GTextOverflowModeFill);
  layer_set_hidden((Layer*)s_AlbumTextLayer, true);

  AdjustTextLayerPosition(s_RTL_AlbumFirstTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_AlbumFirstTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_AlbumFirstTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_AlbumFirstTextLayer, true);

  AdjustTextLayerPosition(s_RTL_AlbumSecondTextLayer, topOffset, widthOffset);
  text_layer_set_text(s_RTL_AlbumSecondTextLayer, "");
  text_layer_set_overflow_mode(s_RTL_AlbumSecondTextLayer, GTextOverflowModeWordWrap);
  layer_set_hidden((Layer*)s_RTL_AlbumSecondTextLayer, true);
}

static void showVolumeState(int percentage) {
  char buffer[30];
  snprintf(buffer, 30, "Volume at %d percent", percentage);
  
  if (toast == NULL) {  
    toast = toast_layer_create(s_window);
  } else {
    toast_layer_hide(toast);
  }
  
  toast_layer_show(toast, buffer, 1000);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Inbox Message Received!");
  
  resetLayers(0,0);
  
  // Get the first pair
  Tuple *t = dict_read_first(iterator);
    
  // Process all pairs present
  while(t != NULL) {
    
    char buffer[50];
    
    if (t->type == TUPLE_CSTRING) {
      snprintf(buffer, 50, "Got key %"PRIu32" and value %s", t->key, t->value->cstring);
    } else if (t->type == TUPLE_INT) {
      snprintf(buffer, 50, "Got key %"PRIu32" and value %"PRIu32"", t->key, t->value->int32);
    }
    
    LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, buffer);
    
    // Process this pair's key
    switch (t->key) {
      // If we got any of the META keys from the companion app, we should be in playing mode.
      case META_ARTIST_KEY:
        TextLayerSetTextRTLAware(s_ArtistTextLayer, s_RTL_ArtistFirstTextLayer, t->value->cstring, false);
        break;
      case META_TRACK_KEY:
        TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackFirstTextLayer, t->value->cstring, false);
        break;
      case META_ALBUM_KEY:
        TextLayerSetTextRTLAware(s_AlbumTextLayer, s_RTL_AlbumFirstTextLayer, t->value->cstring, false);
        break;
      case META_EXTRA_ARTIST_KEY:
        TextLayerSetTextRTLAware(s_ArtistTextLayer, s_RTL_ArtistSecondTextLayer, t->value->cstring, true);
        break;
      case META_EXTRA_TRACK_KEY :
        TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackSecondTextLayer, t->value->cstring, true);
        break;
      case META_EXTRA_ALBUM_KEY :
        TextLayerSetTextRTLAware(s_AlbumTextLayer, s_RTL_AlbumSecondTextLayer, t->value->cstring, true);
        break;
      case META_VOLSTATE_KEY :
        volume_level = t->value->int32;
        showVolumeState(t->value->int32);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  char buffer[50];
  snprintf(buffer, 50, "Inbox message dropped! reason given is %d", reason);
  LogMessageWithTimestamp(APP_LOG_LEVEL_ERROR, buffer);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  char buffer[50];
  snprintf(buffer, 50, "Outbox send failed! reason given is %d", reason);
  LogMessageWithTimestamp(APP_LOG_LEVEL_ERROR, buffer);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Outbox send success!");
}

static void bluetooth_connection_callback(bool connected) {
  // Have to give the connection some time to finish it's re-connection, otherwise communication with phone fails seemingly..
  psleep(200);
  
  if (!connected) {
    LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "BT Disconnected!");
    resetLayers(0,0);
    TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackFirstTextLayer, "No BT Connection", false);
  } else {
    LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "BT Connected!");
    TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackFirstTextLayer, "Loading..", false);  
    Tuplet tuple = TupletInteger(ACTION_INIT_KEY, 0);
    sendToPhone(&tuple);
  }
}

static void initComms() {
  
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Initializing communication!");
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage comms.
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void handle_window_load(Window* window) {
  
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Main Window Loading..");
  
  s_res_media_volup_icon = gbitmap_create_with_resource(RESOURCE_ID_MEDIA_VOLUP_ICON);
  s_res_media_voldown_icon = gbitmap_create_with_resource(RESOURCE_ID_MEDIA_VOLDOWN_ICON);
  
  action_bar_layer_set_click_config_provider(s_ActionBarLayer, (ClickConfigProvider)s_click_config_provider);
  text_layer_set_overflow_mode(s_AlbumTextLayer, GTextOverflowModeFill);
  text_layer_set_overflow_mode(s_TrackTextLayer, GTextOverflowModeFill);
  text_layer_set_overflow_mode(s_ArtistTextLayer, GTextOverflowModeFill);
  
  #ifdef PBL_SDK_3
    // NOTE: 10 is the difference in pixels between ACTION_BAR_WIDTH on SDK2 and SDK3. Not sure if there's a way to calculate this right now.
    resetLayers(STATUS_BAR_LAYER_HEIGHT,-10);
  
    // Set up statusbar ontop of other layers.
    Layer *window_layer = window_get_root_layer(window);
    s_status_bar = status_bar_layer_create();
  
    // Change the status bar width to make space for the action bar
    int16_t width = layer_get_bounds(window_layer).size.w - ACTION_BAR_WIDTH;
    GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
    layer_set_frame(status_bar_layer_get_layer(s_status_bar), frame);
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
  #else
    resetLayers(0,0);
  #endif
    
  if (bluetooth_connection_service_peek()) {
    TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackFirstTextLayer, "Loading..", false);
  } else {
    TextLayerSetTextRTLAware(s_TrackTextLayer, s_RTL_TrackFirstTextLayer, "No BT Connection", false);
  }
  
  // init AppMessage API.
  initComms();
  
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Asking for initial data from phone..");
  
  // Fetch existing metadata.
  Tuplet tuple = TupletInteger(ACTION_INIT_KEY, 0);
  sendToPhone(&tuple);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
  
  #ifdef PBL_SDK_3
  status_bar_layer_destroy(s_status_bar);
  #endif
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;
  uint64_t timestamp = data[0].timestamp;

  if(!did_vibrate) {
    // Print it out
    APP_LOG(APP_LOG_LEVEL_INFO, "t: %llu, x: %d, y: %d, z: %d", timestamp, x, y, z);

    // processing to see if volume adjustment is appropriate
    if (x < -850) {
      volume_mode -= volume_mode / 4.0;
      volume_mode += 1.0/4.0;
      buf[5] = '+';
    } else {
      buf[5] = '_';
      volume_mode -= volume_mode / 4.0;
    }

    // entering volume adjust mode
    if (volume_mode > 0.9 && volume_adjust == 0) {
      volume_adjust = 1;
      volume_matched = 0;
      vibes_short_pulse();
    }

    if (volume_adjust == 1 && abs(y) > 750) {
      volume_adjust = 0;
      volume_matched = 0;
    }
    // processing volume adjust mode
    if (volume_adjust == 1) {
      desired_volume_level = -x/10;
      if ((abs(desired_volume_level - volume_level)) < 10) {
        volume_matched = 1;
      }
      if (volume_matched == 1) {
        buf[10] = 'M';
        if (desired_volume_level - volume_level > 5) {
    	    Tuplet tuple = TupletInteger(ACTION_VOLUP_KEY, 0);
    	    sendToPhone(&tuple);
        } else if (desired_volume_level - volume_level < -5) {
    	    Tuplet tuple = TupletInteger(ACTION_VOLDOWN_KEY, 0);
    	    sendToPhone(&tuple);
        } 
      } else {
        buf[10] = 'i';
      }
    }
    
    // processing to see if skip adjustment is appropriate
    if (y > 850) {
      skip_mode -= skip_mode / 4.0;
      skip_mode += 1.0/4.0;
      buf[9] = 's';
    } else {
      skip_mode -= skip_mode / 4.0;
      buf[9] = 'k';
    }

    // skip cooldown control code
    if (skip_mode > 0.9) {
      skip_cooldown = 1;
      buf[8] = 'P';
      if (vibe_cooldown == 1) {
        vibes_short_pulse();
        vibe_cooldown = 0;
      }
    } else if (skip_mode < 0.2) {
      skip_cooldown = 0;
      buf[8] = 'N';
      vibe_cooldown = 1;
    }

    if (skip_cooldown == 1) {
      if (z < -320) {
    	  Tuplet tuple = TupletInteger(ACTION_BACKWARD_KEY, 0);
    	  sendToPhone(&tuple);
        skip_cooldown = 0;
        skip_mode = 0.3;
      } else if (z > 320) {
        Tuplet tuple = TupletInteger(ACTION_FORWARD_KEY, 0);
        sendToPhone(&tuple);
        skip_cooldown = 0;
        skip_mode = 0.0;
      }
    }

    //snprintf(buf, sizeof(buf), "%d", volume_level);
    if (volume_mode > 0.7) {
      buf[2] = 'X';
    } else {
      buf[2] = 'O';
    }

    text_layer_set_text(diag_layer, buf);

  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // A tap event occured
  volume_adjust = 0;
  volume_matched = 0;
}

void show_musicawindow(void) {
  LogMessageWithTimestamp(APP_LOG_LEVEL_DEBUG, "Initializing window..");

  // Subscribe to let accel data handler control things
  accel_data_service_subscribe(num_samples, accel_data_handler);
  
  // register for BT connection status callback.
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  
  // Set BT module to high responsiveness mode (NOTE: GREATLY INCREASES BATTERY USAGE!)
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

  // Subscribe to tap events
  accel_tap_service_subscribe(accel_tap_handler);
  
  initialise_ui();
  
  #ifdef PBL_SDK_2
    // Init status bar icon. Has to be done BEFORE pushing to stack, of course :)
    window_set_status_bar_icon(s_window, gbitmap_create_with_resource(RESOURCE_ID_HEADPHONES_STATUS_ICON));
  #endif

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = handle_window_load,
    .unload = handle_window_unload,
  });
  
  window_stack_push(s_window, true);

  diag_layer = text_layer_create(GRect(25,50,144,40));

  // shows diag layer
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(diag_layer));
}

void hide_musicawindow(void) {
  window_stack_remove(s_window, true);
  
  bluetooth_connection_service_unsubscribe();

  // unlists accelerometer data stuff
  accel_data_service_unsubscribe();

  // Unsubscribe from tap events
  accel_tap_service_unsubscribe();
 
  // Reset BT module to normal, low-power mode.
  app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
}
