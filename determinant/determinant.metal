#include <metal_stdlib>
using namespace metal;

#define TG 256

kernel void pivot_swap(device float*        A      [[buffer(0)]],
                       constant uint&       n      [[buffer(1)]],
                       constant uint&       k      [[buffer(2)]],
                       device uint*         swaps  [[buffer(3)]],
                       uint                 tid    [[thread_position_in_threadgroup]])
{
    threadgroup float sval[TG];
    threadgroup uint  sidx[TG];

    float best = -1.0f;
    uint  bi   = k;
    for (uint i = k + tid; i < n; i += TG) {
        float v = fabs(A[i * n + k]);
        if (v > best) { best = v; bi = i; }
    }
    sval[tid] = best;
    sidx[tid] = bi;
    threadgroup_barrier(mem_flags::mem_threadgroup);

    for (uint s = TG / 2; s > 0; s >>= 1) {
        if (tid < s && sval[tid + s] > sval[tid]) {
            sval[tid] = sval[tid + s];
            sidx[tid] = sidx[tid + s];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }

    uint p = sidx[0];
    if (p != k) {
        for (uint j = tid; j < n; j += TG) {
            float t = A[k * n + j];
            A[k * n + j] = A[p * n + j];
            A[p * n + j] = t;
        }
        if (tid == 0) swaps[0] += 1;
    }
}

kernel void eliminate(device float*   A  [[buffer(0)]],
                      constant uint&  n  [[buffer(1)]],
                      constant uint&  k  [[buffer(2)]],
                      uint2           gid [[thread_position_in_grid]])
{
    uint j = k + 1 + gid.x;
    uint i = k + 1 + gid.y;
    if (i >= n || j >= n) return;

    float pivot = A[k * n + k];
    if (pivot == 0.0f) return;
    float m = A[i * n + k] / pivot;
    A[i * n + j] -= m * A[k * n + j];
}
