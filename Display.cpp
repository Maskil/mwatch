#include "Display.h"

Display::Display(int16_t cs, int16_t dc, int16_t rst, int16_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, HIGH, 10000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  selectSPI(SPI, SPISettings(20000000, MSBFIRST, SPI_MODE0)); // 4 ~ 20
}

void Display::clearScreen(uint8_t value)
{
  writeScreenBuffer(value);
  refresh(true);
  _writeScreenBuffer(0x24, value);
  writeScreenBufferAgain(value);
}

void Display::writeScreenBuffer(uint8_t value)
{
  if (!_using_partial_mode) _Init_Part();
  if (_initial_write) _writeScreenBuffer(0x26, value); // set previous
  _writeScreenBuffer(0x24, value); // set current
  _initial_write = false; // initial full screen buffer clean done
}

void Display::writeScreenBufferAgain(uint8_t value)
{
  if (!_using_partial_mode) _Init_Part();
  _writeScreenBuffer(0x24, value); // set current
}

void Display::_writeScreenBuffer(uint8_t command, uint8_t value)
{
  _startTransfer();
  _transferCommand(command);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
}

void Display::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void Display::writeImageForFullRefresh(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x26, bitmap, x, y, w, h, invert, mirror_y, pgm);
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void Display::writeImageAgain(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void Display::_writeImage(uint8_t command, const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
  yield(); // avoid wdt
  int16_t wb = (w + 7) / 8; // width bytes, bitmaps are padded
  x -= x % 8; // byte boundary
  w = wb * 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_using_partial_mode) _Init_Part();
  _setPartialRamArea(x1, y1, w1, h1);
  _startTransfer();
  _transferCommand(command);
  for (int16_t i = 0; i < h1; i++)
    for (int16_t j = 0; j < w1 / 8; j++){
      uint8_t data;
      // use wb, h of bitmap for index!
      int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
      data = pgm ? pgm_read_byte(&bitmap[idx]) : bitmap[idx];
      if (invert) data = ~data;
      _transfer(data);
    }
  _endTransfer();
  yield(); // avoid wdt
}

void Display::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImagePart(0x24, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void Display::_writeImagePart(uint8_t command, const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                              int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
#if defined(ESP8266) || defined(ESP32)
  yield(); // avoid wdt
#endif
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;
  int16_t wb_bitmap = (w_bitmap + 7) / 8; // width bytes, bitmaps are padded
  x_part -= x_part % 8; // byte boundary
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
  x -= x % 8; // byte boundary
  w = 8 * ((w + 7) / 8); // byte boundary, bitmaps are padded
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_using_partial_mode) _Init_Part();
  _setPartialRamArea(x1, y1, w1, h1);
  _startTransfer();
  _transferCommand(command);
  for (int16_t i = 0; i < h1; i++)
    for (int16_t j = 0; j < w1 / 8; j++)
      {
        uint8_t data;
        // use wb_bitmap, h_bitmap of bitmap for index!
        int16_t idx = mirror_y ? x_part / 8 + j + dx / 8 + ((h_bitmap - 1 - (y_part + i + dy))) * wb_bitmap : x_part / 8 + j + dx / 8 + (y_part + i + dy) * wb_bitmap;
        if (pgm)
          {
            data = pgm_read_byte(&bitmap[idx]);
          }
        else
            data = bitmap[idx];
        if (invert) data = ~data;
        _transfer(data);
    }
  _endTransfer();
  yield(); // avoid wdt
}

void Display::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else {
    if (_using_partial_mode) _Init_Full();
    _Update_Full();
    _initial_refresh = false; // initial full update done
  }
}

void Display::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (_initial_refresh) return refresh(false); // initial update needs be full update
  // intersection with screen
  int16_t w1 = x < 0 ? w + x : w; // reduce
  int16_t h1 = y < 0 ? h + y : h; // reduce
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  w1 = x1 + w1 < int16_t(WIDTH) ? w1 : int16_t(WIDTH) - x1; // limit
  h1 = y1 + h1 < int16_t(HEIGHT) ? h1 : int16_t(HEIGHT) - y1; // limit
  if ((w1 <= 0) || (h1 <= 0)) return; 
  // make x1, w1 multiple of 8
  w1 += x1 % 8;
  if (w1 % 8 > 0) w1 += 8 - w1 % 8;
  x1 -= x1 % 8;
  if (!_using_partial_mode) _Init_Part();
  _setPartialRamArea(x1, y1, w1, h1);
  _Update_Part();
}

void Display::powerOff()
{
  _PowerOff();
}

void Display::hibernate()
{
  //_PowerOff(); // Not needed before entering deep sleep
  if (_rst >= 0)
  {
    _writeCommand(0x10); // deep sleep mode
    _writeData(0x1);     // enter deep sleep
    _hibernating = true;
  }
}

void Display::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  _startTransfer();
  _transferCommand(0x11); // set ram entry mode
  _transfer(0x03);    // x increase, y increase : normal mode
  _transferCommand(0x44);
  _transfer(x / 8);
  _transfer((x + w - 1) / 8);
  _transferCommand(0x45);
  _transfer(y % 256);
  _transfer(y / 256);
  _transfer((y + h - 1) % 256);
  _transfer((y + h - 1) / 256);
  _transferCommand(0x4e);
  _transfer(x / 8);
  _transferCommand(0x4f);
  _transfer(y % 256);
  _transfer(y / 256);
  _endTransfer();
}

void Display::_PowerOn()
{
  if (!_power_is_on)
  {
    _startTransfer();
    _transferCommand(0x22);
    _transfer(0xf8);
    _transferCommand(0x20);
    _endTransfer();
    _waitWhileBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void Display::_PowerOff()
{
  if (_power_is_on)
  {
    _startTransfer();
    _transferCommand(0x22);
    _transfer(0x83);
    _transferCommand(0x20);
    _endTransfer();
    _waitWhileBusy("_PowerOff", power_off_time);
  }
  _power_is_on = false;
  _using_partial_mode = false;
}

void Display::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x12); // soft reset
  _waitWhileBusy("_SoftReset", 10); // 10ms max according to specs

  _startTransfer();
  _transferCommand(0x01); // Driver output control
  _transfer(0xC7);
  _transfer(0x00);
  _transfer(0x00);
  _transferCommand(0x3C); // BorderWavefrom
  _transfer(darkBorder ? 0x02 : 0x05);
  _transferCommand(0x18); // Read built-in temperature sensor
  _transfer(0x80);
  _endTransfer();

  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
}

void Display::_Init_Full()
{
  _InitDisplay();
  _PowerOn();
  _using_partial_mode = false;
}

void Display::_Init_Part()
{
  _InitDisplay();
  _PowerOn();
  _using_partial_mode = true;
}

void Display::_Update_Full()
{
  _startTransfer();
  _transferCommand(0x22);
  _transfer(0xf4);
  _transferCommand(0x20);
  _endTransfer();
  _waitWhileBusy("_Update_Full", full_refresh_time);
}

void Display::_Update_Part()
{
  _startTransfer();
  _transferCommand(0x22);
  //_transfer(0xcc); // skip temperature load (-5ms)
  _transfer(0xfc);
  _transferCommand(0x20);
  _endTransfer();
  _waitWhileBusy("_Update_Part", partial_refresh_time);
}

void Display::_transferCommand(uint8_t value)
{
  if (_dc >= 0) digitalWrite(_dc, LOW);
  SPI.transfer(value);
  if (_dc >= 0) digitalWrite(_dc, HIGH);
}
