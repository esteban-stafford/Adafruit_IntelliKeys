/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (thach@tinyusb.org) for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "IKOverlay.h"
#include "class/hid/hid.h"

#define IK_DEBUG 1

#if IK_DEBUG
#include <Arduino.h>
#define IK_PRINTF(...) serial1_printf(__VA_ARGS__)
#else
#define IK_PRINTF(...)
#endif

IKOverlay stdOverlays[7];

IKOverlay::IKOverlay() { memset(_membrane, 0, sizeof(_membrane)); }

void IKOverlay::getSwitchReport(int nswitch, ik_report_t *report) {}

void IKOverlay::getMembraneReport(int row, int col, ik_report_t *report) {
  if (row > IK_RESOLUTION_X || col > IK_RESOLUTION_Y) {
    return;
  }
  *report = _membrane[row][col];
}

void IKOverlay::setMembraneReport(int top_row, int top_col, int height,
                                  int width, ik_report_t *report) {
  if (!(top_row < IK_RESOLUTION_X && top_col < IK_RESOLUTION_Y)) {
    IK_PRINTF("Invalid top row or top col [%u, %u]\r\n", top_row, top_col);
    return;
  }

  if (!((top_row + height <= IK_RESOLUTION_X) &&
        (top_col + width <= IK_RESOLUTION_Y))) {
    IK_PRINTF("Invalid height or width [%u, %u] + [%u, %u] = [%u, %u]\r\n",
              top_row, top_col, height, width, top_row + height,
              top_col + width);
    return;
  }

  for (int row = top_row; row < top_row + height; row++) {
    for (int col = top_col; col < top_col + width; col++) {
      _membrane[row][col] = *report;
    }
  }
}

void IKOverlay::initStandardOverlays(void) {
  initStdWebAccess();
  initStdMathAccess();
  initStdAlphabet();
  initStdQwerty();
  initStdBasicWriting();
}

//--------------------------------------------------------------------+
// Web Access
//--------------------------------------------------------------------+
void IKOverlay::initStdWebAccess(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_WEB_ACCESS];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col;
  int const height = 3;
  int const width = 2;

  //------------- first row -------------//
  row = 0;
  col = 0;

  uint8_t const first_row[][2] = {
      {KEYBOARD_MODIFIER_LEFTALT, HID_KEY_ARROW_LEFT},  // backward in browser
      {KEYBOARD_MODIFIER_LEFTALT, HID_KEY_ARROW_RIGHT}, // forward in browser
      {0, HID_KEY_ESCAPE},                              // stop
      {0, HID_KEY_F5},                                  // refresh
      {KEYBOARD_MODIFIER_LEFTALT, HID_KEY_HOME},        // open home page
      {0, HID_KEY_F3},                                  // search
      {0, 0},                                           // TODO bookmark ?
      {KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_H},          // History with Ctrl+H
      {KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_P},          // Print with Ctrl+P
      {KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C},          // Copy with Ctrl+C
      {0, 0}, // TODO Internet Explorer: Launch default browser ?
      {0, 0}, // TODO Netscape ??
  };

  overlay.setMembraneKeyboardRow(row, col, height, width, first_row,
                                 sizeof(first_row) / sizeof(first_row[0]));

  //------------- second row -------------//
  row = 3;
  col = 0;

  uint8_t const second_row[][2] = {
      {0, HID_KEY_TAB},
      {0, HID_KEY_SLASH},
      {0, HID_KEY_GRAVE},
      {0, 0},                                  // empty
      {KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_L}, // goto address bar
                                               // www.
                                               // .com
                                               // .net
                                               // .gov
                                               // .edu
                                               // .org
                                               // IntelliTools ?
  };

  overlay.setMembraneKeyboardRow(row, col, height, width, second_row,
                                 sizeof(second_row) / sizeof(second_row[0]));

  // Row 3 to 8
  initStdQwertyRow3to8(overlay);
}

//--------------------------------------------------------------------+
// Math Access
//--------------------------------------------------------------------+
void IKOverlay::initStdMathAccess(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_MATH_ACCESS];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col, height, width;

  //------------- Calculator -------------//
  height = 6;
  width = 4;

  uint8_t const numbpad[] = {
      HID_KEY_KEYPAD_7,  HID_KEY_KEYPAD_8, HID_KEY_KEYPAD_9,
      HID_KEY_KEYPAD_4,  HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6,
      HID_KEY_KEYPAD_1,  HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3,
      HID_KEY_BACKSPACE, HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_ENTER};

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      row = i * 6;
      col = j * 4;
      report.keyboard.keycode = numbpad[i * 3 + j];
      overlay.setMembraneReport(row, col, height, width, &report);
    }
  }

  //------------- Left Pad -------------//
  height = 3;
  width = 2;

  // row 1
  row = 0;
  col = 14;

  uint8_t const first_row[][2] = {{0, HID_KEY_KEYPAD_ADD},
                                  {0, HID_KEY_KEYPAD_SUBTRACT},
                                  {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_4},
                                  {0, HID_KEY_ARROW_LEFT},
                                  {0, HID_KEY_ARROW_RIGHT}};

  overlay.setMembraneKeyboardRow(row, col, height, width, first_row,
                                 sizeof(first_row) / sizeof(first_row[0]));

  // row 2
  row = 3;
  col = 14;

  uint8_t const second_row[][2] = {{0, HID_KEY_KEYPAD_MULTIPLY},
                                   {0, HID_KEY_KEYPAD_DIVIDE},
                                   {0, HID_KEY_KEYPAD_DECIMAL},
                                   {0, HID_KEY_ARROW_UP},
                                   {0, HID_KEY_ARROW_DOWN}};

  overlay.setMembraneKeyboardRow(row, col, height, width, second_row,
                                 sizeof(second_row) / sizeof(second_row[0]));

  // row 3
  row = 6;
  col = 14;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_KEYPAD_EQUAL;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.keycode = HID_KEY_TAB;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 4
  row = 9;
  col = 14;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_CLEAR;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);
  col += 2 * width;

  report.keyboard.keycode = HID_KEY_ESCAPE;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += 2 * width;

  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;

  // TODO row 5 to 8 is Mouse
}

//--------------------------------------------------------------------+
// Basic Writing
//--------------------------------------------------------------------+
void IKOverlay::initStdBasicWriting(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_BASIC_WRITING];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col;

  // for most keys, height = 3, width = 2
  int const height = 3;
  int const width = 2;

  //------------- First Row -------------//
  // mouse only

  //------------- Second Row -------------//
  row = 3;
  col = 0;

  uint8_t const first_row[][2] = {{0, HID_KEY_ESCAPE},
                                  {0, HID_KEY_TAB},
                                  {KEYBOARD_MODIFIER_LEFTALT, 0},
                                  {KEYBOARD_MODIFIER_LEFTGUI, 0},
                                  {KEYBOARD_MODIFIER_LEFTCTRL, 0}};

  overlay.setMembraneKeyboardRow(row, col, height, width, first_row,
                                 sizeof(first_row) / sizeof(first_row[0]));

  // TODO more mouse

  //------------- Third Row -------------//
  row = 6;
  col = 0;

  uint8_t const second_row[][2] = {{0, HID_KEY_PERIOD},
                                   {0, HID_KEY_COMMA},
                                   {0, HID_KEY_APOSTROPHE},
                                   {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_1},
                                   {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SLASH},
                                   {0, HID_KEY_MINUS}};

  overlay.setMembraneKeyboardRow(row, col, height, width, second_row,
                                 sizeof(second_row) / sizeof(second_row[0]));

  // TODO more mouse

  //------------- Fourth Row -------------//
  row = 9;
  col = 0;

  report.keyboard.modifier = 0;
  for (int i = 0; i < 10; i++) {
    report.keyboard.keycode = HID_KEY_1 + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  report.keyboard.keycode = HID_KEY_BACKSPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Fifth Row -------------//
  row = 12;
  col = 3; // first key is empty

  overlay.initQwertyRow(row, col, height, width);

  //------------- Sixth Row -------------//
  row = 15;
  col = 0;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_CAPS_LOCK;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += width;

  overlay.initAsdfghRow(row, col, height, width);
  col += 9 * width;

  report.keyboard.keycode = HID_KEY_ENTER;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Seventh Row -------------//
  row = 18;
  col = 0;

  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);
  col += 2 * width;

  overlay.initZxcvbnRow(row, col, height, width);
  col += 7 * width;

  report.keyboard.modifier = KEYBOARD_MODIFIER_RIGHTSHIFT;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Eighth Row -------------//
  row = 21;
  col = 8;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(row, col, height, 3 * width, &report);

  col += 4 * width;

  uint8_t const eighth_row[][2] = {{0, HID_KEY_ARROW_LEFT},
                                   {0, HID_KEY_ARROW_RIGHT},
                                   {0, HID_KEY_ARROW_UP},
                                   {0, HID_KEY_ARROW_DOWN}};

  overlay.setMembraneKeyboardRow(row, col, height, width, eighth_row,
                                 sizeof(eighth_row) / sizeof(eighth_row[0]));
}

//--------------------------------------------------------------------+
// Qwerty Overlay
//--------------------------------------------------------------------+
void IKOverlay::initStdQwerty(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_QWERTY];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int col, row;
  int const height = 3;
  int const width = 2;

  //------------- First Row -------------//
  row = 0;
  col = 0;

  uint8_t const first_row[][2] = {
      {0, HID_KEY_ESCAPE},
      {0, HID_KEY_TAB},
      {0, HID_KEY_GRAVE},
      {0, HID_KEY_NUM_LOCK},
      {0, 0}, // TODO what is NUMPAD?
      {0, HID_KEY_INSERT},
      {0, HID_KEY_HOME},
      {0, HID_KEY_END},
      {0, 0}, // TODO smart typing
      {0, HID_KEY_PAGE_UP},
      {0, HID_KEY_PAGE_DOWN},
      {0, HID_KEY_DELETE},
  };

  overlay.setMembraneKeyboardRow(row, col, height, width, first_row,
                                 sizeof(first_row) / sizeof(first_row[0]));

  //------------- Second Row -------------//
  row = 3;
  col = 0;

  report.keyboard.modifier = 0;
  for (int i = 0; i < 12; i++) {
    report.keyboard.keycode = HID_KEY_F1 + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  // Row 3 to 8
  initStdQwertyRow3to8(overlay);
}

void IKOverlay::initStdQwertyRow3to8(IKOverlay &overlay) {
  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int col, row;
  int const height = 3;
  int const width = 2;

  //------------- Third Row -------------//
  row = 6;
  col = 0;

  report.keyboard.modifier = 0;
  for (int i = 0; i < 10; i++) {
    report.keyboard.keycode = HID_KEY_1 + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  report.keyboard.keycode = HID_KEY_MINUS;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += width;

  report.keyboard.keycode = HID_KEY_EQUAL;
  overlay.setMembraneReport(row, col, height, width, &report);

  //------------- Fourth Row -------------//
  row = 9;
  col = 0;

  overlay.initQwertyRow(row, col, height, width);
  col = 10 * width;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_BACKSPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Fifth Row -------------//
  row = 12;
  col = 0;

  overlay.initAsdfghRow(row, col, height, width);
  col = 9 * width;
  // mouse button

  //------------- Sixth Row -------------//
  row = 15;
  col = 0;

  overlay.initZxcvbnRow(row, col, height, width);
  col = 7 * width;

  uint8_t sixth_row[][2] = {{0, HID_KEY_SEMICOLON}, {0, HID_KEY_APOSTROPHE}};
  overlay.setMembraneKeyboardRow(row, col, height, width, sixth_row,
                                 sizeof(sixth_row) / sizeof(sixth_row[0]));

  // more mouse

  //------------- Seventh Row -------------//
  row = 18;
  col = 0;

  uint8_t const seventh_row[][2] = {{0, HID_KEY_CAPS_LOCK},
                                    {KEYBOARD_MODIFIER_LEFTSHIFT, 0},
                                    {KEYBOARD_MODIFIER_LEFTSHIFT, 0},
                                    {0, HID_KEY_SPACE},
                                    {0, HID_KEY_SPACE},
                                    {0, HID_KEY_SPACE},
                                    {0, HID_KEY_COMMA},
                                    {0, HID_KEY_PERIOD},
                                    {0, HID_KEY_SLASH}};
  overlay.setMembraneKeyboardRow(row, col, height, width, seventh_row,
                                 sizeof(seventh_row) / sizeof(seventh_row[0]));

  // mouse report

  //------------- Eighth Row -------------//
  row = 21;
  col = 0;

  uint8_t eighth_row[][2] = {
      {KEYBOARD_MODIFIER_LEFTCTRL, 0},
      {KEYBOARD_MODIFIER_LEFTALT, 0},
      {KEYBOARD_MODIFIER_LEFTGUI, 0},
      {0, HID_KEY_ARROW_LEFT},
      {0, HID_KEY_ARROW_RIGHT},
      {0, HID_KEY_ARROW_UP},
      {0, HID_KEY_ARROW_DOWN},
      {0, HID_KEY_ENTER},
      {0, HID_KEY_ENTER},
  };
  overlay.setMembraneKeyboardRow(row, col, height, width, eighth_row,
                                 sizeof(eighth_row) / sizeof(eighth_row[0]));
}
//--------------------------------------------------------------------+
// Alphabet Overlay
//--------------------------------------------------------------------+
void IKOverlay::initStdAlphabet(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_ALPHABET];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col;
  int const height = 4;
  int width = 3;

  //------------- First Row -------------//
  row = 0;
  col = 0;
  width = 4;

  uint8_t const first_row[][2] = {
      {0, HID_KEY_ESCAPE}, {0, HID_KEY_CAPS_LOCK}, {0, HID_KEY_BACKSPACE}};

  overlay.setMembraneKeyboardRow(row, col, height, width, first_row,
                                 sizeof(first_row) / sizeof(first_row[0]));

  // Arrow
  width = 3;
  report.keyboard.keycode = HID_KEY_ARROW_LEFT;
  overlay.setMembraneReport(1, 14, height, width, &report);

  report.keyboard.keycode = HID_KEY_ARROW_UP;
  overlay.setMembraneReport(0, 18, height, width, &report);

  report.keyboard.keycode = HID_KEY_ARROW_RIGHT;
  overlay.setMembraneReport(1, 21, height, width, &report);

  report.keyboard.keycode = HID_KEY_ARROW_DOWN;
  overlay.setMembraneReport(4, 18, height, width, &report);

  //----------------- Second Row -------------//
  row = 4;
  col = 1;
  width = 2;

  report.keyboard.keycode = HID_KEY_PERIOD;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += width;

  report.keyboard.keycode = HID_KEY_COMMA;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += width;

  width = 3; // although the label is the same, actual test is 3

  report.keyboard.keycode = HID_KEY_SLASH;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(row, col, height, width, &report);
  col += width;

  report.keyboard.keycode = HID_KEY_1;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(row, col, height, width, &report);

  //----------------- Third Row -------------//
  row = 8;
  col = 0;
  width = 3;

  for (int i = 0; i < 8; i++) {
    report.keyboard.keycode = HID_KEY_A + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  //----------------- Fourth Row -------------
  row = 12;
  col = 0;

  for (int i = 0; i < 8; i++) {
    report.keyboard.keycode = HID_KEY_I + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  //----------------- Fifth Row -------------//
  row = 16;
  col = 0;

  for (int i = 0; i < 6; i++) {
    report.keyboard.keycode = HID_KEY_Q + i;
    overlay.setMembraneReport(row, col, height, width, &report);
    col += width;
  }

  report.keyboard.keycode = HID_KEY_ENTER;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Sixth Row -------------//
  row = 20;
  col = 0;

  uint8_t const sixth_row[][2] = {{KEYBOARD_MODIFIER_LEFTSHIFT, 0},
                                  {0, HID_KEY_W},
                                  {0, HID_KEY_X},
                                  {0, HID_KEY_Y},
                                  {0, HID_KEY_Z},
                                  {KEYBOARD_MODIFIER_RIGHTSHIFT, 0}};

  overlay.setMembraneKeyboardRow(row, col, height, width, sixth_row,
                                 sizeof(sixth_row) / sizeof(sixth_row[0]));
  col += 6 * width;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);
}

void IKOverlay::setMembraneKeyboardRow(int row, int col, int height, int width,
                                       uint8_t const kbd_item[][2],
                                       uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    ik_report_t report;
    report.type = IK_REPORT_TYPE_KEYBOARD;
    report.keyboard.modifier = kbd_item[i][0];
    report.keyboard.keycode = kbd_item[i][1];

    setMembraneReport(row, col, height, width, &report);

    col += width;
  }
}

void IKOverlay::initQwertyRow(int row, int col, int height, int width) {
  uint8_t kbd_item[][2] = {{0, HID_KEY_Q}, {0, HID_KEY_W}, {0, HID_KEY_E},
                           {0, HID_KEY_R}, {0, HID_KEY_T}, {0, HID_KEY_Y},
                           {0, HID_KEY_U}, {0, HID_KEY_I}, {0, HID_KEY_O},
                           {0, HID_KEY_P}};

  setMembraneKeyboardRow(row, col, height, width, kbd_item,
                         sizeof(kbd_item) / sizeof(kbd_item[0]));
}

void IKOverlay::initAsdfghRow(int row, int col, int height, int width) {
  uint8_t kbd_item[][2] = {{0, HID_KEY_A}, {0, HID_KEY_S}, {0, HID_KEY_D},
                           {0, HID_KEY_F}, {0, HID_KEY_G}, {0, HID_KEY_H},
                           {0, HID_KEY_J}, {0, HID_KEY_K}, {0, HID_KEY_L}};

  setMembraneKeyboardRow(row, col, height, width, kbd_item,
                         sizeof(kbd_item) / sizeof(kbd_item[0]));
}

void IKOverlay::initZxcvbnRow(int row, int col, int height, int width) {
  uint8_t kb_item[][2] = {{0, HID_KEY_Z}, {0, HID_KEY_X}, {0, HID_KEY_C},
                          {0, HID_KEY_V}, {0, HID_KEY_B}, {0, HID_KEY_N},
                          {0, HID_KEY_M}};

  setMembraneKeyboardRow(row, col, height, width, kb_item,
                         sizeof(kb_item) / sizeof(kb_item[0]));
}
