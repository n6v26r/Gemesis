#pragma once
#include "log.h"
#include "types.h"
#include "utils.h"
#include <cstdio>

struct PlayerState {
  int score = 0;
  DeckMask cards = 0;
  NobleMask nobles = 0;
  int secretRes = 0;
  DeckMask res = 0;
  FullChipSet chipCnt;

  void debug() const {
    fprintf(stderr, "  <PLAYER>\n");
    fprintf(stderr, "  Score: %d\n", score);
    fprintf(stderr, "  Bonus: ");
    for (int chip = 0; chip < GEM_CNT; chip++)
      fprintf(stderr, "%2d ", this->chipCnt.getBonus(chip));
    fprintf(stderr, "   | total: %d\n", chipCnt.totalBonusCnt);
    fprintf(stderr, "  Chips: ");
    for (int chip = 0; chip < CHIP_CNT; chip++)
      fprintf(stderr, "%2d ", this->chipCnt[chip]);
    fprintf(stderr, "| total: %d\n", chipCnt.totalChipCnt);

    fprintf(stderr, "  Cards: ");
    for (int card = 1; card <= CARDS_CNT; card++)
      if (cards.get(card))
        fprintf(stderr, "%d ", card);
    fprintf(stderr, "\n");

    fprintf(stderr, "  Res: ");
    for (int card = 1; card <= CARDS_CNT; card++)
      if (res.get(card))
        fprintf(stderr, "%d ", card);
    fprintf(stderr, "\n");

    fprintf(stderr, "  Nobles: ");
    for (int noble = 1; noble < NOBLE_CNT; noble++)
      if (nobles.get(noble))
        fprintf(stderr, "%d ", noble);
    fprintf(stderr, "\n");
  }

  void buy(int cardidx, ChipSet &c) {
    chipCnt.buy(cardidx, c);
    this->cards.set(cardidx);
    this->res.set(cardidx, 0);
    this->score += CARDS[cardidx][CHIP_CNT];
  }

  void unbuy(int cardidx, ChipSet &before, ChipSet &c) {
    this->score -= CARDS[cardidx][CHIP_CNT];
    chipCnt.unbuy(cardidx, before, c);
    this->cards.set(cardidx, 0);

    // Gold holds if it was reserved or not
    this->res.set(cardidx, (bool)before.gold);
  }

  bool canBuy(int cardIdx) { return chipCnt.canBuy(cardIdx); }

  bool shouldRecive(const int nobleIdx) {
    return chipCnt.shouldRecive(nobleIdx);
  }

  bool operator==(const PlayerState &other) {
    bool eq = this->score == other.score && this->cards == other.cards &&
              this->res == other.res && this->chipCnt == other.chipCnt;
    return eq;
  }
};

struct GameState {
  int currPlayer;
  int playerCnt;
  PlayerState player[MAX_PLAYER_CNT];
  DeckMask cards = 0;
  NobleMask nobles = 0;
  ChipSet chipCnt;

  GameState() {}

  void debug() const {
    fprintf(stderr, "<<<GAMEBOARD>>>\n");
    fprintf(stderr, "Player: %d/%d\n", currPlayer, playerCnt);
    fprintf(stderr, "Chips: ");
    for (int chip = 0; chip < CHIP_CNT; chip++)
      fprintf(stderr, "%d ", chipCnt[chip]);
    fprintf(stderr, "| total: %d\n", chipCnt.totalChipCnt);

    fprintf(stderr, "Cards: ");
    for (int card = 1; card <= CARDS_CNT; card++)
      if (cards.get(card))
        fprintf(stderr, "%d ", card);
    fprintf(stderr, "\n");

    fprintf(stderr, "Nobles: ");
    for (int noble = 1; noble < NOBLE_CNT; noble++)
      if (nobles.get(noble))
        fprintf(stderr, "%d ", noble);
    fprintf(stderr, "\n");

    for (int p = 0; p < playerCnt; p++) {
      player[p].debug();
    }
  }

  void applyMove(Move &m) {
    switch (m.code) {
    case Action::NO_ACTION:
      logErr("NO_ACTION");
      break;
    case Action::TAKE_3_DIFF_GEMS:
      chipCnt -= m.data;
      player[currPlayer].chipCnt += m.data;
      break;
    case Action::TAKE_2_SAME_GEMS:
      chipCnt.modChip(m.quant, -2);
      player[currPlayer].chipCnt.modChip(m.quant, +2);
      break;
    case Action::RES_CARD:
      player[currPlayer].res.set(m.quant);
      player[currPlayer].chipCnt.modChip(GEM_CNT, m.data[GEM_CNT]);
      chipCnt.modChip(GEM_CNT, -m.data[GEM_CNT]);
      cards.set(m.quant, 0);
      break;
    case Action::BUY_CARD:
      player[currPlayer].buy(m.quant, chipCnt);
      cards.set(m.quant, 0);
      // NOTE: sorry for bad code:

      // NOTE: TODO(FIX): If I meet the criteria for more nobles at the same
      // time, consider them all aquired
      for (auto nbls = nobles; nbls; nbls.clearSmallest()) {
        int noble = nbls.getSmallest();
        bool ok = true;
        for (int gem = 0; gem < GEM_CNT; gem++) {
          if (player[currPlayer].chipCnt.getBonus(gem) <
              NOBLE_CARDS[noble][gem]) {
            ok = false;
            break;
          }
        }

        if (ok) {
          nobles.set(noble, 0);
          player[currPlayer].nobles.set(noble);
          player[currPlayer].score += NOBLE_SCORE;
        }
      }
      break;
    default:
      break;
    }
  }
  void unapplyMove(Move &m) {
    switch (m.code) {
    case Action::NO_ACTION:
      logErr("NO_ACTION");
      break;
    case Action::TAKE_3_DIFF_GEMS:
      chipCnt += m.data;
      player[currPlayer].chipCnt -= m.data;
      break;
    case Action::TAKE_2_SAME_GEMS:
      chipCnt.modChip(m.quant, +2);
      player[currPlayer].chipCnt.modChip(m.quant, -2);
      break;
    case Action::RES_CARD:
      player[currPlayer].res.set(m.quant, 0);
      player[currPlayer].chipCnt.modChip(GEM_CNT, -m.data[GEM_CNT]);
      chipCnt.modChip(GEM_CNT, +m.data[GEM_CNT]);
      cards.set(m.quant);
      break;
    case Action::BUY_CARD:
      player[currPlayer].unbuy(m.quant, m.data, chipCnt);
      cards.set(m.quant, !m.data[GEM_CNT]);

      // NOTE: sorry for bad code:
      for (auto nbls = player[currPlayer].nobles; nbls; nbls.clearSmallest()) {
        int noble = nbls.getSmallest();
        bool ok = true;
        for (int gem = 0; gem < GEM_CNT; gem++) {
          if (player[currPlayer].chipCnt.getBonus(gem) <
              NOBLE_CARDS[noble][gem]) {
            ok = false;
            break;
          }
        }

        if (!ok) {
          nobles.set(noble);
          player[currPlayer].nobles.set(noble, 0);
          player[currPlayer].score -= NOBLE_SCORE;
        }
      }
      break;
    default:
      break;
    }
  }

  bool operator==(const GameState &other) {
    bool eq = this->currPlayer == other.currPlayer &&
              this->cards == other.cards && this->nobles == other.nobles;

    for (int c = 0; c < CHIP_CNT; c++)
      eq &= this->chipCnt[c] == other.chipCnt[c];
    for (int p = 0; p < this->playerCnt; p++)
      eq &= this->player[p] == other.player[p];

    return eq;
  }

  int nextPlayer() { return (this->currPlayer + 1) % this->playerCnt; };
  int prevPlayer() {
    return (this->playerCnt + this->currPlayer - 1) % this->playerCnt;
  }
};
