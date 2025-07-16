#include "game.h"
#include "log.h"
#include "move.h"
#include "types.h"
#include "utils.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <vector>

#define INF 100'000'000

struct Score {
  double wins[MAX_PLAYER_CNT];
  Score() {
    for (int p = 0; p < MAX_PLAYER_CNT; p++) {
      wins[p] = 0;
    }
  }

  void operator+=(const Score &other) {
    for (int p = 0; p < MAX_PLAYER_CNT; p++) {
      wins[p] += other.wins[p];
    }
  }

  double &operator[](int idx) { return wins[idx]; }
};

bool isEndGame(GameState &game) {
  bool yes = (game.currPlayer == game.playerCnt - 1);
  if (!yes)
    return yes;

  yes = false;
  for (int p = 0; p < game.playerCnt; p++) {
    yes |= (game.player[p].score >= SCORE_ENDGAME);
  }
  return yes;
};

bool isNone(Move &m) {
  return m.code == Action::NO_ACTION ||
         (m.code == Action::TAKE_3_DIFF_GEMS && !m.quant);
}

Score Eval(GameState &game) {
  if (!isEndGame(game)) {
    Score s;
    for (int p = 0; p < game.playerCnt; p++) {
      if (p == game.currPlayer)
        continue;
      s[p] = (double)1 / (game.playerCnt - 1);
    }
    return s;
  }

  Score score;
  struct EvalInfo {
    int score;
    int cards;
    int player;

    bool operator==(const EvalInfo &other) {
      return cards == other.cards && score == other.score;
    }

    bool operator<(const EvalInfo &other) {
      if (score != other.score)
        return score < other.score;
      return cards > other.cards;
    }

  } details[game.playerCnt];

  for (int p = 0; p < game.playerCnt; p++) {
    details[p] = {game.player[p].score, game.player[p].chipCnt.totalBonusCnt,
                  p};
  }

  std::sort(details, details + game.playerCnt);
  // logInfo("Details: {[%d %d (%d)], [%d %d (%d)]}", details[0].score,
  //         details[0].cards, details[0].player, details[1].score,
  //         details[1].cards, details[1].player);

  int winPlayerCnt = 1;
  for (int i = game.playerCnt - 1; i >= 0; i--) {
    while (i > 0 && details[i] == details[i - 1]) {
      winPlayerCnt++;
      i--;
    }
  }

  for (int i = game.playerCnt - 1; i > game.playerCnt - 1 - winPlayerCnt; i--) {
    score[details[i].player] += ((double)1 / winPlayerCnt);
  }

  // logInfo("Score: <%lf %lf>", score[0], score[1]);
  return score;
}

Move _globalMoveBuffer[MAX_MOVES];
Move getRandMove(GameState &game) {
  int moveCnt = 0;
  getMoves(game, _globalMoveBuffer, moveCnt);
  int pos = rand() % moveCnt;
  // logInfo("choose move: %d / %d", pos, moveCnt);
  return _globalMoveBuffer[pos];
}

struct Node {
  int vis = 0;
  Score score;
  Move move;
  Node *parent;
  Node *child[MAX_MOVES];
  Move moves[MAX_MOVES];
  int moveCnt = 0;

  double winRate(int player) { return (double)score[player] / vis; }

  Node(Node *parent) { this->parent = parent; }

  void expand(GameState &game) {
    getMoves(game, moves, moveCnt);
    assert(moveCnt < MAX_MOVES);
    for (int moveIdx = 0; moveIdx < moveCnt; moveIdx++) {
      child[moveIdx] = new Node(this);
    }
  }

  Score rollOut(GameState &game) {
    Score s;
    for (int r = 0; r < MCTS_NODE_ROLLOUTS; r++) {
      GameState g = game;
      int movesNum = 0;
      while (!isEndGame(g) && movesNum < MCTS_SIM_MAX_MOVES) {
        Move m = getRandMove(g);
        g.applyMove(m);
        while (g.cards.count() < VIS_PER_PACK * PACK_CNT) {
          g.cards.set(1 + rand() % (CARDS_CNT + 1));
        }
        g.currPlayer = g.nextPlayer();
        movesNum++;
      }
      s += Eval(g);
    }
    return s;
  }

  int chooseChild(int player) {
    double lg = log(vis);
    double bestUTC = -INF;
    int bestChildId = 0;
    for (int c = 0; c < moveCnt; c++) {
      double uct;
      if (child[c]->vis == 0) {
        uct = INF;
      } else {
        double explot = child[c]->winRate(player);
        double explor = sqrt(lg / child[c]->vis);
        uct = explot + explor * MCTS_EXPLOR_MUL;
      }

      if (uct > bestUTC) {
        bestUTC = uct;
        bestChildId = c;
      }
    }
    return bestChildId;
  }
};

struct MCTS {
  int steps = 0;
  Node *root;
  std::vector<Move> moveBacklog;

  MCTS() { root = new Node(NULL); }

  Node *select(GameState &game) {
    Node *nd = root;
    moveBacklog = {};
    while (nd->vis != 0) {

      // Leaf node evaled but not yet expanded
      if (nd->moveCnt == 0) {
        nd->expand(game);
        continue;
      }

      auto childIdx = nd->chooseChild(game.currPlayer);
      game.applyMove(nd->moves[childIdx]);
      moveBacklog.push_back(nd->moves[childIdx]);
      game.currPlayer = game.nextPlayer();
      nd = nd->child[childIdx];
    }
    return nd;
  }

  void backprop(Node *nd, Score &s) {
    while (nd) {
      nd->score += s;
      nd->vis += MCTS_NODE_ROLLOUTS;
      nd = nd->parent;
    }
  }

  // Select - Simulate - Backpropagate
  void SSB(GameState &game) {
    GameState g = game;
    Node *leaf = select(g);
    Score s = leaf->rollOut(g);
    backprop(leaf, s);
  }

  Move getBestMove(GameState &game) {
    while (steps < MCTS_FAILSAVE_STEPS) {
      if (getTime() > MCTS_KILL_AFTER) {
        break;
      }

      SSB(game);
      steps++;
    }
    Move m;
    double bestWinProb = -1;
    for (int c = 0; c < root->moveCnt; c++) {
#ifdef DEBUG
      logMove(root->moves[c]);
      logInfo("Win prob: (%lf) <%lf %lf> (%d)\n",
              root->child[c]->winRate(game.currPlayer),
              root->child[c]->score[0], root->child[c]->score[1],
              root->child[c]->vis);
#endif
      if (bestWinProb < root->child[c]->winRate(game.currPlayer)) {
        bestWinProb = root->child[c]->winRate(game.currPlayer);
        m = root->moves[c];
      }
    }
    return m;
  }
};
