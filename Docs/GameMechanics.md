# Game Mechanics and Architecture

## 1. Engine and ROM Interaction
The core engine provides the main loop, input, timing, and rendering primitives (framebuffer/audio). Each game is built as a separate position-independent shared object (`.so` plugin) called a ROM, conforming to a unified API (`init()`, `update()`, `draw()`).
When the engine runs, it uses `dlopen` to load the active ROM.

## 2. World and Camera System
### Screens and Mapping
The game world is split into screens measuring 256x256 pixels each (32x32 tiles, where 1 tile = 8x8 pixels). The active map is streamed into an 8-screen ring buffer (`block_buffer`). The total addressable width is 8 * 256 = 2048 pixels.
Tiles are abstracted into larger composite blocks encoded as macros in `map[]`.

### Camera Movement
The camera operates entirely on the X-axis (horizontal scrolling).
When the player (`mario`) moves, their local camera-relative offset is held at the center of the screen (`sprite.x = 128`).
The fractional delta difference is passed to `camera_move(0, center)`, which shifts `camera_x` (0-255). When `camera_x` exceeds 255 or falls below 0, it wraps around, and the engine streams in the next/previous column of blocks to the `block_buffer`.

## 3. Entities and Physics
### Entity Structure
Every interactive object in the game is an `entity`.
Entities store their type (e.g., PLAYER, ENEMY, PROJECTILE), canonical absolute world positions (`world_screen` and `world_x`), fractional velocities (X and Y), and an embedded `sprite` that contains bounding-box (hitbox) details and textures.
The global array `entities[]` tracks all active objects.
**Important Invariant**: `world_screen` and `world_x` are the authoritative source of truth for horizontal simulation. `sprite.x` is strictly a derived projection calculated immediately before rendering or legacy camera-relative checks. Physics and collisions integrate strictly in world space.

### Entity Lifecycles and Simulation Zones
To save memory and processing, entities are not universally simulated. 
- **World Coordinates**: When an entity is far from the camera, it exists in a dormant state storing only its `world_screen` and `world_x`.
- **Simulation Zone**: If the camera approaches (distance ranges between -128 and 384 pixels from the camera), the entity transitions to `sim_active = true`.
- **Physics Execution**: Only `sim_active` entities are processed per-frame. Their positions are integrated into absolute 16-bit world space coordinates (`get_world_x` / `set_world_x`), resolving tile collisions natively against the world map before mapping back to `world_screen` / `world_x`.
- **Render Zone**: After all physics are processed, active entities' world coordinates are projected to camera-relative `sprite.x` offsets. The rendering bounds are strictly `0` to `256`. If an active entity falls in this viewport, it is drawn to the framebuffer.

### Player Physics
The player entity operates with variable jump physics:
- If the Jump button is held, upward velocity experiences a reduced gravity multiplier (`JUMP_ACCEL_MULT_UP`), allowing higher jumps.
- Conversely, falling or releasing the button applies full gravity (`FALL_ACCEL * JUMP_ACCEL_MULT_DOWN`).
- Friction limits horizontal momentum conditionally based on whether the entity is airborne (`falling == true`) or grounded.

## 4. Collision Detection
### Tile Collisions (Entity vs. World)
Tile collision utilizes a rigorous two-pass, axis-separated AABB (Axis-Aligned Bounding Box) model:
1. **Pass 1 (X-Axis)**: `collide_entity_x` moves the entity horizontally and checks the leading vertical edge (top and bottom corners) against the `block_buffer` using `solid_at()`. If a solid block is hit, the entity is snapped floor-aligned `(((x) >> 4) << 4)` to the tile boundary, and X velocity is zeroed.
2. **Pass 2 (Y-Axis)**: `collide_entity_y` moves the entity vertically and checks the leading horizontal edge (left and right corners). Ground impacts trigger `falling = false`, aligning the feet securely to the tile.

`solid_at` seamlessly translates local draw coordinates plus `camera_x` into global lookups, wrapping backwards or forwards into the appropriate `active_screen` index.

### Entity vs. Entity Collisions
The `handle_entity_collisions` subroutine verifies intersections strictly between the player and `sim_active` entities (enemies, projectiles).
Using `colliding_y`, it differentiates between side/bottom hits vs. top impacts (stomps). 
Enemy sub-behavior is heavily dictated by a vtable architecture (`enemy_types`). When the player strikes an enemy, execution is routed to specific callbacks (e.g., `on_stomped`, `on_side_hit`) corresponding to the hit direction.

## 5. Draw Logic
The frame clears the pixel buffer and proceeds in phases:
1. **Camera Draw**: Renders the background grid by iterating across the 16x16 block grid of the `active_screen` and the `(active_screen + 1)` adjacent screen to fill the 256px viewport smoothly.
2. **Entity Draw**: Loops over the `entities` array. `draw_sprite` transforms the local bounding box and texture arrays into raw pixel injections into the `pixelbuf`, observing hardware-level attributes like `FLIP_X` or `FLIP_Y` dynamically modifying bit traversal direction.
