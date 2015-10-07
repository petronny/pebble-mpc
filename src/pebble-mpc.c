#include <pebble.h>
#include "progress_layer.h"

enum
{
	KEY_TEMPERATURE = 0,
	KEY_CONDITIONS = 1
};

static Window * window;
static GBitmap * previous_bitmap;
static GBitmap * next_bitmap;
static GBitmap * play_bitmap;
static GBitmap * pause_bitmap;
static GBitmap * volume_increase_bitmap;
static GBitmap * volume_decrease_bitmap;
static GBitmap * volume_increasing_bitmap;
static GBitmap * volume_decreasing_bitmap;
static GBitmap * playing_bitmap;
static GBitmap * paused_bitmap;

static TextLayer * author_text_layer;
static TextLayer * title_text_layer;
static TextLayer * current_text_layer;
static TextLayer * duration_text_layer;
static ActionBarLayer * action_bar_layer;
static StatusBarLayer * status_bar_layer;
static ProgressLayer  * progress_layer;
static BitmapLayer * status_bitmap_layer;

static bool play_or_pause;
static char title[100];
static char author[100];
static char current_buffer[10];
static char duration_buffer[10];
static uint16_t current, duration;

static void convert_time ( char * buffer, uint16_t time )
{
	uint8_t seconds = time % 60;
	uint8_t minutes = time / 60;

	if ( minutes >= 60 )
		{
			uint8_t hours = minutes / 60;
			minutes = minutes / 60;
			snprintf ( buffer, 10, "%d:%02d:%02d", hours, minutes, seconds );
		}

	else
		{
			snprintf ( buffer, 10, "%d:%02d", minutes, seconds );
		}
}

static void update_status()
{
	text_layer_set_text ( author_text_layer, author );
	text_layer_set_text ( title_text_layer, title );
	progress_layer_set_progress ( progress_layer, 100 * current / duration );

	convert_time ( current_buffer, current );
	convert_time ( duration_buffer, duration );
	text_layer_set_text ( current_text_layer,  current_buffer );

	if ( play_or_pause )
		{
			action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_SELECT, play_bitmap );
			bitmap_layer_set_bitmap ( status_bitmap_layer, paused_bitmap );
		}

	else
		{
			action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_SELECT, pause_bitmap );
			bitmap_layer_set_bitmap ( status_bitmap_layer, playing_bitmap );
		}

	text_layer_set_text ( duration_text_layer, duration_buffer );
}

static void select_click_handler ( ClickRecognizerRef recognizer, void * context )
{
	play_or_pause = !play_or_pause;
	update_status();
}

static void up_click_handler ( ClickRecognizerRef recognizer, void * context )
{
	/*text_layer_set_text ( author_text_layer, "Previous" );*/
}

static void up_long_click_handler ( ClickRecognizerRef recognizer, void * context )
{
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_UP, volume_increase_bitmap );
	text_layer_set_text ( author_text_layer, "Volume+" );
	bitmap_layer_set_bitmap ( status_bitmap_layer, volume_increasing_bitmap );
}

static void up_long_click_release_handler ( ClickRecognizerRef recognizer, void * context )
{
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_UP, previous_bitmap );
	update_status();
}

static void down_click_handler ( ClickRecognizerRef recognizer, void * context )
{
	/*text_layer_set_text ( author_text_layer, "Next" );*/
}

static void down_long_click_handler ( ClickRecognizerRef recognizer, void * context )
{
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_DOWN, volume_decrease_bitmap );
	text_layer_set_text ( author_text_layer, "Volume-" );
	bitmap_layer_set_bitmap ( status_bitmap_layer, volume_decreasing_bitmap );
}

static void down_long_click_release_handler ( ClickRecognizerRef recognizer, void * context )
{
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_DOWN, next_bitmap );
	update_status();
}

static void click_config_provider ( void * context )
{
	window_single_click_subscribe ( BUTTON_ID_UP, up_click_handler );
	window_single_click_subscribe ( BUTTON_ID_DOWN, down_click_handler );

	window_long_click_subscribe ( BUTTON_ID_SELECT, 1, NULL, select_click_handler );
	window_long_click_subscribe ( BUTTON_ID_UP, 500, up_long_click_handler , up_long_click_release_handler );
	window_long_click_subscribe ( BUTTON_ID_DOWN, 500, down_long_click_handler , down_long_click_release_handler );
}

static void inbox_received_callback ( DictionaryIterator * iterator, void * context )
{
	text_layer_set_text ( author_text_layer, "Received" );
}

static void inbox_dropped_callback ( AppMessageResult reason, void * context )
{
	APP_LOG ( APP_LOG_LEVEL_ERROR, "Message dropped!" );
}

static void outbox_failed_callback ( DictionaryIterator * iterator, AppMessageResult reason, void * context )
{
	APP_LOG ( APP_LOG_LEVEL_ERROR, "Outbox send failed!" );
}

static void outbox_sent_callback ( DictionaryIterator * iterator, void * context )
{
	APP_LOG ( APP_LOG_LEVEL_INFO, "Outbox send success!" );
}

static void get_status()
{
	play_or_pause = false;
	snprintf ( title, 100, "ポッケ村のテーマ" );
	snprintf ( author, 100, "栗田博文: 東京フィルハーモニー交響楽団" );
	current = 1234;
	duration = 5678;
}

static void tick_handler ( struct tm * tick_time, TimeUnits units_changed )
{
	// Get weather update every 30 minutes
	if ( tick_time->tm_sec % 10 == 0 )
		{
			// Begin dictionary
			DictionaryIterator * iter;
			app_message_outbox_begin ( &iter );

			// Add a key-value pair
			dict_write_uint8 ( iter, 0, 0 );

			// Send the message!
			app_message_outbox_send();
		}
}

static void window_load ( Window * window )
{
	Layer * window_layer = window_get_root_layer ( window );
	GRect bounds = layer_get_bounds ( window_layer );

	get_status();

	previous_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_PREVIOUS );
	next_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_NEXT );
	play_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_PLAY );
	pause_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_PAUSE );
	playing_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_PLAYING );
	paused_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_PAUSED );
	volume_increase_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_VOLUME_INCREASE );
	volume_decrease_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_VOLUME_DECREASE );
	volume_increasing_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_VOLUME_INCREASING );
	volume_decreasing_bitmap = gbitmap_create_with_resource ( RESOURCE_ID_VOLUME_DECREASING );

	status_bar_layer = status_bar_layer_create();
	status_bar_layer_set_colors ( status_bar_layer, GColorJaegerGreen, GColorBlack );
	int16_t width = bounds.size.w - ACTION_BAR_WIDTH;
	GRect frame = GRect ( 0, 0, width, STATUS_BAR_LAYER_HEIGHT );
	layer_set_frame ( status_bar_layer_get_layer ( status_bar_layer ), frame );
	layer_add_child ( window_layer, status_bar_layer_get_layer ( status_bar_layer ) );

	action_bar_layer = action_bar_layer_create();
	action_bar_layer_set_click_config_provider ( action_bar_layer, click_config_provider );
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_UP, previous_bitmap );
	action_bar_layer_set_icon ( action_bar_layer, BUTTON_ID_DOWN, next_bitmap );
	action_bar_layer_add_to_window ( action_bar_layer, window );

	author_text_layer = text_layer_create ( GRect ( 12, 25 , 90, 20 ) );
	text_layer_set_background_color ( author_text_layer, GColorClear );
	text_layer_set_text_alignment ( author_text_layer, GTextAlignmentLeft );
	text_layer_set_font ( author_text_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_18_BOLD ) );
	layer_add_child ( window_layer, text_layer_get_layer ( author_text_layer ) );

	title_text_layer = text_layer_create ( GRect ( 12, 43 , 90, 59 ) );
	text_layer_set_background_color ( title_text_layer, GColorClear );
	text_layer_set_text_alignment ( title_text_layer, GTextAlignmentLeft );
	text_layer_set_font ( title_text_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_24_BOLD ) );
	layer_add_child ( window_layer, text_layer_get_layer ( title_text_layer ) );

	current_text_layer = text_layer_create ( GRect ( 12, 102, 45, 20 ) );
	text_layer_set_background_color ( current_text_layer, GColorClear );
	text_layer_set_text_alignment ( current_text_layer, GTextAlignmentLeft );
	text_layer_set_font ( current_text_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_14 ) );
	layer_add_child ( window_layer, text_layer_get_layer ( current_text_layer ) );

	duration_text_layer = text_layer_create ( GRect ( 57, 102, 45, 20 ) );
	text_layer_set_background_color ( duration_text_layer, GColorClear );
	text_layer_set_text_alignment ( duration_text_layer, GTextAlignmentRight );
	text_layer_set_font ( duration_text_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_14 ) );
	layer_add_child ( window_layer, text_layer_get_layer ( duration_text_layer ) );

	progress_layer = progress_layer_create ( GRect ( 12, 120, 90, 4 ) );
	progress_layer_set_corner_radius ( progress_layer, 2 );
	progress_layer_set_foreground_color ( progress_layer, GColorRed );
	progress_layer_set_background_color ( progress_layer, COLOR_FALLBACK ( GColorBlack, GColorJaegerGreen ) );
	layer_add_child ( window_layer, progress_layer );

	GRect status_bitmap_bounds = gbitmap_get_bounds ( volume_increasing_bitmap );
	status_bitmap_layer = bitmap_layer_create ( GRect ( 12, 131, status_bitmap_bounds.size.w, status_bitmap_bounds.size.h ) );
	bitmap_layer_set_alignment ( status_bitmap_layer, GAlignTopLeft );
	bitmap_layer_set_bitmap ( status_bitmap_layer, playing_bitmap );
	layer_add_child ( window_layer, bitmap_layer_get_layer ( status_bitmap_layer ) );

	update_status();

}

static void window_unload ( Window * window )
{
	text_layer_destroy ( author_text_layer );
	text_layer_destroy ( title_text_layer );
	text_layer_destroy ( current_text_layer );
	text_layer_destroy ( duration_text_layer );
	action_bar_layer_destroy ( action_bar_layer );
	status_bar_layer_destroy ( status_bar_layer );
	progress_layer_destroy ( progress_layer );
	bitmap_layer_destroy ( status_bitmap_layer );

	gbitmap_destroy ( previous_bitmap );
	gbitmap_destroy ( next_bitmap );
	gbitmap_destroy ( play_bitmap );
	gbitmap_destroy ( pause_bitmap );
	gbitmap_destroy ( volume_increase_bitmap );
	gbitmap_destroy ( volume_decrease_bitmap );
	gbitmap_destroy ( volume_increasing_bitmap );
	gbitmap_destroy ( volume_decreasing_bitmap );
	gbitmap_destroy ( playing_bitmap );
	gbitmap_destroy ( paused_bitmap );
}

static void init ( void )
{
	tick_timer_service_subscribe ( MINUTE_UNIT, tick_handler );

	// Register callbacks
	app_message_register_inbox_received ( inbox_received_callback );
	// Open AppMessage
	app_message_open ( app_message_inbox_size_maximum(), app_message_outbox_size_maximum() );
	app_message_register_inbox_dropped ( inbox_dropped_callback );
	app_message_register_outbox_failed ( outbox_failed_callback );
	app_message_register_outbox_sent ( outbox_sent_callback );

	window = window_create();
	window_set_background_color ( window, GColorJaegerGreen );
	window_set_window_handlers ( window, ( WindowHandlers )
	{
		.load = window_load,
		 .unload = window_unload,
	} );
	const bool animated = true;
	window_stack_push ( window, animated );
}

static void deinit ( void )
{
	window_destroy ( window );
}

int main ( void )
{
	init();

	APP_LOG ( APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window );

	app_event_loop();
	deinit();
}
