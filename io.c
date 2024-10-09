
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "io.h"

/* Reads next char from stdin. If no more characters, it returns EOF */
int
read_char() {
  char c;
  if (read(0, &c, 1) == 1)
  {
    return c;
  } else {
    return EOF;
  }
}

/* Writes c to stdout.  If no errors occur, it returns 0, otherwise EOF */
int
write_char(char c) {
  if (write(1, &c, 1) == 1)
  {
    return 0;
  } else {
    return EOF;
  }
}

/* Writes a null-terminated string to stdout.  If no errors occur, it returns 0, otherwise EOF */
int
write_string(char* s) {
    int len = 0;
    while (s[len] != '\0') len++; {
        if (write(1, s, len) == len) {
            return 0;
        } else {
      return EOF;
        }
    }
}

/* Writes n to stdout (without any formatting).   
 * If no errors occur, it returns 0, otherwise EOF
 */

int write_int(int n) {
    int countIn = n;
    int length = 0;
    
    // Handle negative numbers
    if (n < 0) {
        length++;
        countIn = -n;
    }

    // Handle the special case where n is 0
    if (n == 0) {
        length = 1;
    } else {
        while (countIn > 0) {
            length++;
            countIn /= 10; // Divide by 10 to get the next digit
        }
    }

  char *buffer = (char *)malloc(length);
  if (!buffer) {
    return -1; // return error if memory allocation fails
  }
  int index = 0; // Initialize index to 0

  if (n < 0) {
    buffer[index++] = '-'; // Add a negative sign if n is negative
    n = -n; // Make n positive
  }

  // when n is 0:
  if (n == 0)
  {
    buffer[index++] = '0';
  } else {  
    while (n > 0) {
        buffer[index++] = n % 10 + '0'; // Add the next digit to the buffer
        n /= 10; // Divide by 10 to get the next digit
    }
  }
  int bytes_written = 0; // Initialize bytes_written to 0
  for (int i = index - 1; i >= 0; i--) {
    if (write(1, &buffer[i], 1) == -1) {
      return -1; // Return -1 if an error occurs
    }
    bytes_written++;
  }
  
  free(buffer); // Free the allocated memory. Because we use malloc!  
  return bytes_written;  // Return the number of bytes written
}