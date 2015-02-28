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
#ifndef DISPLAY_H
#define DISPLAY_H

#define RS_f = 1
#define RW_f = 2
#define CS1_f = 4
#define CS2_f = 8

#include "mbed.h"
#include "font.h"

class Display  {
    public:
        // Constructor:
        Display (PinName _RS, PinName _RW, PinName _E, PinName _CS1, PinName _CS2, PinName DB0, PinName DB1, PinName DB2, PinName DB3, PinName DB4, PinName DB5, PinName DB6, PinName DB7);
        // Send write command to Display
        // Page 0-7, y 0-127
        void write (int page, int y, unsigned int data);
        // 
        void SetAddress(int);
        int SendCommand(unsigned int, int);
        // Row 0-7, col 0-122
        void writec(int row, int Y, int c);
        
    private:
        BusInOut DB;
        DigitalOut RS;
        DigitalOut RW;
        DigitalOut E;
        DigitalOut CS1;
        DigitalOut CS2;
        int CurCol;
};

class DisplayTTY : public Stream {
    public:
        DisplayTTY (Display *d, int _row, int _startY, int _numOfChars=10, int _numOfRows=1, int _charOffset=6, int _flags=0);
        void cls();
    protected:
        virtual int _putc (int c);
        virtual int _getc();
        virtual void newline();
        Display *kalle;
        int row;
        int startY;
        int numOfChars;
        int numOfRows;
        int charOffset;
        int flags;
        int CursPosY;
        int CursPosW;
};

#endif