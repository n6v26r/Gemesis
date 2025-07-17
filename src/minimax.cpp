#include "game.h"
#include "log.h"
#include "move.h"
#include "utils.h"

#include <cassert>
#include <vector>

#define INF 100'000'000
#define FUCKLOAD 1'000'000

Move moves[MAX_MINIMAX_DEPTH + 1][MAX_MOVES];
int bestMove[MAX_MINIMAX_DEPTH + 1];
int moveCnt[MAX_MINIMAX_DEPTH + 1];

#ifdef DEBUG
// NOTE: for debbugging purposes
std::vector<Move> moveBacklog;
#endif
int totalMoves = 0;

struct Score {
  int playerCnt;
  int score[MAX_PLAYER_CNT];
  Score(int playerCnt) { this->playerCnt = playerCnt; }
  Score(int playerCnt, int val) {
    this->playerCnt = playerCnt;
    for (int p = 0; p < playerCnt; p++)
      score[p] = val;
  }
  Score() { playerCnt = 0; }

  int get(int player) {
    int s = score[player];
    int maxOther = 0;
    for (int p = 0; p < playerCnt; p++) {
      if (p == player)
        continue;
      maxOther = MAX(maxOther, score[p]);
    }

    return s - maxOther;
  }
  int &operator[](int idx) { return score[idx]; }
};

int Eval(GameState *game, int player) {
  int score = 0;

  // TODO: extend this
  if (game->isEndGame())
    return game->player[player].score * FUCKLOAD;

  // for (auto nobles = game->nobles; nobles; nobles.clearSmallest()) {
  //   int nb = nobles.getSmallest();
  //   int dist = 0;
  //   for (int gem = 0; gem < GEM_CNT; gem++) {
  //     dist += MAX(0, NOBLE_CARDS[nb][gem] -
  //                        game->player[player].chipCnt.getBonus(gem));
  //     if (dist > MINIMAX_CLOSE_TO_NOBLE_MARGIN)
  //       break;
  //   }
  //   if (dist <= MINIMAX_CLOSE_TO_NOBLE_MARGIN)
  //     score += MINIMAX_CLOSE_TO_NOBLE_MUL;
  // }

  score += game->player[player].score * MINIMAX_TOTAL_SCORE_MUL;
  score += game->player[player].chipCnt.totalChipCnt * MINIMAX_TOTAL_CHIPS_MUL;
  score += game->player[player].chipCnt[GEM_CNT] * MINIMAX_TOTAL_GOLD_MUL;
  score += game->player[player].res.count() * MINIMAX_RES_CNT_MUL;
  score += game->player[player].cards.count() * MINIMAX_NUM_CARDS_MUL;
  return score;
}

Score StaticEval(GameState *game) {
  Score score(game->playerCnt);
  for (int p = 0; p < game->playerCnt; p++)
    score[p] = Eval(game, p);

  return score;
}

int StaticEvalDuo(GameState *game) {
  Score score(game->playerCnt);
  for (int p = 0; p < game->playerCnt; p++)
    score[p] = Eval(game, p);

  return score[0] - score[1];
}

bool triggerExit = false;

void preMM() {
  totalMoves = 0;
  triggerExit = false;
}

bool MMStatusOK() { return !triggerExit; }

Score minimax(int depth, int maxDepth, GameState &game) {
  if (totalMoves % 160000 == 0 && getTime() >= MINIMAX_KILL_AFTER) {
#ifdef DEBUG
    if (!triggerExit)
      logWarn("time: %lf", getTime());
#endif
    triggerExit = true;
    return Score(0);
  }
  if (depth == maxDepth) {
    totalMoves++;
    return StaticEval(&game);
  }

  if (game.player[game.currPlayer].score >= SCORE_ENDGAME) {
    return StaticEval(&game);
  }

  moveCnt[depth] = 0;
  getMoves(game, moves[depth], moveCnt[depth]);

  Score bestScore(game.playerCnt, INF);
  bestScore[game.currPlayer] = -INF;
  for (auto moveIdx = 0; moveIdx < moveCnt[depth]; moveIdx++) {
    auto &move = moves[depth][moveIdx];

#ifdef DEBUG
    // NOTE: for debugging
    GameState save = game;
#endif

    game.applyMove(moves[depth][moveIdx]);
#ifdef DEBUG
    moveBacklog.push_back(move);
#endif

    game.currPlayer = game.nextPlayer();

    Score score = minimax(depth + 1, maxDepth, game);
    game.currPlayer = game.prevPlayer();
#ifdef DEBUG
    moveBacklog.pop_back();
#endif

    game.unapplyMove(moves[depth][moveIdx]);

#ifdef DEBUG
    // DEBUGGING:
    for (int p = 0; p < game.playerCnt; p++) {
      int total = 0;
      for (int i = 0; i < CHIP_CNT; i++) {
        total += game.player[p].chipCnt[i];
      }
      if (total != game.player[p].chipCnt.totalChipCnt) {
        logInfo("DEPTH: %d\n", depth);
        save.debug();
        game.debug();
        logMove(move);
        logErr("HERE!!!");
      }
    }
    if (!(game == save)) {
      save.debug();
      game.debug();
      logMove(move);
    }
    logAssert(game == save, "!EQ");
#endif

    if (score.get(game.currPlayer) > bestScore.get(game.currPlayer)) {
      bestScore = score;
      bestMove[depth] = moveIdx;
    }
  }

  return bestScore;
}

int minimaxDuo(int depth, int maxDepth, GameState &game, int a, int b,
               bool maximize) {
  if (totalMoves % 16000 == 0 && getTime() >= MINIMAX_KILL_AFTER) {
#ifdef DEBUG
    if (!triggerExit)
      logWarn("time: %lf", getTime());
#endif
    triggerExit = true;
    return 0;
  }
  if (depth == maxDepth) {
    totalMoves++;
    return StaticEvalDuo(&game);
  }

  if (game.isEndGame()) {
    return StaticEvalDuo(&game);
  }

  moveCnt[depth] = 0;
  getMoves(game, moves[depth], moveCnt[depth]);

  int bestScore = (maximize ? -INF : INF);
  for (auto moveIdx = 0; moveIdx < moveCnt[depth]; moveIdx++) {
    auto &move = moves[depth][moveIdx];

#ifdef DEBUG
    // NOTE: for debugging
    GameState save = game;
#endif

    game.applyMove(moves[depth][moveIdx]);
#ifdef DEBUG
    moveBacklog.push_back(move);
#endif
    game.currPlayer = game.nextPlayer();

    int score = minimaxDuo(depth + 1, maxDepth, game, a, b, !maximize);
    game.currPlayer = game.prevPlayer();
#ifdef DEBUG
    moveBacklog.pop_back();
#endif
    game.unapplyMove(moves[depth][moveIdx]);

    if (maximize) {
      if (score > bestScore) {
        bestScore = score;
        bestMove[depth] = moveIdx;
      }
      if (bestScore > b) {
        break;
      }
      a = MAX(a, bestScore);
    } else {
      if (score < bestScore) {
        bestScore = score;
        bestMove[depth] = moveIdx;
      }
      if (bestScore < a) {
        break;
      }
      b = MIN(bestScore, b);
    }

#ifdef DEBUG
    // DEBUGGING:
    for (int p = 0; p < game.playerCnt; p++) {
      int total = 0;
      for (int i = 0; i < CHIP_CNT; i++) {
        total += game.player[p].chipCnt[i];
      }
      if (total != game.player[p].chipCnt.totalChipCnt) {
        logInfo("DEPTH: %d\n", depth);
        save.debug();
        game.debug();
        logMove(move);
        logErr("HERE!!!");
      }
    }
    // Other asserts
    if (!(game == save)) {
      save.debug();
      game.debug();
      logMove(move);
    }
    logAssert(game == save, "!EQ");
#endif
  }

  return bestScore;
}
