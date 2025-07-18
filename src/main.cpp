#include "agent.h"
#include "game.h"
#include "types.h"
#include "utils.h"
#include <cassert>
#include <cstdlib>

#ifdef USE_MINIMAX
#include "minimax.cpp"
#else
#include "mcts.cpp"
#endif

void init() {
  getTime();
  srand(SEED);
}

int main() {
  init();
  GameState game;
  readGameState(&game);

#ifdef DEBUG
  game.debug();
#endif

  Move m = {NO_ACTION, 0, {}};
#ifdef USE_MINIMAX
  for (int depth = MIN_MINIMAX_DEPTH; depth <= MAX_MINIMAX_DEPTH; depth++) {
    GameState g = game;
    preMM();
    int score;
    if (g.playerCnt != 2)
      score = minimax(0, depth, g).get(game.currPlayer);
    else
      score = minimaxDuo(0, depth, g, -INF, +INF, !game.currPlayer);
    if (!MMStatusOK()) {
      logInfo("Timeout at depth: %d", depth);
      break;
    } else {
      logArbiter("Depth: %d Evaluated: %d moves. Evaluation score: %d", depth,
                 totalMoves, score);
      m = moves[0][bestMove[0]];
    }
  }
#else
  MCTS MonteCarlo;
  m = MonteCarlo.getBestMove(game);
#endif

  makeFinalMove(m);

  return 0;
}
