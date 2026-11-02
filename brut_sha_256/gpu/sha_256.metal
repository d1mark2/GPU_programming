#include <metal_stdlib>                      
using namespace metal;                       

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32-(n))))
#define S0(x)  (ROTR(x,7)  ^ ROTR(x,18) ^ ((x) >> 3))
#define S1(x)  (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))
#define BS0(x) (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))
#define BS1(x) (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))
#define CH(x,y,z)  (((x)&(y)) ^ (~(x)&(z)))
#define MAJ(x,y,z) (((x)&(y)) ^ ((x)&(z)) ^ ((y)&(z)))

enum { 
a, b, c, d, e, f, g, h 
};             

constant uint32_t H0[8] = {                   
  0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
  0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
};

constant uint32_t K[64] = {                  
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void index_to_candidate (thread uchar* out, device const uchar* charset,
                         uint charsetLen, uint passLen, ulong idx)
{
  for (int i = int(passLen) - 1; i >= 0; i--) 
  {
    out[i] = charset[idx % charsetLen];
    idx /= charsetLen;
  }
}

void atouint(thread uint32_t* w, thread const uchar* source)
{
  for (int i = 0; i < 16; i++)
    w[i] = ((uint32_t)source[4*i] << 24) + ((uint32_t)source[4*i+1] << 16) +
           ((uint32_t)source[4*i+2] << 8)  +  (uint32_t)source[4*i+3];
}

void sha_256 (thread const uchar* source, thread uint32_t* result)
{
  uint32_t w[64] = {0};
  uint32_t state[8];
  uint32_t t1, t2;

  atouint(w, source);

  for (int i = 0; i < 8; i++) 
  {           
    state[i]  = H0[i];
    result[i] = H0[i];
  }

  for (int i = 16; i < 64; i++)
    w[i] = w[i-16] + S0(w[i-15]) + w[i-7] + S1(w[i-2]);

  for (int i = 0; i < 64; i++)
  {
    t1 = state[h] + BS1(state[e]) + CH(state[e], state[f], state[g]) + K[i] + w[i];
    t2 = BS0(state[a]) + MAJ(state[a], state[b], state[c]);

    for (int j = 7; j > 0; j--)
      state[j] = state[j-1] + (j == e ? t1 : 0);
    state[a] = t1 + t2;
  }

  for (int i = 0; i < 8; i++)
    result[i] += state[i];
}

kernel void sha256_search(device const uchar* charset [[buffer(0)]],
                          constant uint &charset_len [[buffer(1)]],
                          constant uint &pass_len [[buffer(2)]],
                          constant ulong &batchOffset [[buffer(3)]],
                          device const uint32_t* target_hash [[buffer(4)]],
                          device atomic_uint* found_flag [[buffer(5)]],
                          device ulong* found_index [[buffer(6)]],
                          uint tid [[thread_position_in_grid]])
{
  if (atomic_load_explicit(found_flag, memory_order_relaxed) != 0) 
    return;

  ulong idx = batchOffset + (ulong)tid;

  uchar block[64] = {0};
  index_to_candidate(block, charset, charset_len, pass_len, idx);

  block[pass_len] = 0x80;                 
  ulong bit_len = (ulong)pass_len * 8;
  for (int i = 0; i < 8; i++) 
    block[63 - i] = (uchar)(bit_len >> (8 * i));

  uint32_t digest[8];
  sha_256(block, digest);

  bool match = true;
  for (int i = 0; i < 8; i++)
  {
    if (digest[i] != target_hash[i]) 
    {
      match = false; 
      break; 
    }
  }

  if (match) 
  {
    uint expected = 0;
    if (atomic_compare_exchange_weak_explicit(found_flag, &expected, 1u,
        memory_order_relaxed, memory_order_relaxed))
    found_index[0] = idx;
  }
}