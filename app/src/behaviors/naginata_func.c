
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>
#include "naginata_func.h"

uint32_t timestamp;

#define NG_WIN 0
#define NG_MAC 1
#define NG_LINUX 2
#define NG_IOS 3

typedef union {
    uint8_t os : NG_MAC;
    bool live_conv : true;
    bool tategaki : true;
    bool kouchi_shift : true;
} user_config_t;

user_config_t naginata_config;

// 薙刀式をオン
// void naginata_on(void) {
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, true, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT4, true, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT4, false, timestamp));
// }

// 薙刀式をオフ
// void naginata_off(void) {
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG2, true, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG2, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT5, true, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT5, false, timestamp));
// }

void nofunc() {}

void ng_T() { ng_left(1); }

void ng_Y() { ng_right(1); }

void ng_ST() {
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_left(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ng_SY() {
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_right(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_JKQ() { // ^{End}
    ng_eof();
}

// void ngh_JKW() { // 『』{改行}{↑}
//     ng_send_unicode_string_P(PSTR("『』"));
//     ng_up(1);
// }

void ngh_JKE() { // ディ
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(D, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(D, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(H, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(H, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(I, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(I, false, timestamp));
}

void ngh_JKR() { // ^s
    ng_save();
}

// void ngh_JKT() { // ・
//     ng_send_unicode_string_P(PSTR("・"));
// }

// void ngh_JKA() { // ……{改行}
//     ng_send_unicode_string_P(PSTR("……"));
// }

// void ngh_JKS() { // (){改行}{↑}
//     ng_send_unicode_string_P(PSTR("()"));
//     ng_up(1);
// }

void ngh_JKD() { // ？{改行}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LS(SLASH), true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LS(SLASH), false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
}

// void ngh_JKF() { // 「」{改行}{↑}
//     ng_send_unicode_string_P(PSTR("「」"));
//     ng_up(1);
// }

// void ngh_JKG() { // 《》{改行}{↑}
//     ng_send_unicode_string_P(PSTR("《》"));
//     ng_up(1);
// }

// void ngh_JKZ() { // ――{改行}
//     ng_send_unicode_string_P(PSTR("――"));
// }

// void ngh_JKX() { // 【】{改行}{↑}
//     ng_send_unicode_string_P(PSTR("【】"));
//     ng_up(1);
// }

void ngh_JKC() { // ！{改行}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LS(N1), true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
}

void ngh_JKV() { // {改行}{↓}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ng_down(1);
}

void ngh_JKB() { // {改行}{←}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ng_left(1);
}

void ngh_DFY() { // {Home}
    ng_home();
}

void ngh_DFU() { // +{End}{BS}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_end();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(BSPC, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(BSPC, false, timestamp));
}

void ngh_DFI() { // {vk1Csc079}
    ng_saihenkan();
}

void ngh_DFO() { // {Del}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
}

void ngh_DFP() { // {Esc 3}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ESC, false, timestamp));
}

void ngh_DFH() { // {Enter}{End}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ng_end();
}

void ngh_DFJ() { // {↑}
    ng_up(1);
}

void ngh_DFK() { // +{↑}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_up(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_DFL() { // +{↑ 7}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_up(7);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_DFSCLN() { // ^i
    ng_katakana();
}

void ngh_DFN() { // {End}
    ng_end();
}

void ngh_DFM() { // {↓}
    ng_down(1);
}

void ngh_DFCOMM() { // +{↓}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_down(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_DFDOT() { // +{↓ 7}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_down(7);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_DFSLSH() { // ^u
    ng_hiragana();
}

void ngh_MCQ() { // {Home}{→}{End}{Del 4}{←}
    ng_home();
    ng_right(1);
    ng_end();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ng_left(1);
}

// void ngh_MCW() { // ^x『^v』{改行}{Space}+{↑}^x
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("『"));
//     ng_paste();
//     ng_send_unicode_string_P(PSTR("』"));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encodSPACETER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_up(1);
//     unZMK_EVENT_RAISE(LEFTeycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_cut();
// }

void ngh_MCE() { // {Home}{改行}{Space 3}{←}
    ng_home();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ng_left(1);
}

void ngh_MCR() { // {Space 3}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
}

// void ngh_MCT() { // 〇{改行}
//     ng_send_unicode_string_P(PSTR("〇"));
// }

void ngh_MCA() { // {Home}{→}{End}{Del 2}{←}
    ng_home();
    ng_right(1);
    ng_end();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DELETE, false, timestamp));
    ng_left(1);
}

// void ngh_MCS() { // ^x(^v){改行}{Space}+{↑}^x
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("("));
//     ng_paste();
//     ng_send_unicode_string_P(PSTR(")"));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encodSPACETER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_up(1);
//     unZMK_EVENT_RAISE(LEFTeycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_cut();
// }

void ngh_MCD() { // {Home}{改行}{Space 1}{←}
    ng_home();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
    ng_left(1);
}

// void ngh_MCF() { // ^x「^v」{改行}{Space}+{↑}^x
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("「"));
//     ng_paste();
//     ng_send_unicode_string_P(PSTR("」"));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encodSPACETER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_up(1);
//     unZMK_EVENT_RAISE(LEFTeycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_cut();
// }

// void ngh_MCG() { // ^x｜{改行}^v《》{改行}{↑}{Space}+{↑}^x
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("｜"));
//     ng_paste();
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encodSPACETER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_up(1);
// unZMK_EVENT_RAISE(LEFTeycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("《》"));
//     ng_up(1);
// }

// void ngh_MCZ() { // 　　　×　　　×　　　×{改行 2}
//     ng_send_unicode_string_P(PSTR("　　　×　　　×　　　×"));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
// }

// void ngh_MCX() { // ^x【^v】{改行}{Space}+{↑}^x
//     ng_cut();
//     ng_send_unicode_string_P(PSTR("【"));
//     ng_paste();
//     ng_send_unicode_string_P(PSTR("】"));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encodSPACETER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_up(1);
//     unZMK_EVENT_RAISE(LEFTeycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
//     ng_cut();
// }

// void ngh_MCC() { // ／{改行}
//     ng_send_unicode_string_P(PSTR("／"));
// }

// void ngh_MCV() { // {改行}{End}{改行}「」{改行}{↑}
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
//     ng_end();
//     ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
// ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
//     ng_send_unicode_string_P(PSTR("「」"));
//     ng_up(1);
// }

void ngh_MCB() { // {改行}{End}{改行}{Space}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ng_end();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp));
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp));
}

void ngh_CVY() { // +{Home}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_home();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVU() { // ^x
    ng_cut();
}

void ngh_CVI() { // ^v
    ng_paste();
}

void ngh_CVO() { // ^y
    ng_redo();
}

void ngh_CVP() { // ^z
    ng_undo();
}

void ngh_CVH() { // ^c
    ng_copy();
}

void ngh_CVJ() { // {→}
    ng_right(1);
}

void ngh_CVK() { // +{→}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_right(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVL() { // +{→ 5}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_right(5);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVSCLN() { // +{→ 20}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_right(20);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVN() { // +{End}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_end();
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVM() { // {←}
    ng_left(1);
}

void ngh_CVCOMM() { // +{←}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_left(1);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVDOT() { // +{← 5}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_left(5);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ngh_CVSLSH() { // +{← 20}
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, true, timestamp));
    ng_left(20);
    ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LSHIFT, false, timestamp));
}

void ng_cut() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(X), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(X), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(X), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(X), false, timestamp));
        break;
    }
}

void ng_copy() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(C), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(C), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(C), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(C), false, timestamp));
        break;
    }
}

void ng_paste() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(V), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(V), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(V), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(V), false, timestamp));
        break;
    }
}

void ng_up(uint8_t c) {
    for (uint8_t i = 0; i < c; i++) { // サイズ削減
        if (naginata_config.tategaki) {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(UP, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(UP, false, timestamp));
        } else {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LEFT, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LEFT, false, timestamp));
        }
    }
}

void ng_down(uint8_t c) {
    for (uint8_t i = 0; i < c; i++) {
        if (naginata_config.tategaki) {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DOWN, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DOWN, false, timestamp));
        } else {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(RIGHT, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(RIGHT, false, timestamp));
        }
    }
}

void ng_left(uint8_t c) {
    for (uint8_t i = 0; i < c; i++) {
        if (naginata_config.tategaki) {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LEFT, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LEFT, false, timestamp));
        } else {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DOWN, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(DOWN, false, timestamp));
        }
    }
}

void ng_right(uint8_t c) {
    for (uint8_t i = 0; i < c; i++) {
        if (naginata_config.tategaki) {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(RIGHT, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(RIGHT, false, timestamp));
        } else {
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(UP, true, timestamp));
            ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(UP, false, timestamp));
        }
    }
}

void ng_home() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(HOME, true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(HOME, false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(A), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(A), false, timestamp));
        break;
    }
}

void ng_end() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(END, true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(END, false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(E), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(E), false, timestamp));
        break;
    }
}

void ng_katakana() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(I), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(I), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(K), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(K), false, timestamp));
        break;
    }
}

void ng_save() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(S), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(S), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(S), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(S), false, timestamp));
        break;
    }
}

void ng_hiragana() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(U), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(U), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(J), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(J), false, timestamp));
        break;
    }
}

void ng_redo() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(Y), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(Y), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LS(LG((S))), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LS(LG((S))), false, timestamp));
        break;
    }
}

void ng_undo() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(Z), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(Z), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(Z), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(Z), false, timestamp));
        break;
    }
}

void ng_saihenkan() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT4, true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(INT4, false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, false, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LANG1, false, timestamp));
        break;
    }
}

void ng_eof() {
    switch (naginata_config.os) {
    case NG_WIN:
    case NG_LINUX:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(END), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LC(END), false, timestamp));
        break;
    case NG_MAC:
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(DOWN), true, timestamp));
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(LG(DOWN), false, timestamp));
        break;
    }
}
