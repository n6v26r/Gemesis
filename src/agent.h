#pragma once
#include "game.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>

template <class... Args> inline void logArbiter(const char *s, Args... args) {
  fprintf(stderr, "kibitz ");
  fprintf(stderr, s, args...);
  fputc('\n', stderr);
}

inline void readPlayerState(PlayerState *p) {
  for (int chip = 0; chip < CHIP_CNT; chip++) {
    int cnt;
    scanf("%d", &cnt);
    p->chipCnt.modChip(chip, +cnt);
  }

  int cardCnt;
  scanf("%d", &cardCnt);
  for (int i = 0; i < cardCnt; i++) {
    int card;
    scanf("%d", &card);
    p->cards.set(card);
    p->chipCnt.addBonus(CARDS[card][GEM_CNT]);
    p->score += CARDS[card][CHIP_CNT];
  }

  int resCnt;
  scanf("%d", &resCnt);
  for (int i = 0; i < resCnt; i++) {
    int card;
    scanf("%d", &card);
    if (card > 0)
      p->res.set(card);
    else
      p->secretRes++;
  }

  int nobleCnt;
  scanf("%d", &nobleCnt);
  for (int i = 0; i < nobleCnt; i++) {
    int noble;
    scanf("%d", &noble);
    if (noble > 0) {
      p->nobles.set(noble);
      p->score += NOBLE_SCORE;
    }
  }
}

inline void readGameState(GameState *g) {
  // Discard values
  int currRound;
  int packSize[PACK_CNT];

  scanf("%d%d%d", &g->playerCnt, &g->currPlayer, &currRound);
  g->currPlayer--;
  for (int chip = 0; chip < CHIP_CNT; chip++) {
    int cnt = 0;
    scanf("%d", &cnt);
    g->chipCnt.modChip(chip, +cnt);
  }

  for (int pk = 0; pk < PACK_CNT; pk++) {
    scanf("%d", &packSize[pk]);
    for (int i = 0; i < VIS_PER_PACK; i++) {
      int card;
      scanf("%d", &card);
      if (card > 0)
        g->cards.set(card);
    }
  }

  int numNobles;
  scanf("%d", &numNobles);
  for (int i = 0; i < numNobles; i++) {
    int nobleID;
    scanf("%d", &nobleID);
    g->nobles.set(nobleID);
  }

  for (int p = 0; p < g->playerCnt; p++) {
    readPlayerState(&g->player[p]);
  }
}

inline void makeFinalMove(Move &m) {
  switch (m.code) {
  case Action::NO_ACTION:
    printf("1 0\n");
    break;
  case Action::TAKE_3_DIFF_GEMS:
    printf("1 %d ", m.quant);
    for (int i = 0; i < GEM_CNT; i++)
      if (m.data[i])
        printf("%d ", i);
    printf("\n");
    break;
  case Action::TAKE_2_SAME_GEMS:
    printf("2 %d\n", m.quant);
    break;
  case Action::RES_CARD:
    printf("3 %d\n", m.quant);
    break;
  case Action::BUY_CARD:
    printf("4 %d\n", m.quant);
    break;
  }
}
