#include <pebble.h>
#include "config.h"

static int _version;
static int _foreground;
static int _background;

//htoi converts a hex string to an integer
int htoi(const char *s, unsigned int *res) {
  if ('0' == s[0] && ('x' == s[1] || 'X' == s[1]))
  s += 2;
  int c;
  unsigned int rc;
  for (rc = 0; '\0' != (c = *s); s++) {
    if ( c >= 'a' && c <= 'f') {
      c = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      c = c - 'A' + 10;
    } else if (c >= '0' && c <= '9') {
      c = c - '0';
    } else {
      return -1;
    }
    rc *= 16;
    rc += (unsigned int) c;
  }
  *res = rc;
  return 0;
}

//GColorfromHEXSTR converts an hexstring to a GColor value.
GColor GColorFromHEXSTR(char const* hexstring) {
  unsigned int x;
  if (0 == htoi(hexstring,&x)) {
    unsigned int red = (x & 0xff0000) >> 16;
    unsigned int green = (x & 0x00ff00) >> 8;
    unsigned int blue = (x & 0x0000ff) >> 0;
    return GColorFromRGB(red,green,blue);
  }
  return GColorClear;
}

int getVersion(){return _version;}
void setVersion(int value){_version = value;}

GColor getForeground(){return GColorFromHEX(_foreground);}
int getFColor(){return _foreground;}
void setForeground(int value){_foreground = value;}

GColor getBackground(){return GColorFromHEX(_background);}
int getBColor(){return _background;}
void setBackground(int value){_background = value;}

// Set value for language attributes
void configInt(DictionaryIterator *iter, int key, void (*function)(int)) {
  Tuple *tuple = dict_find(iter, key);
 
  //APP_LOG(APP_LOG_LEVEL_INFO, "Handling key %d",key);
  if (tuple) {
    int tmp = tuple->value->int32;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Key %d has value %d",key,tmp);
    (*function)(tmp);
    persist_write_int(key,tmp);
    //APP_LOG(APP_LOG_LEVEL_INFO, "Stored Key %d is now %d",key,(int)persist_read_int(key));
  }
}


// Handle the return values from pebble-js-app.js
void config_in_received_handler(DictionaryIterator *iter, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "In config received handler");

  configInt(iter,VERSION_PKEY,setVersion);
  configInt(iter,FOREGROUND_PKEY,setForeground);
  configInt(iter,BACKGROUND_PKEY,setBackground);

  //APP_LOG(APP_LOG_LEVEL_INFO, "Voorgrond %d",_foreground);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Achtergrond %d",_background);
}

// Set value for language attributes
void setInt(int key, int value, void (*function)(int)) {
 	if (persist_exists(key)) {
    int tmp = persist_read_int(key);
		(*function)(tmp);
	}	else {
		(*function)(value);
	}
}

// Housekeeping for previous versions
void cleanupStorage() {
  int version;
  
 	if (persist_exists(VERSION_PKEY)) {
    version = persist_read_int(VERSION_PKEY);
    if (version < 20) {
      /* placeholdfer for conversion software */
    }                                   
  } else {
      if (persist_exists(0)) persist_delete(0);
      if (persist_exists(1)) persist_delete(1);
      persist_write_int(VERSION_PKEY,15);    
  }
}

void config_init(){
  cleanupStorage();
  setInt(VERSION_PKEY,0,setVersion);
  setInt(FOREGROUND_PKEY,0xFFFFFF,setForeground);
  setInt(BACKGROUND_PKEY,0x3F00FF,setBackground);
}
