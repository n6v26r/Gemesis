#include "agent.h"
#include "game.h"
#include "log.h"
#include "types.h"
#include "utils.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "minimax.cpp"

void init() {
  getTime();
  srand(SEED);
}

int main() {
  init();
  GameState game;
  readGameState(&game);

  game.debug();

  Move m;

  // for (int depth = MIN_MINIMAX_DEPTH; depth <= MAX_MINIMAX_DEPTH; depth++) {
  //   GameState g = game;
  //   preMM();
  //   if (g.playerCnt != 2)
  //     minimax(0, depth, g);
  //   else
  //     minimaxDuo(0, depth, g, -INF, +INF, !game.currPlayer);
  //   if (!MMStatusOK()) {
  //     logInfo("Reached EOT at depth: %d", depth);
  //     break;
  //   } else {
  //     logArbiter("Depth: %d Evaluated: %d pos", depth, totalPos);
  //     m = moves[0][bestMove[0]];
  //   }
  // }

  // MORE than 6 has a bug
  minimaxDuo(0, 6, game, -INF, +INF, !game.currPlayer);
  m = moves[0][bestMove[0]];
  makeFinalMove(m);

  FILE *f = fopen("output.ok", "a");
  switch (m.code) {
  case Action::NO_ACTION:
    fprintf(f, "1 0\n");
    break;
  case Action::TAKE_3_DIFF_GEMS:
    fprintf(f, "1 %d ", m.quant);
    for (int i = 0; i < GEM_CNT; i++)
      if (m.data[i])
        fprintf(f, "%d ", i);
    fprintf(f, "\n");
    break;
  case Action::TAKE_2_SAME_GEMS:
    fprintf(f, "2 %d\n", m.quant);
    break;
  case Action::RES_CARD:
    fprintf(f, "3 %d\n", m.quant);
    break;
  case Action::BUY_CARD:
    fprintf(f, "4 %d\n", m.quant);
    break;
  }

  return 0;
}
