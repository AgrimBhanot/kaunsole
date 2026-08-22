# Collision System Refactoring & World-Space Alignment

## Overview

This document outlines the architectural updates made to the entity collision detection and resolution pipeline. The primary objective of these changes is to eliminate screen-space snapping artifacts, unify coordinate spaces between the rendering pipeline and collision checks, and properly snap entity hitboxes against block boundaries when collisions occur.

## 1. Helper Function: `solid_at`

The `solid_at` function acts as the bridge between **draw space** (screen pixels) and **world/page space** (indexed block buffers).

```c
uint16_t solid_at(int16_t draw_x, int16_t draw_y) {
    int32_t raw_x = draw_x + camera_x;
    uint8_t screen;
    uint8_t bx;

    if (raw_x < 0) {
        screen = active_screen;
        bx = 0;
    } else if (raw_x < 256) {
        screen = active_screen;
        bx = raw_x / 16;
    } else {
        screen = (active_screen + 1) % N_SCREENS;
        bx = (raw_x - 256) / 16;
    }

    uint8_t by = (draw_y + camera_y) / 16;
    return block_buffer[screen][by][bx];
}
```

### World-Space Projection (`raw_x`)

The function takes screen-relative draw coordinates (`draw_x`, `draw_y`) and offsets them by `camera_x` and `camera_y` to map them directly into absolute multi-screen page space.

### Cross-Page Boundary Handling

If `raw_x` exceeds a single 256-pixel page width, the calculation automatically shifts to the adjacent screen:

```c
screen = (active_screen + 1) % N_SCREENS;
```

This allows seamless hit-testing across screen edges.

### Buffer Sampling

The function resolves block indices (`bx`, `by`) by dividing the world pixel coordinates by 16, then returns the corresponding block identifier from:

```c
block_buffer[screen][by][bx]
```

---

## 2. Resolution Pipeline Changes in `collide_entity`

The core updates focus on **world-space grid snapping** for horizontal collisions (`block_tl` and `block_tr`).

This replaces naive screen-space pixel shifts that could cause desynchronization when custom hitbox offsets or camera scrolling were introduced.

### A. Left-Side Collision Resolution (`block_tl`)

#### Previous Behavior

The previous implementation calculated block boundaries directly relative to `sprite->x` without accounting for the camera offset. This could result in improper snapping when hitboxes were not aligned with screen-space increments of 16 pixels.

#### Updated Implementation

```c
if (block_tl != 0) {
    int16_t body_left_x = sprite->x + sprite->hitbox.x + camera_x;
    int16_t snapped_row_left = (body_left_x / 16) * 16 + 16;
    sprite->x = snapped_row_left - sprite->hitbox.x - camera_x;
}
```

#### World Mapping

The entity's left hitbox edge is converted into absolute world coordinates by adding `camera_x`.

#### Boundary Snap

The collision position is snapped to the right edge of the blocking tile:

```c
(body_left_x / 16) * 16 + 16
```

This calculates the next 16-pixel grid boundary in world space.

#### Screen Re-Projection

The snapped world coordinate is converted back into draw space by subtracting `camera_x`, while also accounting for the hitbox offset.

This ensures that the entity's hitbox edge is positioned flush against the block boundary.

---

### B. Right-Side Collision Resolution (`block_tr`)

#### Previous Behavior

The previous implementation was subject to the same screen-space grid mismatch. This could cause entities with customized hitbox widths or offsets to snap backwards unpredictably.

#### Updated Implementation

```c
if (block_tr != 0) {
    int16_t body_right_x =
        sprite->x + sprite->hitbox.x + sprite->hitbox.width + camera_x;

    int16_t snapped_row_right = (body_right_x / 16) * 16;

    sprite->x =
        snapped_row_right - sprite->hitbox.x - sprite->hitbox.width - camera_x;
}
```

#### World Mapping

The rightmost edge of the hitbox is evaluated in world coordinates using `camera_x`.

#### Boundary Snap

The collision position is snapped to the left edge of the target solid block:

```c
(body_right_x / 16) * 16
```

This aligns the hitbox boundary with the 16-pixel world-space tile grid.

#### Screen Re-Projection

The resulting world-space position is translated back into draw space.

Because both the collision check and resolution operate against the same world-space grid, the entity remains tightly aligned with the block boundary regardless of hitbox width or offset configuration.

---

## Summary

The refactoring establishes a consistent coordinate-space model throughout collision detection and resolution:

1. **Draw-space coordinates** are converted into **world-space coordinates** using the camera offset.
2. Collision queries sample the appropriate block using world-space coordinates.
3. Collision boundaries are calculated against the **16×16 world-space tile grid**.
4. The resulting position is converted back into draw space.
5. Hitbox offsets and widths are explicitly included during resolution.

This approach eliminates screen-space snapping artifacts and ensures that entity collision boundaries remain correctly aligned with the world grid while the camera scrolls or the entity uses custom hitbox configurations.

> ## IMPORTANT NOTE

The `collision.c` and `collision.h` files have been moved to `roms/`, as the collision functionality is more appropriately implemented as part of the game. The corresponding changes have been made throughout the project to reflect the new file locations and ensure that the collision system continues to function correctly.
