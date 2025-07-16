#pragma once
#include "game.h"
#include "log.h"
#include "types.h"
#include "utils.h"
#include <cstdio>

inline void getMoves(GameState &game, Move *moveArr, int &moveCnt) {
  moveCnt = 0;
  // logInfo(">Generating moves...");
  // Action 1 or 2
  // TODO: Rewrite this

  int gemsTypes = 0;
  for (int gem = 0; gem < GEM_CNT; gem++) {
    gemsTypes += (game.chipCnt[gem] > 0);
  }

  int chips = MIN(
      3, MIN(MAX_HOLD_CHIPS - game.player[game.currPlayer].chipCnt.totalChipCnt,
             gemsTypes));
  // logInfo(">Can take: %d chips\n", chips);
  switch (chips) {
  case 3:
    for (int i = 0; i < GEM_CNT; i++) {
      if (!game.chipCnt[i])
        continue;
      for (int j = i + 1; j < GEM_CNT; j++) {
        if (!game.chipCnt[j])
          continue;
        for (int k = j + 1; k < GEM_CNT; k++) {
          if (!game.chipCnt[k])
            continue;
          moveArr[moveCnt++] = Move{TAKE_3_DIFF_GEMS, 3, ChipSet()};
          moveArr[moveCnt - 1].data.modChip(i, +1);
          moveArr[moveCnt - 1].data.modChip(j, +1);
          moveArr[moveCnt - 1].data.modChip(k, +1);
        }
      }
    }
    break;
  case 2:
    for (int i = 0; i < GEM_CNT; i++) {
      if (!game.chipCnt[i])
        continue;
      for (int j = i + 1; j < GEM_CNT; j++) {
        if (!game.chipCnt[j])
          continue;
        moveArr[moveCnt++] = Move{TAKE_3_DIFF_GEMS, 2, ChipSet()};
        moveArr[moveCnt - 1].data.modChip(i, +1);
        moveArr[moveCnt - 1].data.modChip(j, +1);
      }
    }
    break;
  case 1:
    for (int i = 0; i < GEM_CNT; i++) {
      if (!game.chipCnt[i])
        continue;
      moveArr[moveCnt++] = Move{TAKE_3_DIFF_GEMS, 1, ChipSet()};
      moveArr[moveCnt - 1].data.modChip(i, +1);
    }
    break;
  }

  if (game.player[game.currPlayer].chipCnt.totalChipCnt + 2 <= MAX_HOLD_CHIPS) {
    for (int i = 0; i < GEM_CNT; i++) {
      if (game.chipCnt[i] >= 4) {
        moveArr[moveCnt++] = Move{TAKE_2_SAME_GEMS, i};
      }
    }
  }

  // Action 3 (Reserve)
  auto placeholder =
      game.player[game.currPlayer].res.count() +
              game.player[game.currPlayer].secretRes <
          MAX_HOLD_RES &&
      (game.player[game.currPlayer].chipCnt.totalChipCnt < MAX_HOLD_CHIPS ||
       !game.chipCnt[GEM_CNT]);
  if (placeholder)
    for (auto cards = game.cards; cards; cards.clearSmallest()) {
      int cardnum = cards.getSmallest();
      moveArr[moveCnt++] = Move{
          RES_CARD, cardnum, (int[]){0, 0, 0, 0, 0, game.chipCnt[GEM_CNT] > 0}};
    };

  // Action 4

  // Visible on the table
  for (auto cards = game.cards; cards; cards.clearSmallest()) {
    int cardnum = cards.getSmallest();
    if (game.player[game.currPlayer].canBuy(cardnum)) {
      moveArr[moveCnt++] =
          Move{BUY_CARD, cardnum,
               (int[]){
                   game.player[game.currPlayer].chipCnt[0],
                   game.player[game.currPlayer].chipCnt[1],
                   game.player[game.currPlayer].chipCnt[2],
                   game.player[game.currPlayer].chipCnt[3],
                   game.player[game.currPlayer].chipCnt[4],
                   0,
               }}; // keep gem number in order to correctly undo move
    }
  }

  // Reserved
  for (auto cards = game.player[game.currPlayer].res; cards;
       cards.clearSmallest()) {
    int cardnum = cards.getSmallest();
    if (game.player[game.currPlayer].canBuy(cardnum)) {
      moveArr[moveCnt++] = Move{BUY_CARD, cardnum,
                                (int[]){
                                    game.player[game.currPlayer].chipCnt[0],
                                    game.player[game.currPlayer].chipCnt[1],
                                    game.player[game.currPlayer].chipCnt[2],
                                    game.player[game.currPlayer].chipCnt[3],
                                    game.player[game.currPlayer].chipCnt[4],
                                    1,
                                }};
    }
  }

  if (moveCnt == 0) {
    moveArr[moveCnt++] = {TAKE_3_DIFF_GEMS, 0};
  }
}

inline void logMove(Move &m) {
  fprintf(stderr, "[" ANSI_GREEN " Move " ANSI_RESET "]: ");
  switch (m.code) {
  case NO_ACTION:
    fprintf(stderr, "NONE (error)");
    break;
  case TAKE_3_DIFF_GEMS:
    fprintf(stderr, "Take Chips: {%d %d %d %d %d %d}", m.data[0], m.data[1],
            m.data[2], m.data[3], m.data[4], m.data[5]);
    break;
  case TAKE_2_SAME_GEMS:
    fprintf(stderr, "Take 2 chips of %d", m.quant);
    break;
  case RES_CARD:
    fprintf(stderr, "Reserve: %d, getting {%d %d %d %d %d %d}", m.quant,
            m.data[0], m.data[1], m.data[2], m.data[3], m.data[4], m.data[5]);
    break;
  case BUY_CARD:
    fprintf(stderr, "Buy card: %d (gems before: %d %d %d %d %d [%d]])", m.quant,
            m.data[0], m.data[1], m.data[2], m.data[3], m.data[4], m.data[5]);
    break;
  }
  fputc('\n', stderr);
}
