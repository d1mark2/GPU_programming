#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "sha_256.h"

int is_valid (uint32_t* gen_hash, uint32_t* password_hash);
void make_word (unsigned char* result, unsigned char* word, int len);

void make_word (unsigned char* result, unsigned char* word, int len)
{
  strncpy((char*)result, (char*)word, len);
  result[len] = 0x80;
  result[63] = len*8; 
}

int main (int argc, char* argv[])
{
  uint32_t password_hash[8] = {0};
  if (argc == 2)
  {
    unsigned char password_word[64] = {0};

    make_word(password_word, (unsigned char*)argv[1], strlen(argv[1]));
    sha_256(password_word, password_hash);
  }

  for (int i = 0; i < 8; i++)
    printf("%08x", password_hash[i]);
  
  return 0;
}