The initial version of object motion vectors is built, the runtime launches cleanly, and startup checks pass. G-buffer shaders containing the new code compile during level loading.

How it works:
- The vertex shader of each G-buffer draw preserves final vertex positions and samples positions of matching vertices from the previous frame.
- The pixel shader writes the delta into an auxiliary G-buffer render target.
- Where an object has moved, FSR receives this velocity vector; stationary regions fall back to the camera motion vector.
- Positions are recorded selectively for moving draws: draws whose vertex shader constants (transform matrix, bone palette) changed relative to the previous frame. Static geometry incurs zero history overhead.
- This covers animations evaluated in the vertex stage: skinning, weapons, cloth physics, and enemies.
