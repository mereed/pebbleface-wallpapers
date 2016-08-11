#include <pebble.h>
#include "netdownload.h"



static Window *window;
static TextLayer *text_layer;
static BitmapLayer *bitmap_layer;
static GBitmap *current_bmp;

TextLayer *layer_date_text;
TextLayer *layer_date_text2;
TextLayer *layer_time_text;
TextLayer *layer_time_text2;

static GFont time_font;
static GFont date_font;

GBitmap *background_image = NULL;
static BitmapLayer *background_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;
static GBitmap *no_bluetooth_image;
static BitmapLayer *no_bluetooth_layer;

BitmapLayer *layer_conn_img;
GBitmap *img_bt_connect;
GBitmap *img_bt_disconnect;

int cur_day = -1;

int charge_percent = 0;

static uint8_t batteryPercent;

static GBitmap *battery_image = NULL;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;


#ifdef PBL_PLATFORM_CHALK
static char *images[] = {
  "http://www.themapman.com/galleryimages/palm1.png",
  "http://www.themapman.com/galleryimages/treesunset.png",
  "http://www.themapman.com/galleryimages/monarch4.png",
  "http://www.themapman.com/galleryimages/monarch5.png",
  "http://www.themapman.com/galleryimages/monarch3.png",
  "http://www.themapman.com/galleryimages/sunset1.png",
  "http://www.themapman.com/galleryimages/rave.png",
  "http://www.themapman.com/galleryimages/colours.png",
  "http://www.themapman.com/galleryimages/skater.png",
  "http://www.themapman.com/galleryimages/earth.png",
  "http://www.themapman.com/galleryimages/greenhills.png",
  "http://www.themapman.com/galleryimages/nightmtn.png",
  "http://www.themapman.com/galleryimages/berry.png",
  "http://www.themapman.com/galleryimages/earth2.png"
};
#endif

#ifdef PBL_PLATFORM_BASALT
static char *images[] = {
  "http://www.themapman.com/galleryimages/rave-b.png",
  "http://www.themapman.com/galleryimages/earth2-b2.png",
  "http://www.themapman.com/galleryimages/music2.png",
  "http://www.themapman.com/galleryimages/berry-b.png",
  "http://www.themapman.com/galleryimages/nightmtn-b.png",
  "http://www.themapman.com/galleryimages/wave1-b.png",
  "http://www.themapman.com/galleryimages/lighthouse1-b.png",
  "http://www.themapman.com/galleryimages/palm1-b.png",
  "http://www.themapman.com/galleryimages/treesunset-b.png",
  "http://www.themapman.com/galleryimages/monarch4-b.png",
  "http://www.themapman.com/galleryimages/monarch5-b.png",
  "http://www.themapman.com/galleryimages/monarch3-b.png",
  "http://www.themapman.com/galleryimages/sunset1-b.png",
  "http://www.themapman.com/galleryimages/colours-b.png",
  "http://www.themapman.com/galleryimages/skater-b.png",
  "http://www.themapman.com/galleryimages/earth-b.png",
  "http://www.themapman.com/galleryimages/greenhills-b.png",
  "http://www.themapman.com/galleryimages/eveningpalms1-b.png",
  "http://www.themapman.com/galleryimages/surfer1-b.png"
 
};
#endif

#ifdef PBL_PLATFORM_APLITE
static char *images[] = {
  "http://www.themapman.com/galleryimages/palm1-a.png",
  "http://www.themapman.com/galleryimages/treesunset-a.png",
  "http://www.themapman.com/galleryimages/monarch4-a.png",
  "http://www.themapman.com/galleryimages/monarch5-a.png",
  "http://www.themapman.com/galleryimages/monarch3-a.png",
  "http://www.themapman.com/galleryimages/sunset1-a.png",
  "http://www.themapman.com/galleryimages/rave-a.png",
  "http://www.themapman.com/galleryimages/colours-a.png",
  "http://www.themapman.com/galleryimages/skater-a.png",
  "http://www.themapman.com/galleryimages/earth-a.png",
  "http://www.themapman.com/galleryimages/greenhills-a.png",
  "http://www.themapman.com/galleryimages/nightmtn-a.png"
};
#endif

static unsigned long image = 0;




void show_next_image() {
  // show that we are loading by showing no image
  bitmap_layer_set_bitmap(bitmap_layer, NULL);

  text_layer_set_text(text_layer, "Wallpaper Loading");

  // Unload the current image if we had one and save a pointer to this one
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
    current_bmp = NULL;
  }

  netdownload_request(images[image]);

  image++;
  if (image >= sizeof(images)/sizeof(char*)) {
    image = 0;
  }
}


void change_battery_icon(bool charging) {

if (battery_image) {
    gbitmap_destroy(battery_image);
    battery_image = NULL;
  }
	if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }  else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

void update_battery_state(BatteryChargeState charge_state) {

  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) {
        change_battery_icon(false);  
    return;
  }
	
  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);
	
} 

void battery_layer_update_callback(Layer *me, GContext* ctx) {        
  //draw the remaining battery percentage
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(1, 1, ((batteryPercent/100.0)*13.0), 7), 0, GCornerNone);
}



static void toggle_bluetooth(bool connected) {
  if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
//	  show_next_image();
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_disconnect);
        vibes_short_pulse();
	}
}



void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth(connected);
}

void update_time(struct tm *tick_time) {

	static char time_text[] = "00:00";
    static char date_text[] = "xxx xxx 00xx xx xxxxxxx";

    char *time_format;

    int new_cur_day = tick_time->tm_year*1000 + tick_time->tm_yday;
    if (new_cur_day != cur_day) {
        cur_day = new_cur_day;

      strftime(date_text, sizeof(date_text), "%a, %e %b", tick_time);

	  text_layer_set_text(layer_date_text, date_text);
				
  }
	
	if (clock_is_24h_style()) {
        time_format = "%R";
		
    } else {
        time_format = "%l:%M";
    }

    strftime(time_text, sizeof(time_text), time_format, tick_time);

    if (!clock_is_24h_style() && (time_text[0] == '0')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
	
    text_layer_set_text(layer_time_text, time_text);
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);

// added this for testing
	
if (units_changed & MINUTE_UNIT) {
 show_next_image();
	

}

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	// font resources

	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_REGULAR_28));
    date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_16));	

	
	//add background layer
  background_image = gbitmap_create_with_resource( RESOURCE_ID_LOADING );
  background_layer = bitmap_layer_create( layer_get_frame( window_layer ) );
  bitmap_layer_set_bitmap( background_layer, background_image );
  layer_add_child( window_layer, bitmap_layer_get_layer( background_layer ) );

  text_layer = text_layer_create((GRect) { .origin = { 0, 36 }, .size = { bounds.size.w, 40 } });
  text_layer_set_text(text_layer, "Wallpapers");
  text_layer_set_font(text_layer, date_font);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  current_bmp = NULL;
	
  img_bt_connect     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  img_bt_disconnect  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_BLUETOOTH);	

#ifdef PBL_PLATFORM_CHALK
  layer_conn_img  = bitmap_layer_create(GRect(86, 121, 7, 11));
#else
  layer_conn_img  = bitmap_layer_create(GRect(128, 154, 7, 11));
#endif
  bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_conn_img)); 
		
#ifdef PBL_PLATFORM_CHALK
    layer_time_text = text_layer_create(GRect(-1, 122, 180, 70));
    layer_date_text = text_layer_create(GRect(0, 149, 180, 20));
#else
	layer_time_text = text_layer_create(GRect(-1, 123, 144, 70));
    layer_date_text = text_layer_create(GRect(0, 148, 144, 20));
#endif
	
    text_layer_set_background_color(layer_date_text, GColorClear);
    text_layer_set_font(layer_date_text, date_font);
    text_layer_set_background_color(layer_time_text, GColorClear);
    text_layer_set_font(layer_time_text, time_font);

    text_layer_set_text_alignment(layer_date_text, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_time_text, GTextAlignmentCenter);

	// set-up layer colours
    text_layer_set_text_color(layer_time_text, GColorBlack);
    text_layer_set_text_color(layer_date_text, GColorBlack);

	// composing layers
    layer_add_child(window_layer, text_layer_get_layer(layer_date_text));
    layer_add_child(window_layer, text_layer_get_layer(layer_time_text));
	
	
  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT);
  GRect bitmap_bounds_batt = gbitmap_get_bounds(battery_image);
#ifdef PBL_PLATFORM_CHALK
  GRect frame_batt = GRect(82, 168, bitmap_bounds_batt.size.w, bitmap_bounds_batt.size.h);
#else
  GRect frame_batt = GRect(5, 154, bitmap_bounds_batt.size.w, bitmap_bounds_batt.size.h);
#endif
  battery_layer = bitmap_layer_create(frame_batt);
  battery_image_layer = bitmap_layer_create(frame_batt);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));

    update_battery_state(battery_state_service_peek());
    toggle_bluetooth(bluetooth_connection_service_peek());
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(current_bmp);
}

void download_complete_handler(NetDownload *download) {
  printf("Loaded image with %lu bytes", download->length);
  printf("Heap free is %u bytes", heap_bytes_free());

  GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
  bitmap_layer_set_bitmap(bitmap_layer, bmp);

  // Save pointer to currently shown bitmap (to free it)
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
  }
  current_bmp = bmp;

  // Free the memory now
  free(download->data);

  // We null it out now to avoid a double free
  download->data = NULL;
  netdownload_destroy(download);
}




static void init(void) {


// international language support
  setlocale(LC_ALL, "");
	

  // Need to initialize this first to make sure it is there when
  // the window_load function is called by window_stack_push.
  netdownload_initialize(download_complete_handler);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
	
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(&update_battery_state);
  bluetooth_connection_service_subscribe(&toggle_bluetooth);
	
}


static void deinit(void) {
  netdownload_deinitialize(); // call this to avoid 20B memory leak
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
