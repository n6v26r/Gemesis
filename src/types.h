#pragma once
#include "log.h"
#include "utils.h"
#include <cstdio>
#include <system_error>

typedef char byte;

template <typename T> struct BitSet {
  T data;

  BitSet() { this->data = 0; }
  BitSet(T data) { this->data = data; }

  void flip(int bit) { data ^= ((T)1 << bit); }

  void set(int bit) { data |= ((T)1 << bit); }

  void set(int bit, bool val) {
    data &= ~((T)1 << bit);
    data |= ((T)1 << bit) * val;
  }

  bool get(int bit) const { return (data & ((T)1 << bit)) > 0; }

  T operator<<(const int bit) { return this->data << bit; }

  T operator>>(const int bit) { return this->data >> bit; }

  void operator<<=(const int bit) { this->data <<= bit; }

  void operator>>=(const int bit) { this->data >>= bit; }

  T operator|(const T bit) { return this->data | bit; }

  T operator&(const T bit) { return this->data & bit; }

  T operator^(const T bit) { return this->data ^ bit; }

  void operator&=(const T bit) { this->data &= bit; }

  void operator^=(const T bit) { this->data ^= bit; }

  void operator|=(const T bit) { this->data |= bit; }

  void operator=(const T data) { this->data = data; }

  bool operator==(const BitSet<T> &other) { return this->data == other.data; }

  operator bool() { return this->data; }

  int count() {
    int cnt = 0;
    T cp = this->data;
    while (cp) {
      cnt++;
      cp = cp & (cp - 1);
    }
    return cnt;
  }

  int getSmallest() {
    if constexpr (sizeof(T) <= sizeof(__U64_TYPE)) {
      return __builtin_ctzll(this->data);
    } else
      return (((this->data << 64) >> 64) == 0
                  ? __builtin_ctzll((this->data << 64) >> 64) +
                        __builtin_ctzll((this->data) >> 64)
                  : __builtin_ctzll((this->data << 64) >> 64));
  }
  void clearSmallest() { this->data = this->data & (this->data - 1); }
};

typedef BitSet<__int128_t> DeckMask;
typedef BitSet<unsigned short int> NobleMask;

struct ChipSet {
  char gemCnt[GEM_CNT];
  char totalChipCnt = 0;
  char gold = 0;

  ChipSet() {
    for (int gem = 0; gem < GEM_CNT; gem++) {
      gemCnt[gem] = 0;
    }
    gold = 0;
    totalChipCnt = 0;
  }

  ChipSet(int chips[CHIP_CNT]) {
    for (int gem = 0; gem < GEM_CNT; gem++) {
      gemCnt[gem] = chips[gem];
      totalChipCnt += chips[gem];
    }
    gold = chips[GEM_CNT];
    totalChipCnt += gold;
  }

  void operator+=(const ChipSet &other) {
    for (int gem = 0; gem < GEM_CNT; gem++) {
      this->gemCnt[gem] += other.gemCnt[gem];
      this->totalChipCnt += other.gemCnt[gem];
    }
    this->gold += other.gold;
    this->totalChipCnt += other.gold;
  }

  void operator-=(const ChipSet &other) {
    for (int gem = 0; gem < GEM_CNT; gem++) {
      this->gemCnt[gem] -= other.gemCnt[gem];
      this->totalChipCnt -= other.gemCnt[gem];
    }
    this->gold -= other.gold;
    this->totalChipCnt -= other.gold;
  }

  friend ChipSet operator+(const ChipSet &a, const ChipSet &b) {
    ChipSet r = a;
    r += b;
    return r;
  }

  friend ChipSet operator-(const ChipSet &a, const ChipSet &b) {
    ChipSet r = a;
    r -= b;
    return r;
  }

  void modChip(int idx, int val = 1) {
    if (idx == GEM_CNT)
      gold += val;
    else
      gemCnt[idx] += val;
    totalChipCnt += val;
  }

  const byte operator[](int idx) const {
    return (idx < GEM_CNT ? gemCnt[idx] : gold);
  }
};

struct FullChipSet : ChipSet {
private:
  int bonusCnt[GEM_CNT];

public:
  using ChipSet::ChipSet;
  using ChipSet::modChip;

  int totalBonusCnt = 0;

  void addBonus(int gem) {
    bonusCnt[gem]++;
    totalBonusCnt++;
  }
  void remBonus(int gem) {
    bonusCnt[gem]--;
    totalBonusCnt--;
  }

  int getBonus(int gem) const { return bonusCnt[gem]; }

  FullChipSet() : ChipSet() {
    for (int gem = 0; gem < GEM_CNT; gem++) {
      this->bonusCnt[gem] = 0;
    }
  }

  int needsGold(const int cardIdx) {
    int addgold = 0;
    int gold = this->gold;
    for (int gem = 0; gem < GEM_CNT; gem++) {
      int needsGold =
          MAX(0, (CARDS[cardIdx][gem] - bonusCnt[gem] - gemCnt[gem]));
      if (needsGold < gold)
        gold -= needsGold;
      else {
        addgold += needsGold - gold;
        gold = 0;
      }
    }
    return addgold;
  }

  bool shouldRecive(const int nobleIdx) {
    bool ok = true;
    for (int gem = 0;
         gem < GEM_CNT && ok /*missusing for loops, send me to prison*/;
         gem++) {
      ok &= (bonusCnt[gem] >= NOBLE_CARDS[nobleIdx][gem]);
    }
    return ok;
  }

  bool canBuy(const int cardIdx) { return needsGold(cardIdx) == 0; }

  void buy(int cardIdx, ChipSet &r) {
    totalChipCnt = 0;
    for (int gem = 0; gem < GEM_CNT; gem++) {
      int hasToPay = MAX(CARDS[cardIdx][gem] - bonusCnt[gem], 0);

      if (hasToPay > gemCnt[gem]) {
        gold -= (hasToPay - gemCnt[gem]);
        r.modChip(GEM_CNT, (hasToPay - gemCnt[gem]));
        r.modChip(gem, gemCnt[gem]);
        gemCnt[gem] = 0;
      } else {
        gemCnt[gem] -= hasToPay;
        r.modChip(gem, hasToPay);
      }
      totalChipCnt += gemCnt[gem];
    }
    totalChipCnt += gold;

    addBonus(CARDS[cardIdx][GEM_CNT]);
  }

  void unbuy(const int cardIdx, const ChipSet &before, ChipSet &r) {
    remBonus(CARDS[cardIdx][GEM_CNT]);
    totalChipCnt = 0;
    for (int gem = 0; gem < GEM_CNT; gem++) {
      int hasToPay = MAX(CARDS[cardIdx][gem] - bonusCnt[gem], 0);
      gold += MAX(0, hasToPay - before[gem]);
      r.modChip(GEM_CNT, -(MAX(0, hasToPay - before[gem])));
      r.modChip(gem, -(before[gem] - gemCnt[gem]));
      gemCnt[gem] = before[gem];
      totalChipCnt += gemCnt[gem];
    }
    totalChipCnt += gold;
  }

  bool operator==(const FullChipSet &other) {
    bool eq = true;
    for (int gem = 0; gem < GEM_CNT; gem++) {
      eq |= gemCnt[gem] == other.gemCnt[gem];
      eq |= bonusCnt[gem] == other.bonusCnt[gem];
    }
    eq |= gold == other.gold;
    eq |= totalChipCnt == other.totalChipCnt;
    eq |= totalBonusCnt == other.totalBonusCnt;
    return eq;
  }
};

enum Action {
  NO_ACTION = 0,
  TAKE_3_DIFF_GEMS = 1,
  TAKE_2_SAME_GEMS = 2,
  RES_CARD = 3,
  BUY_CARD = 4,
};

struct Move {
  Action code;
  int quant; // color / num of chips / card id
  ChipSet data;
};
