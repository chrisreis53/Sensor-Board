/*
 Copyright (c) 2010 Anders Hörnfeldt

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/
 
#include "Display.h"
#include "font.h"

Display::Display (PinName _RS, PinName _RW, PinName _E, PinName _CS1, PinName _CS2, PinName DB0, PinName DB1, PinName DB2, PinName DB3, PinName DB4, PinName DB5, PinName DB6, PinName DB7)
    : DB(DB0,DB1,DB2,DB3,DB4,DB5,DB6,DB7), RS(_RS), RW(_RW), E(_E), CS1(_CS1), CS2(_CS2) {
    DB.mode(PullNone);
    E = 0;
    SendCommand(0x3F, 4|8);
    for (int c=0;c<128;c++) {
        for (int r=0;r<8;r++)
        {
            write(r,c,0);
        }
    }
}
DisplayTTY::DisplayTTY (Display *d, int _row, int _startY, int _numOfChars, int _numOfRows, int _charOffset, int _flags)
{
    kalle = d;
    row = _row;
    startY = _startY;
    numOfChars = _numOfChars;
    numOfRows = _numOfRows;
    charOffset = _charOffset;
    flags = _flags;
    CursPosY = 0;
    CursPosW = row;
    cls();
}
int Display::SendCommand(unsigned int Command, int f) {
    int value = 1;
    E = 0;
    RS = 0;
    RW = 0;
    CS1 = 0;
    CS2 = 0;
    if (f&1)
        RS = 1;
    if (f&2)
        RW = 1;
    if (f&4)
        CS1 = 1;
    if (f&8)
        CS2 = 1;
    wait(0.0000003); // 300ns
    E = 1;
    if (f&2)
    {
        DB.input();
        wait(0.0000003);
        value = DB;
    }
    else
    {
        DB.output();
        DB = Command;
    }
    wait(0.0000001);
    E = 0;
    return value;
}

void Display::write (int page, int y, unsigned int data) {
    int f = 0;
    if (y<64)
        f = 4;
    else
        f = 8;
    CurCol = y;
    SendCommand(0xB8+(page&0x07), f);
    SendCommand(0x40+(y&0x3F),f);
    SendCommand(data, f+1);
}

void Display::writec (int row, int Y, int c) {
    if (c>31 && c<127)
    {
        write(row,Y+0,font5x8[(c-32)*5+0]);
        write(row,Y+1,font5x8[(c-32)*5+1]);
        write(row,Y+2,font5x8[(c-32)*5+2]);
        write(row,Y+3,font5x8[(c-32)*5+3]);
        write(row,Y+4,font5x8[(c-32)*5+4]);
    }
}
int DisplayTTY::_putc (int c)
{
    if (c == '\n')
        newline();
    else
    {
        kalle->writec(CursPosW, startY+CursPosY*charOffset, c);

        if (++CursPosY>=numOfChars)
            newline();
    }

    return 0;
}
int DisplayTTY::_getc() {
    return 0;
}
void DisplayTTY::newline() {
    CursPosY=0;
    if ((++CursPosW-row)>=numOfRows)
    {
        CursPosW = row;
    }
}
void DisplayTTY::cls() {
    CursPosY=0;
    CursPosW = row;
    for (int c=0;c<numOfChars*numOfRows;c++)
    {
        _putc(' ');
    }
}