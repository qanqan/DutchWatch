#include <pebble.h>
#include "string.h"
#include "config.h"
#include <ctype.h>
#include "num2words-nl.h"
  
static bool btConnected = false;
  
#define DEBUG 1
#define BUFFER_SIZE 44

static Window *window;

typedef struct {
	TextLayer *currentLayer;
	TextLayer *nextLayer;	
	PropertyAnimation *currentAnimation;
	PropertyAnimation *nextAnimation;
} Line;

static Line line1;
static Line line2;
static Line line3;
static Line line4;

static TextLayer *background_layer;

static struct tm *t;
static GFont lightFont;
static GFont boldFont;

static char line1Str[2][BUFFER_SIZE];
static char line2Str[2][BUFFER_SIZE];
static char line3Str[2][BUFFER_SIZE];
static char line4Str[2][BUFFER_SIZE];

// Animation handler
static void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	Layer *textLayer = text_layer_get_layer((TextLayer *)context);
	GRect rect = layer_get_frame(textLayer);
	rect.origin.x = 144;
	layer_set_frame(textLayer, rect);
}

// Animate line
static void makeAnimationsForLayers(Line *line, TextLayer *current, TextLayer *next)
{
	GRect fromRect = layer_get_frame(text_layer_get_layer(next));
	GRect toRect = fromRect;
	toRect.origin.x -= 144;
	
	line->nextAnimation = property_animation_create_layer_frame(text_layer_get_layer(next), &fromRect, &toRect);
	animation_set_duration((Animation *)line->nextAnimation, 400);
	animation_set_curve((Animation *)line->nextAnimation, AnimationCurveEaseOut);
	animation_schedule((Animation *)line->nextAnimation);
	
	GRect fromRect2 = layer_get_frame(text_layer_get_layer(current));
	GRect toRect2 = fromRect2;
	toRect2.origin.x -= 144;
	
	line->currentAnimation = property_animation_create_layer_frame(text_layer_get_layer(current), &fromRect2, &toRect2);
	animation_set_duration((Animation *)line->currentAnimation, 400);
	animation_set_curve((Animation *)line->currentAnimation, AnimationCurveEaseOut);
	
	animation_set_handlers((Animation *)line->currentAnimation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);
	
	animation_schedule((Animation *)line->currentAnimation);
}

// Update line
static void updateLineTo(Line *line, char lineStr[2][BUFFER_SIZE], char *value)
{
	TextLayer *next, *current;
	
	GRect rect = layer_get_frame(text_layer_get_layer(line->currentLayer));
	current = (rect.origin.x == 0) ? line->currentLayer : line->nextLayer;
	next = (current == line->currentLayer) ? line->nextLayer : line->currentLayer;
	
	// Update correct text only
	if (current == line->currentLayer) {
		memset(lineStr[1], 0, BUFFER_SIZE);
		memcpy(lineStr[1], value, strlen(value));
		text_layer_set_text(next, lineStr[1]);
	} else {
		memset(lineStr[0], 0, BUFFER_SIZE);
		memcpy(lineStr[0], value, strlen(value));
		text_layer_set_text(next, lineStr[0]);
	}
	
	makeAnimationsForLayers(line, current, next);
}

// Check to see if the current line needs to be updated
static bool needToUpdateLine(Line *line, char lineStr[2][BUFFER_SIZE], char *nextValue)
{
	char *currentStr;
	GRect rect = layer_get_frame(text_layer_get_layer(line->currentLayer));
	currentStr = (rect.origin.x == 0) ? lineStr[0] : lineStr[1];

	if (memcmp(currentStr, nextValue, strlen(nextValue)) != 0 ||
		(strlen(nextValue) == 0 && strlen(currentStr) != 0)) {
		return true;
	}
	return false;
}

// Update screen based on new time
static void display_time(struct tm *t)
{
	// The current time text will be stored in the following 4 strings
	char textLine1[BUFFER_SIZE];
	char textLine2[BUFFER_SIZE];
	char textLine3[BUFFER_SIZE];
	char textLine4[BUFFER_SIZE];
	
	time_to_4words(t->tm_hour, t->tm_min, textLine1, textLine2, textLine3, textLine4, BUFFER_SIZE);
	
	if (needToUpdateLine(&line1, line1Str, textLine1)) {
		updateLineTo(&line1, line1Str, textLine1);	
	}
	if (needToUpdateLine(&line2, line2Str, textLine2)) {
		updateLineTo(&line2, line2Str, textLine2);	
	}
	if (needToUpdateLine(&line3, line3Str, textLine3)) {
		updateLineTo(&line3, line3Str, textLine3);	
	}
	if (needToUpdateLine(&line4, line4Str, textLine4)) {
		updateLineTo(&line4, line4Str, textLine4);	
	}
}

// Update screen without animation first time we start the watchface
static void display_initial_time(struct tm *t)
{
	time_to_4words(t->tm_hour, t->tm_min, line1Str[0], line2Str[0], line3Str[0], line4Str[0], BUFFER_SIZE);
	
	text_layer_set_text(line1.currentLayer, line1Str[0]);
	text_layer_set_text(line2.currentLayer, line2Str[0]);
	text_layer_set_text(line3.currentLayer, line3Str[0]);
	text_layer_set_text(line4.currentLayer, line4Str[0]);
}

// Time handler called every minute by the system
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	t = tick_time;
  display_time(tick_time);
}

static void set_preferences(Line* line, GColor kleur) {
  text_layer_set_text_color(line->currentLayer,kleur);
  text_layer_set_text_color(line->nextLayer,kleur);
}

static void default_preferences(Line* line, GColor kleur, int y, bool bold) {
	line->currentLayer = text_layer_create(GRect(0, y, 144, 50));
	line->nextLayer = text_layer_create(GRect(144, y, 144, 50));
  if (bold) {
  	text_layer_set_font(line->currentLayer, boldFont);
  	text_layer_set_font(line->nextLayer, boldFont);
  } else {
  	text_layer_set_font(line->currentLayer, lightFont);
  	text_layer_set_font(line->nextLayer, lightFont);
  }
  set_preferences(line, kleur);
  text_layer_set_background_color(line->currentLayer, GColorClear);
  text_layer_set_background_color(line->nextLayer, GColorClear);
  text_layer_set_text_alignment(line->currentLayer, GTextAlignmentLeft);
  text_layer_set_text_alignment(line->nextLayer, GTextAlignmentLeft);
}

// Handler for updating the settings of the boxen after the configscreen closes with save
static void in_received_handler(DictionaryIterator *iter, void *context) {
  // call autoconf_in_received_handler
  //APP_LOG(APP_LOG_LEVEL_INFO, "In received handler");
	config_in_received_handler(iter, context);

  GColor front = getForeground();

  text_layer_set_background_color(background_layer,getBackground());
  set_preferences(&line1, front);
  set_preferences(&line2, front);
  set_preferences(&line3, front);
  set_preferences(&line4, front);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  display_initial_time(t);
}

//Bleutooth handler
static void bluetooth_connection_handler(bool connected){
	btConnected = connected;
}

//Stating main window of the app
static void main_window_load(Window *window) {
  // Determine window layer
  Layer *window_layer = window_get_root_layer(window);
  GColor back = getBackground();
  GColor front = getForeground();
      
  // Create the background of the 4 boxes
  background_layer = text_layer_create(GRect( 0, 0,144,168));
  text_layer_set_background_color(background_layer,back);

  default_preferences(&line1, front, 18, false);
  default_preferences(&line2, front, 48, false);
  default_preferences(&line3, front, 78, false);
  default_preferences(&line4, front, 110, true);

  layer_add_child(window_layer, text_layer_get_layer(background_layer));
	layer_add_child(window_layer, text_layer_get_layer(line1.currentLayer));
	layer_add_child(window_layer, text_layer_get_layer(line1.nextLayer));
	layer_add_child(window_layer, text_layer_get_layer(line2.currentLayer));
	layer_add_child(window_layer, text_layer_get_layer(line2.nextLayer));
	layer_add_child(window_layer, text_layer_get_layer(line3.currentLayer));
	layer_add_child(window_layer, text_layer_get_layer(line3.nextLayer));
	layer_add_child(window_layer, text_layer_get_layer(line4.currentLayer));
	layer_add_child(window_layer, text_layer_get_layer(line4.nextLayer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(background_layer);
  text_layer_destroy(line1.currentLayer);
  text_layer_destroy(line1.nextLayer);
  text_layer_destroy(line2.currentLayer);
  text_layer_destroy(line2.nextLayer);
  text_layer_destroy(line3.currentLayer);
  text_layer_destroy(line3.nextLayer);
  text_layer_destroy(line4.currentLayer);
  text_layer_destroy(line4.nextLayer);
  property_animation_destroy(line1.nextAnimation);
  property_animation_destroy(line1.currentAnimation);
  property_animation_destroy(line2.nextAnimation);
  property_animation_destroy(line2.currentAnimation);
  property_animation_destroy(line3.nextAnimation);
  property_animation_destroy(line3.currentAnimation);
  property_animation_destroy(line4.nextAnimation);
  property_animation_destroy(line4.currentAnimation);
}

static void init() {
  //init the configscreen
  config_init();

  // Custom fonts
	lightFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_31));
	boldFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_BOLD_36));

  // Create main Window element and assign to pointer
  window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);
  
  app_message_register_inbox_received(in_received_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  btConnected = bluetooth_connection_service_peek();
  bluetooth_connection_service_subscribe(bluetooth_connection_handler);

  // Configure time on init
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
	display_initial_time(t);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit() {
  bluetooth_connection_service_unsubscribe();
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
