#include <pebble.h>

#define VERSION_PKEY 2

#define FOREGROUND_PKEY 3
GColor getForeground();

#define BACKGROUND_PKEY 4
GColor getBackground();

void config_in_received_handler(DictionaryIterator *iter, void *context); 

void config_init();

void config_deinit();