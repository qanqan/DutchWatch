#include "num2words-nl.h"
#include "string.h"

static const char* const ONES[] = {
  "",
  "een",
  "twee",
  "drie",
  "vier",
  "vijf",
  "zes",
  "zeven",
  "acht",
  "negen"
};

static const char* const TEENS[] ={
  "tien",
  "elf",
  "twaalf",
  "dertien",
  "veertien",
  "vijftien",
  "zestien",
  "zeven",
  "acht",
  "negen"
};

static const char* const TENS[] = {
  "",
  "tien",
  "twintig",
  "dertig",
  "veertig",
  "vijftig",
  "zestig"
};

static size_t append_number(char* words, int num) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1) {
      strcat(words, TEENS[ones_val]);
      len += strlen(TEENS[ones_val]);
      return len;
    }
  }
  
  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
    if (tens_val > 0) {
       strcat(words, " en-");
       len += 4;
    }
  }
  
  if (tens_val > 0) {
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
  }
  return len;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}


void time_to_words(int hours, int minutes, char* words, size_t length) {

  size_t remaining = length;
  memset(words, 0, length);
  int iminutes = 60 - minutes;

  if (minutes > 30) {
    hours+=1;
    if (iminutes > 16 && iminutes < 20) {
      remaining -= append_number(words, iminutes);
      remaining -= append_string(words, remaining, "-tien-");
    } else if (iminutes < 17 || (iminutes % 10) == 0) {
      remaining -= append_string(words, remaining, " -");
      remaining -= append_number(words, iminutes);     
    } else {
      remaining -= append_number(words, iminutes);
    }
    remaining -= append_string(words, remaining, "-voor-");
  } else if (minutes > 0) {
    if (minutes > 16 && minutes < 20) {
      remaining -= append_number(words, minutes);
      remaining -= append_string(words, remaining, "-tien-");
    } else if (minutes < 17 || (minutes % 10) == 0) {
      remaining -= append_string(words, remaining, " -");
      remaining -= append_number(words, minutes);     
    } else {
      remaining -= append_number(words, minutes);
    }
    remaining -= append_string(words, remaining, "-over-");
  } else {
      remaining -= append_string(words, remaining, " - - -");      
  }
  if (hours == 0 || hours == 12) {
    remaining -= append_string(words, remaining, TEENS[2]);
  } else {
    remaining -= append_number(words, hours % 12);
  }
  remaining -= append_string(words, remaining, "-");
}

void time_to_4words(int hours, int minutes, char *line1, char *line2, char *line3, char *line4, size_t length)
{
	char value[length];
  
	time_to_words(hours, minutes, value, length);
	
	memset(line1, 0, length);
	memset(line2, 0, length);
	memset(line3, 0, length);
	memset(line4, 0, length);
	
	char *start = value;
	char *pch = strstr (start, "-");
	while (pch != NULL) {
		if (line1[0] == 0) {
			memcpy(line1, start, pch-start);
		} else if (line2[0] == 0) {
			memcpy(line2, start, pch-start);
		} else if (line3[0] == 0) {
			memcpy(line3, start, pch-start);
    } else if (line4[0] == 0) {
			memcpy(line4, start, pch-start);
		}
		start += pch-start+1;
		pch = strstr(start, "-");
	}
}
