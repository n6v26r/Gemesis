# Gemesis
💎 Splendor game agent

Implements both Minimax (with alpha-beta pruning for 2 player games) and Monte Carlo Tree Search.
Compatible with the protocol described at: [splendor-tools](https://github.com/nerdvana-ro/splendor-tools/).

## Building

```
cmake .
make
```
> [!NOTE]
> Check `src/utils.h` for config options and parameters.
> Mainly, uncomment `#define USE_MINIMAX` to use minimax (with alpha beta for 2 players) instead of MCTS.
