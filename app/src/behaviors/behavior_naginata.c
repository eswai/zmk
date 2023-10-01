/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_naginata

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 薙刀式

// 31キーを32bitの各ビットに割り当てる
#define B_Q (1UL << 0)
#define B_W (1UL << 1)
#define B_E (1UL << 2)
#define B_R (1UL << 3)
#define B_T (1UL << 4)

#define B_Y (1UL << 5)
#define B_U (1UL << 6)
#define B_I (1UL << 7)
#define B_O (1UL << 8)
#define B_P (1UL << 9)

#define B_A (1UL << 10)
#define B_S (1UL << 11)
#define B_D (1UL << 12)
#define B_F (1UL << 13)
#define B_G (1UL << 14)

#define B_H (1UL << 15)
#define B_J (1UL << 16)
#define B_K (1UL << 17)
#define B_L (1UL << 18)
#define B_SEMI (1UL << 19)

#define B_Z (1UL << 20)
#define B_X (1UL << 21)
#define B_C (1UL << 22)
#define B_V (1UL << 23)
#define B_B (1UL << 24)

#define B_N (1UL << 25)
#define B_M (1UL << 26)
#define B_COMMA (1UL << 27)
#define B_DOT (1UL << 28)
#define B_SLASH (1UL << 29)

#define B_SPACE (1UL << 30)

static uint8_t ng_chrcount = 0; // 文字キー入力のカウンタ
// static bool is_naginata = false;   // 薙刀式がオンかオフか
// static uint8_t naginata_layer = 0; // NG_*を配置しているレイヤー番号
// static uint16_t ngon_keys[2];      // 薙刀式をオンにするキー(通常HJ)
// static uint16_t ngoff_keys[2];     // 薙刀式をオフにするキー(通常FG)
static uint8_t keycnt = 0UL; // 　押しているキーの数
static uint32_t keycomb = 0UL; // 同時押しの状態を示す。32bitの各ビットがキーに対応する。
// static bool is_henshu = false; // 編集モードかどうか
// static bool henshu_done = false; // 編集モードを実行したか、編集モード空打ちだったか

const uint32_t ng_key[] = {

    [A - A] = B_A,     [B - A] = B_B,         [C - A] = B_C,         [D - A] = B_D,
    [E - A] = B_E,     [F - A] = B_F,         [G - A] = B_G,         [H - A] = B_H,
    [I - A] = B_I,     [J - A] = B_J,         [K - A] = B_K,         [L - A] = B_L,
    [M - A] = B_M,     [N - A] = B_N,         [O - A] = B_O,         [P - A] = B_P,
    [Q - A] = B_Q,     [R - A] = B_R,         [S - A] = B_S,         [T - A] = B_T,
    [U - A] = B_U,     [V - A] = B_V,         [W - A] = B_W,         [X - A] = B_X,
    [Y - A] = B_Y,     [Z - A] = B_Z,         [SEMI - A] = B_SEMI,   [COMMA - A] = B_COMMA,
    [DOT - A] = B_DOT, [SLASH - A] = B_SLASH, [SPACE - A] = B_SPACE,
};

#define NKEYS 3  // 最大何キーまでバッファに貯めるか
#define NDOUJI 3 // 組み合わせにある同時押しするキーの数、薙刀式なら3

struct keystroke {
    uint32_t keycode;
    uint32_t pressTime;
    uint32_t releaseTime;
};

static struct keystroke nginput[NKEYS]; // 入力バッファ
static struct keystroke ngingroup[NKEYS][NDOUJI]; // 入力バッファを同時押しの組み合わせでグループ化

static uint8_t doujiSize[NKEYS] = {0}; // ngingroupの配列のサイズ

#define NCOMBI 9                               // COMBI配列の大きさ
const uint8_t COMBINDEX[] = {0, 1, 3, NCOMBI}; // COMBI配列の各キー数の最初の位置

/*
  nginputをngingroupへ変換する
  同時押しの組み合わせを列挙している
  数字はninputの配列添字、-1はnull
*/
const int8_t COMBI[NCOMBI][NKEYS][NDOUJI] = {
    // 1 key
    {{0, -1, -1}, {-1, -1, -1}, {-1, -1, -1}}, // 0
    // 2 keys
    {{0, -1, -1}, {1, -1, -1}, {-1, -1, -1}}, // 0 1
    {{0, 1, -1}, {-1, -1, -1}, {-1, -1, -1}}, // 01
    // 3 keys
    {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}, // 0   1   2
    {{0, 1, -1}, {2, -1, -1}, {-1, -1, -1}}, // 01  2
    {{0, -1, -1}, {1, 2, -1}, {-1, -1, -1}}, // 0   12
    {{0, 1, 2}, {-1, -1, -1}, {-1, -1, -1}}, // 012
    {{0, 1, -1}, {0, 2, -1}, {-1, -1, -1}},  // 01  02 : 0が連続シフト
    {{0, 1, -1}, {1, 2, -1}, {-1, -1, -1}},  // 01  12 : 1が連続シフト
};

// カナ変換テーブル
typedef struct {
    uint32_t key;
    uint32_t kana[6];
} naginata_keymap;

#define NONE 0

const naginata_keymap ngmap[] = {
    // 清音
    {.key = B_J, .kana = {A, NONE, NONE, NONE, NONE, NONE}},            // あ
    {.key = B_K, .kana = {I, NONE, NONE, NONE, NONE, NONE}},            // い
    {.key = B_L, .kana = {U, NONE, NONE, NONE, NONE, NONE}},            // う
    {.key = B_SPACE | B_O, .kana = {E, NONE, NONE, NONE, NONE, NONE}},  // え
    {.key = B_SPACE | B_N, .kana = {O, NONE, NONE, NONE, NONE, NONE}},  // お
    {.key = B_F, .kana = {K, A, NONE, NONE, NONE, NONE}},               // か
    {.key = B_W, .kana = {K, I, NONE, NONE, NONE, NONE}},               // き
    {.key = B_H, .kana = {K, U, NONE, NONE, NONE, NONE}},               // く
    {.key = B_S, .kana = {K, E, NONE, NONE, NONE, NONE}},               // け
    {.key = B_V, .kana = {K, O, NONE, NONE, NONE, NONE}},               // こ
    {.key = B_SPACE | B_U, .kana = {S, A, NONE, NONE, NONE, NONE}},     // さ
    {.key = B_R, .kana = {S, I, NONE, NONE, NONE, NONE}},               // し
    {.key = B_O, .kana = {S, U, NONE, NONE, NONE, NONE}},               // す
    {.key = B_SPACE | B_A, .kana = {S, E, NONE, NONE, NONE, NONE}},     // せ
    {.key = B_B, .kana = {S, O, NONE, NONE, NONE, NONE}},               // そ
    {.key = B_N, .kana = {T, A, NONE, NONE, NONE, NONE}},               // た
    {.key = B_SPACE | B_G, .kana = {T, I, NONE, NONE, NONE, NONE}},     // ち
    {.key = B_SPACE | B_L, .kana = {T, U, NONE, NONE, NONE, NONE}},     // つ
    {.key = B_E, .kana = {T, E, NONE, NONE, NONE, NONE}},               // て
    {.key = B_D, .kana = {T, O, NONE, NONE, NONE, NONE}},               // と
    {.key = B_M, .kana = {N, A, NONE, NONE, NONE, NONE}},               // な
    {.key = B_SPACE | B_D, .kana = {N, I, NONE, NONE, NONE, NONE}},     // に
    {.key = B_SPACE | B_W, .kana = {N, U, NONE, NONE, NONE, NONE}},     // ぬ
    {.key = B_SPACE | B_R, .kana = {N, E, NONE, NONE, NONE, NONE}},     // ね
    {.key = B_SPACE | B_J, .kana = {N, O, NONE, NONE, NONE, NONE}},     // の
    {.key = B_C, .kana = {H, A, NONE, NONE, NONE, NONE}},               // は
    {.key = B_X, .kana = {H, I, NONE, NONE, NONE, NONE}},               // ひ
    {.key = B_SPACE | B_X, .kana = {H, I, NONE, NONE, NONE, NONE}},     // ひ
    {.key = B_SPACE | B_SEMI, .kana = {H, U, NONE, NONE, NONE, NONE}},  // ふ
    {.key = B_P, .kana = {H, E, NONE, NONE, NONE, NONE}},               // へ
    {.key = B_Z, .kana = {H, O, NONE, NONE, NONE, NONE}},               // ほ
    {.key = B_SPACE | B_Z, .kana = {H, O, NONE, NONE, NONE, NONE}},     // ほ
    {.key = B_SPACE | B_F, .kana = {M, A, NONE, NONE, NONE, NONE}},     // ま
    {.key = B_SPACE | B_B, .kana = {M, I, NONE, NONE, NONE, NONE}},     // み
    {.key = B_SPACE | B_COMMA, .kana = {M, U, NONE, NONE, NONE, NONE}}, // む
    {.key = B_SPACE | B_S, .kana = {M, E, NONE, NONE, NONE, NONE}},     // め
    {.key = B_SPACE | B_K, .kana = {M, O, NONE, NONE, NONE, NONE}},     // も
    {.key = B_SPACE | B_H, .kana = {Y, A, NONE, NONE, NONE, NONE}},     // や
    {.key = B_SPACE | B_P, .kana = {Y, U, NONE, NONE, NONE, NONE}},     // ゆ
    {.key = B_SPACE | B_I, .kana = {Y, O, NONE, NONE, NONE, NONE}},     // よ
    {.key = B_DOT, .kana = {R, A, NONE, NONE, NONE, NONE}},             // ら
    {.key = B_SPACE | B_E, .kana = {R, I, NONE, NONE, NONE, NONE}},     // り
    {.key = B_I, .kana = {R, U, NONE, NONE, NONE, NONE}},               // る
    {.key = B_SLASH, .kana = {R, E, NONE, NONE, NONE, NONE}},           // れ
    {.key = B_SPACE | B_SLASH, .kana = {R, E, NONE, NONE, NONE, NONE}}, // れ
    {.key = B_A, .kana = {R, O, NONE, NONE, NONE, NONE}},               // ろ
    {.key = B_SPACE | B_DOT, .kana = {W, A, NONE, NONE, NONE, NONE}},   // わ
    {.key = B_SPACE | B_C, .kana = {W, O, NONE, NONE, NONE, NONE}},     // を
    {.key = B_COMMA, .kana = {N, N, NONE, NONE, NONE, NONE}},           // ん
    {.key = B_SEMI, .kana = {MINUS, NONE, NONE, NONE, NONE, NONE}},     // ー

    // 濁音
    {.key = B_J | B_F, .kana = {G, A, NONE, NONE, NONE, NONE}},    // が
    {.key = B_J | B_W, .kana = {G, I, NONE, NONE, NONE, NONE}},    // ぎ
    {.key = B_F | B_H, .kana = {G, U, NONE, NONE, NONE, NONE}},    // ぐ
    {.key = B_J | B_S, .kana = {G, E, NONE, NONE, NONE, NONE}},    // げ
    {.key = B_J | B_V, .kana = {G, O, NONE, NONE, NONE, NONE}},    // ご
    {.key = B_F | B_U, .kana = {Z, A, NONE, NONE, NONE, NONE}},    // ざ
    {.key = B_J | B_R, .kana = {Z, I, NONE, NONE, NONE, NONE}},    // じ
    {.key = B_F | B_O, .kana = {Z, U, NONE, NONE, NONE, NONE}},    // ず
    {.key = B_J | B_A, .kana = {Z, E, NONE, NONE, NONE, NONE}},    // ぜ
    {.key = B_J | B_B, .kana = {Z, O, NONE, NONE, NONE, NONE}},    // ぞ
    {.key = B_F | B_N, .kana = {D, A, NONE, NONE, NONE, NONE}},    // だ
    {.key = B_J | B_G, .kana = {D, I, NONE, NONE, NONE, NONE}},    // ぢ
    {.key = B_F | B_L, .kana = {D, U, NONE, NONE, NONE, NONE}},    // づ
    {.key = B_J | B_E, .kana = {D, E, NONE, NONE, NONE, NONE}},    // で
    {.key = B_J | B_D, .kana = {D, O, NONE, NONE, NONE, NONE}},    // ど
    {.key = B_J | B_C, .kana = {B, A, NONE, NONE, NONE, NONE}},    // ば
    {.key = B_J | B_X, .kana = {B, I, NONE, NONE, NONE, NONE}},    // び
    {.key = B_F | B_SEMI, .kana = {B, U, NONE, NONE, NONE, NONE}}, // ぶ
    {.key = B_F | B_P, .kana = {B, E, NONE, NONE, NONE, NONE}},    // べ
    {.key = B_J | B_Z, .kana = {B, O, NONE, NONE, NONE, NONE}},    // ぼ
    {.key = B_F | B_L, .kana = {V, U, NONE, NONE, NONE, NONE}},    // ゔ

    // 半濁音
    {.key = B_M | B_C, .kana = {P, A, NONE, NONE, NONE, NONE}},    // ぱ
    {.key = B_M | B_X, .kana = {P, I, NONE, NONE, NONE, NONE}},    // ぴ
    {.key = B_V | B_SEMI, .kana = {P, U, NONE, NONE, NONE, NONE}}, // ぷ
    {.key = B_V | B_P, .kana = {P, E, NONE, NONE, NONE, NONE}},    // ぺ
    {.key = B_M | B_Z, .kana = {P, O, NONE, NONE, NONE, NONE}},    // ぽ

    // 小書き
    {.key = B_Q | B_H, .kana = {X, Y, A, NONE, NONE, NONE}},    // ゃ
    {.key = B_Q | B_P, .kana = {X, Y, U, NONE, NONE, NONE}},    // ゅ
    {.key = B_Q | B_I, .kana = {X, Y, O, NONE, NONE, NONE}},    // ょ
    {.key = B_Q | B_J, .kana = {X, A, NONE, NONE, NONE, NONE}}, // ぁ
    {.key = B_Q | B_K, .kana = {X, I, NONE, NONE, NONE, NONE}}, // ぃ
    {.key = B_Q | B_L, .kana = {X, U, NONE, NONE, NONE, NONE}}, // ぅ
    {.key = B_Q | B_O, .kana = {X, E, NONE, NONE, NONE, NONE}}, // ぇ
    {.key = B_Q | B_N, .kana = {X, O, NONE, NONE, NONE, NONE}}, // ぉ
    {.key = B_Q | B_DOT, .kana = {X, W, A, NONE, NONE, NONE}},  // ゎ
    {.key = B_G, .kana = {X, T, U, NONE, NONE, NONE}},          // っ
    {.key = B_Q | B_S, .kana = {X, K, E, NONE, NONE, NONE}},    // ヶ
    {.key = B_Q | B_F, .kana = {X, K, A, NONE, NONE, NONE}},    // ヵ

    // 清音拗音 濁音拗音 半濁拗音
    {.key = B_R | B_H, .kana = {S, Y, A, NONE, NONE, NONE}},       // しゃ
    {.key = B_R | B_P, .kana = {S, Y, U, NONE, NONE, NONE}},       // しゅ
    {.key = B_R | B_I, .kana = {S, Y, O, NONE, NONE, NONE}},       // しょ
    {.key = B_J | B_R | B_H, .kana = {Z, Y, A, NONE, NONE, NONE}}, // じゃ
    {.key = B_J | B_R | B_P, .kana = {Z, Y, U, NONE, NONE, NONE}}, // じゅ
    {.key = B_J | B_R | B_I, .kana = {Z, Y, O, NONE, NONE, NONE}}, // じょ
    {.key = B_W | B_H, .kana = {K, Y, A, NONE, NONE, NONE}},       // きゃ
    {.key = B_W | B_P, .kana = {K, Y, U, NONE, NONE, NONE}},       // きゅ
    {.key = B_W | B_I, .kana = {K, Y, O, NONE, NONE, NONE}},       // きょ
    {.key = B_J | B_W | B_H, .kana = {G, Y, A, NONE, NONE, NONE}}, // ぎゃ
    {.key = B_J | B_W | B_P, .kana = {G, Y, U, NONE, NONE, NONE}}, // ぎゅ
    {.key = B_J | B_W | B_I, .kana = {G, Y, O, NONE, NONE, NONE}}, // ぎょ
    {.key = B_G | B_H, .kana = {T, Y, A, NONE, NONE, NONE}},       // ちゃ
    {.key = B_G | B_P, .kana = {T, Y, U, NONE, NONE, NONE}},       // ちゅ
    {.key = B_G | B_I, .kana = {T, Y, O, NONE, NONE, NONE}},       // ちょ
    {.key = B_J | B_G | B_H, .kana = {D, Y, A, NONE, NONE, NONE}}, // ぢゃ
    {.key = B_J | B_G | B_P, .kana = {D, Y, U, NONE, NONE, NONE}}, // ぢゅ
    {.key = B_J | B_G | B_I, .kana = {D, Y, O, NONE, NONE, NONE}}, // ぢょ
    {.key = B_D | B_H, .kana = {N, Y, A, NONE, NONE, NONE}},       // にゃ
    {.key = B_D | B_P, .kana = {N, Y, U, NONE, NONE, NONE}},       // にゅ
    {.key = B_D | B_I, .kana = {N, Y, O, NONE, NONE, NONE}},       // にょ
    {.key = B_X | B_H, .kana = {H, Y, A, NONE, NONE, NONE}},       // ひゃ
    {.key = B_X | B_P, .kana = {H, Y, U, NONE, NONE, NONE}},       // ひゅ
    {.key = B_X | B_I, .kana = {H, Y, O, NONE, NONE, NONE}},       // ひょ
    {.key = B_J | B_X | B_H, .kana = {B, Y, A, NONE, NONE, NONE}}, // びゃ
    {.key = B_J | B_X | B_P, .kana = {B, Y, U, NONE, NONE, NONE}}, // びゅ
    {.key = B_J | B_X | B_I, .kana = {B, Y, O, NONE, NONE, NONE}}, // びょ
    {.key = B_M | B_X | B_H, .kana = {P, Y, A, NONE, NONE, NONE}}, // ぴゃ
    {.key = B_M | B_X | B_P, .kana = {P, Y, U, NONE, NONE, NONE}}, // ぴゅ
    {.key = B_M | B_X | B_I, .kana = {P, Y, O, NONE, NONE, NONE}}, // ぴょ
    {.key = B_B | B_H, .kana = {M, Y, A, NONE, NONE, NONE}},       // みゃ
    {.key = B_B | B_P, .kana = {M, Y, U, NONE, NONE, NONE}},       // みゅ
    {.key = B_B | B_I, .kana = {M, Y, O, NONE, NONE, NONE}},       // みょ
    {.key = B_E | B_H, .kana = {R, Y, A, NONE, NONE, NONE}},       // りゃ
    {.key = B_E | B_P, .kana = {R, Y, U, NONE, NONE, NONE}},       // りゅ
    {.key = B_E | B_I, .kana = {R, Y, O, NONE, NONE, NONE}},       // りょ

    // 清音外来音 濁音外来音
    {.key = B_M | B_E | B_K, .kana = {T, H, I, NONE, NONE, NONE}},       // てぃ
    {.key = B_M | B_E | B_P, .kana = {T, H, U, NONE, NONE, NONE}},       // てゅ
    {.key = B_J | B_E | B_K, .kana = {D, H, I, NONE, NONE, NONE}},       // でぃ
    {.key = B_J | B_E | B_P, .kana = {D, H, U, NONE, NONE, NONE}},       // でゅ
    {.key = B_M | B_D | B_L, .kana = {T, O, X, U, NONE, NONE}},          // とぅ
    {.key = B_J | B_D | B_L, .kana = {D, O, X, U, NONE, NONE}},          // どぅ
    {.key = B_M | B_R | B_O, .kana = {S, Y, E, NONE, NONE, NONE}},       // しぇ
    {.key = B_M | B_G | B_O, .kana = {T, Y, E, NONE, NONE, NONE}},       // ちぇ
    {.key = B_J | B_R | B_O, .kana = {Z, Y, E, NONE, NONE, NONE}},       // じぇ
    {.key = B_J | B_G | B_O, .kana = {D, Y, E, NONE, NONE, NONE}},       // ぢぇ
    {.key = B_V | B_SEMI | B_J, .kana = {F, A, NONE, NONE, NONE, NONE}}, // ふぁ
    {.key = B_V | B_SEMI | B_K, .kana = {F, I, NONE, NONE, NONE, NONE}}, // ふぃ
    {.key = B_V | B_SEMI | B_O, .kana = {F, E, NONE, NONE, NONE, NONE}}, // ふぇ
    {.key = B_V | B_SEMI | B_N, .kana = {F, O, NONE, NONE, NONE, NONE}}, // ふぉ
    {.key = B_V | B_SEMI | B_P, .kana = {F, Y, U, NONE, NONE, NONE}},    // ふゅ
    {.key = B_V | B_K | B_O, .kana = {I, X, E, NONE, NONE, NONE}},       // いぇ
    {.key = B_V | B_L | B_K, .kana = {W, I, NONE, NONE, NONE, NONE}},    // うぃ
    {.key = B_V | B_L | B_O, .kana = {W, E, NONE, NONE, NONE, NONE}},    // うぇ
    {.key = B_V | B_L | B_N, .kana = {U, X, O, NONE, NONE, NONE}},       // うぉ
    {.key = B_M | B_Q | B_J, .kana = {V, A, NONE, NONE, NONE, NONE}},    // ゔぁ
    {.key = B_M | B_Q | B_K, .kana = {V, I, NONE, NONE, NONE, NONE}},    // ゔぃ
    {.key = B_M | B_Q | B_O, .kana = {V, E, NONE, NONE, NONE, NONE}},    // ゔぇ
    {.key = B_M | B_Q | B_N, .kana = {V, O, NONE, NONE, NONE, NONE}},    // ゔぉ
    {.key = B_M | B_Q | B_P, .kana = {V, U, X, Y, U, NONE}},             // ゔゅ
    {.key = B_V | B_H | B_J, .kana = {K, U, X, A, NONE, NONE}},          // くぁ
    {.key = B_V | B_H | B_K, .kana = {K, U, X, I, NONE, NONE}},          // くぃ
    {.key = B_V | B_H | B_O, .kana = {K, U, X, E, NONE, NONE}},          // くぇ
    {.key = B_V | B_H | B_N, .kana = {K, U, X, O, NONE, NONE}},          // くぉ
    {.key = B_V | B_H | B_DOT, .kana = {K, U, X, W, A, NONE}},           // くゎ
    {.key = B_F | B_H | B_J, .kana = {G, U, X, A, NONE, NONE}},          // ぐぁ
    {.key = B_F | B_H | B_K, .kana = {G, U, X, I, NONE, NONE}},          // ぐぃ
    {.key = B_F | B_H | B_O, .kana = {G, U, X, E, NONE, NONE}},          // ぐぇ
    {.key = B_F | B_H | B_N, .kana = {G, U, X, O, NONE, NONE}},          // ぐぉ
    {.key = B_F | B_H | B_DOT, .kana = {G, U, X, W, A, NONE}},           // ぐゎ
    {.key = B_V | B_L | B_J, .kana = {T, S, A, NONE, NONE, NONE}},       // つぁ

    // 追加
    {.key = B_SPACE, .kana = {SPACE, NONE, NONE, NONE, NONE, NONE}},
    {.key = B_V | B_SPACE, .kana = {COMMA, ENTER, NONE, NONE, NONE, NONE}},
    {.key = B_U, .kana = {BACKSPACE, NONE, NONE, NONE, NONE, NONE}},

    // enter
    {.key = B_V | B_M, .kana = {ENTER, NONE, NONE, NONE, NONE, NONE}},
    // enter+シフト(連続シフト)
    {.key = B_SPACE | B_V | B_M, .kana = {ENTER, NONE, NONE, NONE, NONE, NONE}},

    // // 非標準の変換
    // {.key = B_X | B_C | B_M, .kana = "pyu"}, // ピュ

    // 別途処理しないといけない変換
    {.key = B_T, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}},           //
    {.key = B_Y, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}},           //
    {.key = B_SPACE | B_T, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}}, //
    {.key = B_SPACE | B_Y, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}}, //
    {.key = B_H | B_J, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}},     // 　かなオン
    {.key = B_F | B_G, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}},     // 　かなオフ

};

uint32_t scoring(struct keystroke ks[], uint8_t size) {
    uint16_t now = k_uptime_get();

    if (size == 1) { // 単打の重み
        return 100; // 単打を優先するか、同時押しを優先するかをチューニングする
    }

    // 前置シフト
    // if (!naginata_config.kouchi_shift &&
    //     (ks[1].keycode == SPACE || ks[1].keycode == SPACE2)) {
    //     return 0;
    // }

    // 点数=キー同士が重なる時間を、それぞれのキーを押している時間で割る
    uint16_t s2 = ks[0].pressTime;
    uint16_t e2 = ks[0].releaseTime > 0 ? ks[0].releaseTime : now;
    for (uint8_t i = 1; i < size; i++) {
        if (ks[i].pressTime > s2) {
            s2 = ks[i].pressTime;
        }
        if (ks[i].releaseTime > 0 && ks[i].releaseTime < e2) {
            e2 = ks[i].releaseTime;
        }
    }
    uint32_t w = e2 - s2; // キーが重なっている時間、uint16_tにするとおかしくなる？
    uint32_t s = 0;
    if (s2 < e2) {
        for (uint8_t i = 0; i < size; i++) {
            s += w * 1000 / ((ks[i].releaseTime > 0 ? ks[i].releaseTime : now) - ks[i].pressTime);
        }
    }

    return s;
}

// ngingroupを作って中で一番評価値が高い組み合わせngingroupに入れる
uint8_t evaluate() {

    struct keystroke tmpgroup[NKEYS][NDOUJI]; // 入力バッファを同時押しの組み合わせでグループ化
    uint32_t score = 0;
    uint32_t maxScore = 0;
    uint8_t tdoujiSize[NKEYS] = {0}; // ngingroupの配列のサイズ

    // naginata_keymap bngmap; // PROGMEM buffer
    // int8_t bcombi = 0;      // PROGMEM buffer
    bool isExist = false;
    uint8_t keySize = 0;

    for (uint8_t i = COMBINDEX[ng_chrcount - 1]; i < COMBINDEX[ng_chrcount];
         i++) { // 組み合わせごとに
        uint8_t tkeySize = 0;
        for (uint8_t j = 0; j < NKEYS; j++) { // 組み合わせの順番に
            // memcpy_P(&bcombi, &COMBI[i][j][0], sizeof(bcombi));
            if (COMBI[i][j][0] == -1) {
                break;
            }

            // ngingroupを作る
            uint8_t tdouji = 0;
            for (uint8_t k = 0; k < NDOUJI; k++) { // 同時に押しているキー
                // memcpy_P(&bcombi, &COMBI[i][j][k], sizeof(bcombi));
                if (COMBI[i][j][k] == -1) {
                    break;
                } else {
                    tmpgroup[j][k] = nginput[COMBI[i][j][k]];
                    tdouji++;
                }
            }
            tdoujiSize[j] = tdouji; // あとで辞書にない可能性もあるけど、オーバーライトされるか

            // バッファ内のキーを組み合わせる
            uint32_t keycomb_buf = 0UL;
            for (uint8_t k = 0; k < tdouji; k++) {
                keycomb_buf |= ng_key[tmpgroup[j][k].keycode - A];
            }
            // 辞書に存在するかチェック
            isExist = false;
            for (uint16_t k = 0; k < sizeof ngmap / sizeof ngmap[0]; k++) {
                // memcpy_P(&bngmap, &ngmap[k], sizeof(bngmap));
                if (keycomb_buf == ngmap[k].key) {
                    isExist = true;
                    break;
                }
            }
            if (!isExist) {
                break; // 辞書になければ追加しない
            }
            tkeySize++;
        } // j

        if (!isExist) {
            continue; // 辞書になければ追加しない
        }

        uint32_t s = 0;
        for (uint8_t j = 0; j < tkeySize; j++) {
            s += scoring(tmpgroup[j], tdoujiSize[j]);
        }
        score = s / tkeySize;

        if (score >= maxScore) {
            maxScore = score;
            keySize = tkeySize;
            for (uint8_t j = 0; j < keySize; j++) {
                doujiSize[j] = tdoujiSize[j];
                for (uint8_t k = 0; k < doujiSize[j]; k++) {
                    ngingroup[j][k] = tmpgroup[j][k];
                }
            }
        }
    }

    return keySize;
}

// キー入力を文字に変換して出力する
// partial 部分変換するかどうか
// 戻り値 変換処理したキーの数
uint8_t naginata_type(bool partial, int32_t timestamp) {

    // if (ng_chrcount == 1 && nginput[0].keycode == NG_SPACE2) {
    //     tap_code(KC_ENT);
    //     return 1;
    // }

    // naginata_keymap bngmap; // PROGMEM buffer

    uint8_t keySize = evaluate();

    // かなへ変換する
    if (partial)
        keySize = 1; // partialの時は、最初の組み合わせだけ出力する

    for (uint8_t i = 0; i < keySize; i++) {
        // バッファ内のキーを組み合わせる
        uint32_t keycomb_buf = 0UL;
        for (uint8_t j = 0; j < doujiSize[i]; j++) {
            keycomb_buf |= ng_key[ngingroup[i][j].keycode - A];
        }
        switch (keycomb_buf) {
        // case B_T:
        //     ng_left(1);
        //     break;
        // case B_Y:
        //     ng_right(1);
        //     break;
        // case B_SPACE | B_T:
        //     register_code(KC_LSFT);
        //     ng_left(1);
        //     unregister_code(KC_LSFT);
        //     break;
        // case B_SPACE | B_Y:
        //     register_code(KC_LSFT);
        //     ng_right(1);
        //     unregister_code(KC_LSFT);
        //     break;
        // case B_H | B_J:
        //     naginata_on();
        //     break;
        // case B_F | B_G:
        //     naginata_off();
        //     break;
        default:
            for (uint16_t j = 0; j < sizeof ngmap / sizeof ngmap[0]; j++) {
                if (keycomb_buf == ngmap[j].key) {
                    for (uint8_t k = 0; k < 6; k++) {
                        if (ngmap[j].kana[k] == NONE)
                            break;
                        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ngmap[j].kana[k],
                                                                               true, timestamp));
                        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ngmap[j].kana[k],
                                                                               false, timestamp));
                    }
                    break;
                }
            }
            break;
        }
    }

    if (partial) {
        return doujiSize[0];
    } else {
        return ng_chrcount;
    }
}

// 薙刀式の入力処理
bool naginata_press(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;

    // まれに薙刀モードオンのまま、レイヤーがオフになることがあるので、対策
    // if (n_modifier == 0 && is_naginata && !layer_state_is(naginata_layer))
    //     layer_on(naginata_layer);
    // if (n_modifier == 0 && !is_naginata && layer_state_is(naginata_layer))
    //     layer_off(naginata_layer);
    // if (n_modifier > 0 && layer_state_is(naginata_layer))
    //     layer_off(naginata_layer);

    // OS切り替え(UNICODE出力)
    // if (record->event.pressed) {
    //     switch (keycode) {
    //     case NG_ON:
    //         naginata_on();
    //         return false;
    //         break;
    //     case NG_OFF:
    //         naginata_off();
    //         return false;
    //         break;
    //     case NG_CLR:
    //         naginata_clear();
    //         return false;
    //         break;
    //     case NGSW_WIN:
    //         switchOS(NG_WIN);
    //         return false;
    //         break;
    //     case NGSW_MAC:
    //         switchOS(NG_MAC);
    //         return false;
    //         break;
    //     case NGSW_LNX:
    //         switchOS(NG_LINUX);
    //         return false;
    //         break;
    //     case NG_MLV:
    //         mac_live_conversion_toggle();
    //         return false;
    //         break;
    //     case NG_SHOS:
    //         ng_show_os();
    //         return false;
    //         break;
    //     case NG_TAYO:
    //         tategaki_toggle();
    //         return false;
    //         break;
    //     case NG_KOTI:
    //         kouchi_shift_toggle();
    //         return false;
    //         break;
    //     }
    // }
    // if (!is_naginata)
    //     return enable_naginata(keycode, record);

    // if (process_modifier(keycode, record))
    //     return true;

    switch (keycode) {
    case A ... Z:
    case SPACE:
    case DOT:
    case COMMA:
    case SLASH:
    case SEMI:
        keycnt++;

        if (keycnt > NKEYS || ng_chrcount >= NKEYS) {
            int ntyped = naginata_type(true, event.timestamp);

            // 押しているキーは残す
            // シフト系のキー以外を残すと2度変換してしまう
            uint8_t tch = 0;
            for (uint8_t i = 0; i < ntyped; i++) {
                if (nginput[i].releaseTime == 0 &&
                    (nginput[i].keycode == SPACE || nginput[i].keycode == F ||
                     nginput[i].keycode == V || nginput[i].keycode == J ||
                     nginput[i].keycode == M)) {
                    nginput[tch] = nginput[i];
                    if (ntyped < ng_chrcount) { // 仕切り直す
                        nginput[tch].pressTime = nginput[ntyped].pressTime;
                    } else {
                        nginput[tch].pressTime = event.timestamp;
                    }
                    tch++;
                    break; // キャリーオーバーするのも１キーだけ
                }
            }
            for (uint8_t i = ntyped; i < ng_chrcount; i++) {
                nginput[tch] = nginput[i];
                tch++;
            }
            ng_chrcount = tch;
            keycomb = 0UL;
            for (uint8_t i = 0; i < ng_chrcount; i++) {
                keycomb |= ng_key[nginput[i].keycode];
            }
        }

        keycomb |= ng_key[keycode - A]; // キーの重ね合わせ
        // if (is_henshu) {
        //     if (exec_henshu(keycomb))
        //         return true;
        // }

        // is_henshu = check_henshu();

        nginput[ng_chrcount] = (struct keystroke){.keycode = keycode,
                                                  .pressTime = event.timestamp,
                                                  .releaseTime = 0}; // キー入力をバッファに貯める
        ng_chrcount++;

        return false;
        break;
    }

    return true;
}

bool naginata_release(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;

    switch (keycode) {
    case A ... Z:
    case SPACE:
    case DOT:
    case COMMA:
    case SLASH:
    case SEMI:
        if (keycnt > 0)
            keycnt--;

        keycomb &= ~ng_key[keycode - A]; // キーの重ね合わせ
        // is_henshu = check_henshu();

        for (uint8_t i = 0; i < ng_chrcount; i++) { // 　連続シフト　もも
            if (keycode == nginput[i].keycode && nginput[i].releaseTime == 0) {
                nginput[i].releaseTime = event.timestamp;
                break;
            }
        }
        // 全部キーを離したら
        if (keycnt == 0 && ng_chrcount > 0) {
            // if (henshu_done) { // 編集モードを実行した後のDF等は変換しない
            //     henshu_done = false;
            //     ng_chrcount = 0;
            //     keycomb = 0;
            //     return false;
            // }
            naginata_type(false, event.timestamp);
            ng_chrcount = 0;
            keycomb = 0;
        }
        return false;
        break;
    }
    return true;
}

// 薙刀式

static int behavior_naginata_init(const struct device *dev) { return 0; };

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);

    naginata_press(binding, event);
    return ZMK_BEHAVIOR_OPAQUE;
    // return ZMK_EVENT_RAISE(
    //     zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp));
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);

    naginata_release(binding, event);

    return ZMK_BEHAVIOR_OPAQUE;
    // return ZMK_EVENT_RAISE(
    //     zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp));
}

static const struct behavior_driver_api behavior_naginata_driver_api = {
    .binding_pressed = on_keymap_binding_pressed, .binding_released = on_keymap_binding_released};

#define KP_INST(n)                                                                                 \
    DEVICE_DT_INST_DEFINE(n, behavior_naginata_init, NULL, NULL, NULL, APPLICATION,                \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_naginata_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)
