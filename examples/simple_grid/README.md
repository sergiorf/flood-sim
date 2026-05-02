# Simple Grid Example

This example runs a toy `10x10` bowl-shaped terrain with uniform rainfall and a simple downhill redistribution rule.

The current Phase 1 model uses a closed boundary condition. Water can move only between cells inside the grid and cannot flow out across the outer edge of the raster.

Build and run from the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/floodsim_simple_grid
```
