#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "sha_256.h"
#define LEN_ABC 37

unsigned char abc[] = "abcdefghijklmnopqrstuvwxyz";
uint32_t password_hash[8] = {0};

int is_valid (uint32_t* gen_hash, uint32_t* password_hash);
int password_gen(unsigned char *arr, int depth, int password_len);
void make_word (unsigned char* result, unsigned char* word, int len);

int password_gen(unsigned char arr[64], int depth, int password_len) 
{
  if (depth == password_len) 
  {
    uint32_t gen_hash[8] = {0};
    unsigned char gen_word[64] = {0};
    int valid = 0;
    
    make_word(gen_word, arr, password_len);
    sha_256(gen_word, gen_hash);
    valid = is_valid(gen_hash, password_hash);

    if (valid)
      printf("password: %s\n", arr);
    return valid;
  }

  for (int i = 0; i < LEN_ABC; i++) 
  {
    arr[depth] = abc[i];
    if (password_gen(arr, depth + 1,password_len))
      return 1;
  }
  return 0;
}

void make_word (unsigned char* result, unsigned char* word, int len)
{
  strncpy((char*)result, (char*)word, len);
  result[len] = 0x80;
  result[63] = len*8; 
}

int is_valid (uint32_t* gen_hash, uint32_t* password_hash)
{
  int i = 0;
  while ((i < 4) && (gen_hash[i] == password_hash[i]))
    i++;

  return i == 4;
}

int main (int argc, char* argv[])
{
  if (argc == 3)
  {
    uint32_t len = atoi(argv[1]);
    unsigned char password_word[64] = {0};

    make_word(password_word, (unsigned char*)argv[2], len);
    sha_256(password_word, password_hash);
  } else 
  {  
    FILE* hash = fopen("hash.txt", "r");
    for (int i = 0;i < 8; i++)
      fscanf(hash, "%x", &password_hash[i]);
  }

  unsigned char brut[64] = {0};
  for (int i = 1; i < 8; i++)
    if (password_gen(&brut, 0, i)) break;

  for (int i = 0; i < 8; i++)
    printf("%x", password_hash[i]);
   
  return 0;
}