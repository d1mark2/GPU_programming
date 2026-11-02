#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "sha_256.h"
#define LEN_ABC 37

unsigned char abc[] = "abcdefghijklmnopqrstuvwxyz";
uint32_t password_hash[8] = {0};

int is_valid (uint32_t* gen_hash, uint32_t* password_hash);
int password_gen(uint64_t index, unsigned char *abc, int password_len, unsigned char *arr);
void make_word (unsigned char* result, unsigned char* word, int len);

int password_gen(uint64_t index, unsigned char *abc, int password_len, unsigned char *arr) 
{
  for (int pos = password_len - 1; pos >= 0; pos--) 
  {
    arr[pos] = abc[index % LEN_ABC];
    index /= LEN_ABC;
  }
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

void parse_hash(char* hash, uint32_t* result_hash)
{
  for (int i = 0; i < 8; i++)
    sscanf(hash + 8 * i,"%8x",&result_hash[i]);
}

int main (int argc, char* argv[])
{
  unsigned char brut[64] = {0};
  
  
  if (argc == 2)
  parse_hash(argv[1], password_hash);
  else 
  {  
    printf("add target hash");
    return 1;
  }
  
  time_t start = clock();
  for (int password_len = 1; password_len < 8; password_len++)
  {
    uint64_t index_max = (uint64_t)pow(LEN_ABC, password_len);
    for (uint64_t index = 0; index < index_max; index++)
    {
      if (password_gen(index, abc, password_len, brut))
      {
        time_t end = clock();
        double diff_sec = (double)(end - start) / (double)CLOCKS_PER_SEC;

        printf("time: %lf", diff_sec);
        return 0;
      }
    }
  }
   
  return 1;
}