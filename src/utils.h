#pragma once
#include <chrono>

inline double getTime() {
  static auto start = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  double time_taken =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();

  time_taken *= 1e-9;
  return time_taken;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// #define DEBUG
#define USE_MINIMAX

// CATEGORY: Bot params

// Minimax
#define SEED 1
#define MIN_MINIMAX_DEPTH 4
#define MAX_MINIMAX_DEPTH 12
#define MINIMAX_KILL_AFTER 2.0f

// Static eval multipliers
// NOTE: in here for fine-tuning
#include "settings.h"

// MCTS
#define MCTS_EXPLOR_MUL 1.41
#define MCTS_KILL_AFTER 2.0f
#define MCTS_NODE_ROLLOUTS 10
#define MCTS_FAILSAVE_STEPS 100000000
#define MCTS_SIM_MAX_MOVES 100 /*rounds * 2 */

// CATEGORY: Game constants
#define VIS_PER_PACK 4
#define PACK_CNT 3
#define GEM_CNT 5
#define CHIP_CNT 6
#define CARDS_CNT 90
#define NOBLE_CNT 10
#define NOBLE_SCORE 3

#define MAX_HOLD_CHIPS 10
#define MAX_HOLD_RES 3

#define MAX_PLAYER_CNT 4

#define SCORE_ENDGAME 15

// NOTE: numar prea mare
#define MAX_MOVES 100

const int CARDS[CARDS_CNT + 1][7] = {
    {-1, -1, -1, -1, -1, -1, -1}, {0, 0, 0, 3, 0, 0, 0}, {3, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 3, 2, 0},        {0, 0, 3, 0, 0, 3, 0}, {0, 3, 0, 0, 0, 4, 0},
    {0, 1, 2, 0, 0, 0, 0},        {0, 0, 1, 2, 0, 1, 0}, {0, 0, 0, 1, 2, 2, 0},
    {2, 0, 0, 0, 1, 3, 0},        {1, 2, 0, 0, 0, 4, 0}, {0, 0, 0, 4, 0, 0, 1},
    {0, 0, 0, 0, 4, 1, 1},        {4, 0, 0, 0, 0, 2, 1}, {0, 4, 0, 0, 0, 3, 1},
    {0, 0, 4, 0, 0, 4, 1},        {2, 0, 0, 2, 0, 0, 0}, {2, 0, 2, 0, 0, 1, 0},
    {0, 2, 0, 0, 2, 2, 0},        {0, 0, 2, 0, 2, 3, 0}, {0, 2, 0, 2, 0, 4, 0},
    {0, 1, 1, 1, 1, 0, 0},        {1, 0, 1, 1, 1, 1, 0}, {1, 1, 0, 1, 1, 2, 0},
    {1, 1, 1, 0, 1, 3, 0},        {1, 1, 1, 1, 0, 4, 0}, {0, 1, 1, 2, 1, 0, 0},
    {1, 0, 1, 1, 2, 1, 0},        {2, 1, 0, 1, 1, 2, 0}, {1, 2, 1, 0, 1, 3, 0},
    {1, 1, 2, 1, 0, 4, 0},        {0, 1, 0, 2, 2, 0, 0}, {2, 0, 1, 0, 2, 1, 0},
    {2, 2, 0, 1, 0, 2, 0},        {0, 2, 2, 0, 1, 3, 0}, {1, 0, 2, 2, 0, 4, 0},
    {1, 0, 0, 1, 3, 0, 0},        {0, 1, 3, 1, 0, 1, 0}, {1, 3, 1, 0, 0, 2, 0},
    {0, 0, 1, 3, 1, 3, 0},        {3, 1, 0, 0, 1, 4, 0}, {0, 0, 0, 0, 5, 0, 2},
    {0, 5, 0, 0, 0, 1, 2},        {0, 0, 5, 0, 0, 2, 2}, {5, 0, 0, 0, 0, 3, 2},
    {0, 0, 0, 5, 0, 4, 2},        {6, 0, 0, 0, 0, 0, 3}, {0, 6, 0, 0, 0, 1, 3},
    {0, 0, 6, 0, 0, 2, 3},        {0, 0, 0, 6, 0, 3, 3}, {0, 0, 0, 0, 6, 4, 3},
    {0, 0, 0, 3, 5, 0, 2},        {0, 3, 5, 0, 0, 1, 2}, {0, 0, 3, 5, 0, 2, 2},
    {5, 0, 0, 0, 3, 3, 2},        {3, 5, 0, 0, 0, 4, 2}, {0, 2, 4, 1, 0, 0, 2},
    {0, 0, 2, 4, 1, 1, 2},        {1, 0, 0, 2, 4, 2, 2}, {4, 1, 0, 0, 2, 3, 2},
    {2, 4, 1, 0, 0, 4, 2},        {2, 0, 0, 2, 3, 0, 1}, {0, 0, 3, 2, 2, 1, 1},
    {3, 2, 2, 0, 0, 2, 1},        {2, 3, 0, 0, 2, 3, 1}, {0, 2, 2, 3, 0, 4, 1},
    {2, 0, 3, 0, 3, 0, 1},        {3, 2, 0, 3, 0, 1, 1}, {0, 3, 2, 0, 3, 2, 1},
    {3, 0, 3, 2, 0, 3, 1},        {0, 3, 0, 3, 2, 4, 1}, {0, 7, 0, 0, 0, 0, 4},
    {0, 0, 7, 0, 0, 1, 4},        {0, 0, 0, 7, 0, 2, 4}, {0, 0, 0, 0, 7, 3, 4},
    {7, 0, 0, 0, 0, 4, 4},        {3, 7, 0, 0, 0, 0, 5}, {0, 3, 7, 0, 0, 1, 5},
    {0, 0, 3, 7, 0, 2, 5},        {0, 0, 0, 3, 7, 3, 5}, {7, 0, 0, 0, 3, 4, 5},
    {3, 6, 3, 0, 0, 0, 4},        {0, 3, 6, 3, 0, 1, 4}, {0, 0, 3, 6, 3, 2, 4},
    {3, 0, 0, 3, 6, 3, 4},        {6, 3, 0, 0, 3, 4, 4}, {0, 3, 5, 3, 3, 0, 3},
    {3, 0, 3, 5, 3, 1, 3},        {3, 3, 0, 3, 5, 2, 3}, {5, 3, 3, 0, 3, 3, 3},
    {3, 5, 3, 3, 0, 4, 3}};

const int NOBLE_CARDS[NOBLE_CNT + 1][GEM_CNT] = {
    {-1, -1, -1, -1, -1}, {4, 4, 0, 0, 0}, {0, 4, 4, 0, 0}, {0, 0, 4, 4, 0},
    {0, 0, 0, 4, 4},      {4, 0, 0, 0, 4}, {3, 3, 3, 0, 0}, {0, 3, 3, 3, 0},
    {0, 0, 3, 3, 3},      {3, 0, 0, 3, 3}, {3, 3, 0, 0, 3},
};
