// Indirect-count multi-draws masked on the GPU (nvn_mtl.m mdx_run)
#include <metal_stdlib>
using namespace metal;
struct mjob { device const uint *rec; device const uint *cnt; device uint *out; uint i, words; };   // i: the draw's index in its multi-draw; for an indirect mesh draw (words 0) the mesh grid's limit
#define MDX_MESH_PARTS 8u
#define MDX_MESH_GRID 1024u
kernel void mdx_mask(device const mjob *j [[buffer(0)]], uint t [[thread_position_in_grid]]) {
  mjob m = j[t]; if (!m.out) return;
  // An indirect mesh draw: NVN's {count, first} as Metal's threadgroups {x, y, z}:
  // +0: {count, 1, 1, 0} for a task shader
  // +16: {ceil(count / limit), 1, 1, 0} for an object function
  // +32: 8 parts of at most MDX_MESH_GRID for mesh shaders with no object function (chain 39417042cfe8)
  if (!m.words) {
    uint c = m.rec[0], f = m.rec[1];
    m.out[0] = c; m.out[1] = 1u; m.out[2] = 1u; m.out[3] = 0u;
    m.out[4] = c / m.i + (c % m.i != 0u); m.out[5] = 1u; m.out[6] = 1u; m.out[7] = 0u;
    for (uint k = 0; k < MDX_MESH_PARTS; k++) {
      uint o = k * MDX_MESH_GRID;
      device uint *p = m.out + 8u + 4u * k;
      p[0] = c > o ? min(c - o, MDX_MESH_GRID) : 0u;
      p[1] = 1u;
      p[2] = 1u;
      p[3] = f + o;
    }
    return;
  }
  bool live = m.i < *m.cnt;
  for (uint k = 0; k < m.words; k++) m.out[k] = live ? m.rec[k] : 0u; }
