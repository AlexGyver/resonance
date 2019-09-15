/*
	GyverOLED - лёгкая и быстрая библиотека для олед дисплея с возможностью выбора буфера
	20 маленьких символов по горизонтали, по вертикали 4 (для х32) и 8 (для х64)
	
	Если НЕТ буфера
	- авто отправка всегда вкл (update() не нужен)
	- точка - рисуется в байте и перезаписывает всю остальную инфу
	- текст - перезаписывает байт

	Если ХАРД буфер
	- авто отправка всегда вкл (update() не нужен)
	- точка - берётся байт из буфера и добавляется точка
	- берётся из буфера и добавляет инфу

	Если СОФТ буфер
	- авто отправка вкл (update() не нужен)
	- авто отправка выкл (update() нужен)
	- точка - берётся байт из буфера и добавляется точка
	- текст - берёт из буфера и дополняет
*/

#ifndef GyverOLED_h
#define GyverOLED_h

#if defined(USE_MICRO_WIRE)
#include <microWire.h>	// лёгкая библиотека Wire (для atmega328)
#else
#include <Wire.h>		// обычная Wire
#endif

#include <Arduino.h>
#include "charMap.h"

#define OLED128x32 0
#define OLED128x64 1

#define OLED_HEIGHT_32          0x02  // oled 128x32
#define OLED_HEIGHT_64          0x12  // oled 128x64
#define OLED_64          		0x3F
#define OLED_32          		0x1F

#define OLED_COMMAND_MODE       0x00
#define OLED_ONE_COMMAND_MODE   0x80
#define OLED_DATA_MODE          0x40
#define OLED_ONE_DATA_MODE      0xC0

const uint8_t _oled_init[] PROGMEM = {
	0xAE, // oled off
	0xD5, // CLOCK_DIV_RATIO
	0x80,
	0x8D, // Charge pump
	0x14,
	0x20, // Memory mode
	0x01, // Vertical OLED_ADDRESSing
	0xA1, // Flip horizontally
	0xC8, // Flip vertically
	0x81, // Set contrast
	0xCF, // brighter
	0xDB, // Set vcom detect
	0x40, // brighter
	0xAF,  // Display on
};
#ifdef OLED_HARD_BUFFER_64
#define HARD_BUFFER
#endif

#ifdef OLED_SOFT_BUFFER_32
#define SOFT_BUFFER
byte _oled_buffer[128*32/8];
#endif

#ifdef OLED_SOFT_BUFFER_64
#define SOFT_BUFFER
byte _oled_buffer[128*64/8];
#endif

class GyverOLED {
public:
	GyverOLED(byte address = 0x3C);
	void init(uint8_t oledType, int freq = 400);	
	void clear();
	void setContrast(byte value);
	void sendCommand(byte command);
	void flipH();
	void flipV();
	
	void home();
	void setCursor(int column, int line);	
	void scale1X();
	void scale2X();	
	void setInverse(boolean inv);
	void printChar(char c);
	
	void println();
	
	void print(char c);	
	void print(char data[]);
	void print(String data);
	void print(uint32_t data);
	void print(double data);
	void print(double data, byte decimals);
	
	void println(char c);
	void println(char data[]);
	void println(String data);
	void println(uint32_t data);
	void println(double data);
	void println(double data, byte decimals);
	
	boolean isEnd();
	
	void dot(int8_t x, int8_t y);
	void line(int8_t x0, int8_t y0, int8_t x1, int8_t y1);
	
	void update();		
	
private:
	void _printBytes(uint32_t data);
	void _printBytes(uint32_t data, byte decimals);
	byte OLED_ADDRESS = 0x3C;
	uint8_t _getFont(uint8_t font, uint8_t row);
	int _stretch(int x);
	void _singleCMD(uint8_t x);
	boolean _type, _invState = 0;
	byte _scale = 1;
	byte _x = 0, _y = 0;
	byte _lastChar;
	byte _numRows = 8;
};

// ====================== РЕАЛИЗАЦИЯ =======================
GyverOLED::GyverOLED(byte address){
	OLED_ADDRESS = address;
}

// ======================= INIT =======================
void GyverOLED::init(uint8_t oledType, int freq) {
	Wire.begin();
	Wire.setClock(freq);
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	for (uint8_t i = 0; i < 14; i++) Wire.write(pgm_read_byte(&_oled_init[i]));
	
	Wire.write(0xDA);
	if (oledType) Wire.write(OLED_HEIGHT_64);
	else Wire.write(OLED_HEIGHT_32);

	Wire.write(0xA8);
	if (oledType) Wire.write(OLED_64);
	else Wire.write(OLED_32);
	
	Wire.endTransmission();
	
	_type = oledType;
	_numRows = (_type) ? 8 : 4;
	GyverOLED::setCursor(0, 0);
}


// ======================= CLEAR =======================
void GyverOLED::clear() {
#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x21);
	Wire.write(0);
	Wire.write(127);
	
	Wire.write(0x22);
	Wire.write(0);
	Wire.write(_numRows-1);
	
	Wire.endTransmission();
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
#endif
	int maxPacks = (_type) ? 1024 : 512;
	for (int i = 0; i < maxPacks; i++) {
#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
		Wire.write(0);
#endif
#ifdef SOFT_BUFFER
		_oled_buffer[i] = 0;
#endif
	}
#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
	Wire.endTransmission();
#endif
	//GyverOLED::setCursor(0, 0);
}

// ======================= CONTRAST =======================
void GyverOLED::setContrast(byte value) {
	sendCommand(0x81);
	sendCommand(value);
}

// ======================= CURSOR =======================
void GyverOLED::setCursor(int column, int line) {
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	// Set column address range
	Wire.write(0x21); Wire.write(column * 6); Wire.write(column * 6 + _scale * 6 - 1);
	// Set page address range
	Wire.write(0x22); Wire.write(line); Wire.write(line + _scale - 1);
	Wire.endTransmission();
	_x = column;
	_y = line;
}

void GyverOLED::home() {
	GyverOLED::setCursor(0, 0);
}

// ======================= PRINT =======================
uint8_t GyverOLED::_getFont(uint8_t font, uint8_t row) {
	font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
	if (font <= 90) return pgm_read_byte(&(charMap[font][row]));		// для английских букв и символов
	else if (font >= 112 && font <= 159) {							// и пизд*ц для русских
		return pgm_read_byte(&(charMap[font - 17][row]));
	} else if (font >= 96 && font <= 111) {
		return pgm_read_byte(&(charMap[font + 47][row]));
	}
}

void GyverOLED::printChar(char c) {	
#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
#endif
	for (uint8_t col = 0; col < 6; col++) {
		int bits;
		bits = (col != 5) ? GyverOLED::_getFont((char)c, col) : 0;
		if (_invState) bits = ~bits;
		
#ifdef SOFT_BUFFER		
		int index = _y + (_x*6 + col * _scale) * _numRows;
		if (_scale == 1) {		
		_oled_buffer[index] |= bits;
		bits = _oled_buffer[index];
		} else {
			int newBits = _stretch(bits);
			_oled_buffer[index] |= newBits;
			_oled_buffer[index+1] |= newBits >> 8;
			_oled_buffer[index+_numRows] |= newBits;
			_oled_buffer[index+_numRows+1] |= newBits >> 8;
		}
#endif

#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
		// тут нужно для ХАРД буфера прочитать байт и дописать к нему bits
		
		if (_scale == 1) Wire.write(bits);
		else {
			bits = _stretch(bits);
			Wire.write(bits);
			Wire.write(bits >> 8);
			Wire.write(bits);
			Wire.write(bits >> 8);
		}
#endif
	}
#if defined(OLED_AUTO_SEND) || !defined(SOFT_BUFFER) || defined(HARD_BUFFER)
	Wire.endTransmission();
#endif
}

void GyverOLED::_printBytes(uint32_t data) {
	int8_t bytes[10];
	byte amount;
	for (byte i = 0; i < 10; i++) {
		bytes[i] = data % 10;
		data /= 10;
		if (data == 0) {
			amount = i;
			break;
		}
	}	
	for (int8_t i = amount; i >= 0; i--) {
		GyverOLED::print((char)(bytes[i] + '0'));
	}
}


void GyverOLED::print(uint32_t data) {GyverOLED::_printBytes((long) data);}
void GyverOLED::println(uint32_t data) {GyverOLED::_printBytes((long) data); GyverOLED::println();}

void GyverOLED::print(double data, byte decimals) {
	if (data < 0) {
		print('-');
		data = -data;
	}
	uint32_t integer = data;
	_printBytes((uint32_t) integer);
	print('.');
	data -= integer;
	for (byte i = 0; i < decimals; i++) {	
		data *= 10.0;
		_printBytes((uint32_t) data);
		data -= (byte)data;
	}
}

void GyverOLED::print(double data) {
	GyverOLED::print(data, 2);
}

void GyverOLED::println(double data, byte decimals) {
	print(data, decimals);
	println();
}

void GyverOLED::println(double data) {
	print(data, 2);
	println();
}

void GyverOLED::println() {
	_y += _scale;
	_x = 0;
	if (_y > _numRows-1) _y = _numRows-1;
	setCursor(_x, _y);
}

void GyverOLED::print(char c) {	
	if ((byte)c == 10) GyverOLED::println();
	else {
		if ((byte)c <= 127) {printChar(c); _x += _scale;}		// если лат. буквы и символы
		else {
			if (_lastChar == 209 && (byte)c == 145) printChar((char)181);			// е вместо ё
			else if (_lastChar == 208 && (byte)c == 129) printChar((char)149);		// Е вместо Ё
			else if (_lastChar == 226 && (byte)c == 128) ;							// тире вместо длинного тире
			else if (_lastChar == 128 && (byte)c == 147) {printChar((char)45);_x += _scale;}		// тире вместо длинного тире			
			else if ((byte)c <= 191) {printChar(c); _x += _scale;}
		}
		_lastChar = c;		
		
		if (_x >= 20) {
			_x = 0;
			_y += _scale;			
		}
		setCursor(_x, _y);
	}	
}

void GyverOLED::println(char c) {
	printChar(c);
	_y += _scale;
	if (_y > _numRows-1) _y = _numRows-1;
	setCursor(_x, _y);
}

void GyverOLED::print(char data[]) {
	byte i = 0;
	byte lastPref;
	while (data[i] != '\0') {
		/*if ((byte)data[i] <= 127) print(data[i]);		// если лат. буквы и символы
		else {
			if (lastPref == 209 && (byte)data[i] == 145) print((char)181);			// е вместо ё
			else if (lastPref == 208 && (byte)data[i] == 129) print((char)149);		// Е вместо Ё
			else if ((byte)data[i] <= 191) print(data[i]);
		}
		lastPref = data[i];*/
		print(data[i]);
		i++;
	}
}

void GyverOLED::println(char data[]) {
	GyverOLED::print(data);
	GyverOLED::println();
}

void GyverOLED::print(String data) {
	byte stringSize = data.length();
	for (byte i = 0; i < stringSize; i++) {
		print((char)data[i]);
	}
}

void GyverOLED::println(String data) {
	GyverOLED::print(data);
	GyverOLED::println();
}

// ======================= FONT =======================
void GyverOLED::scale1X() {
	_scale = 1;
	setCursor(_x, _y);
}
void GyverOLED::scale2X() {
	_scale = 2;
	setCursor(_x, _y);
}

void GyverOLED::setInverse(boolean inv) {
	_invState = inv;
}

void GyverOLED::flipH() {
	GyverOLED::sendCommand(0xA0);
}

void GyverOLED::flipV() {
	GyverOLED::sendCommand(0xC0);
}

// ======================= GRAPHICS =======================
/*void GyverOLED::dot(int8_t x, int8_t y, boolean mode) {
	// точка через вертикальные линии
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x21); Wire.write(x); Wire.write(x);  // Column range
	Wire.write(0x22); Wire.write(0); Wire.write(_numRows-1);  // Page range
	Wire.endTransmission();
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	for (int i = 0; i < 8; i++) {
		if (y > 7) Wire.write(- mode);
		else if (y < 0) Wire.write(0);
		else Wire.write((1 << y) - mode);
		y = y - 8;
	}
	Wire.endTransmission();
}*/

void GyverOLED::dot(int8_t x, int8_t y) {
	if (x > 127 || y > ((_type) ? 63 : 31) ||
		x < 0 || y < 0) return;
#if defined(SOFT_BUFFER)
	// буфер точечный	
	int index = y / _numRows + x * _numRows;
	bitWrite(_oled_buffer[index], y % _numRows, 1);

#ifdef OLED_AUTO_SEND
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x21); Wire.write(x); Wire.write(127);
	Wire.write(0x22); Wire.write(y / _numRows); Wire.write(_numRows-1);
	Wire.endTransmission();

	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	Wire.write(_oled_buffer[index]);
	Wire.endTransmission();
#endif
	
#elif defined(HARD_BUFFER)
	// свой буфер - прочитали, дописали, отправили
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x00 + ((x + 2) & 0x0F));
	Wire.write(0x10 + ((x + 2) >> 4));
	Wire.write(0xB0 + (y >> 3));
	Wire.endTransmission();
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	Wire.write(thisByte);
	Wire.endTransmission();	

#else	
	// без буфера - отправили и всё
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x21); Wire.write(x); Wire.write(127);
	Wire.write(0x22); Wire.write(y / _numRows); Wire.write(_numRows-1);
	Wire.endTransmission();

	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	Wire.write(1 << (y % _numRows));
	Wire.endTransmission();
	
#endif
}

void GyverOLED::update() {
#if defined(SOFT_BUFFER) && !defined(OLED_AUTO_SEND)
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(0x21);
	Wire.write(0);
	Wire.write(127);
	
	Wire.write(0x22);
	Wire.write(0);
	Wire.write( (_type) ? 7 : 3 );
	
	Wire.endTransmission();
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	int maxPacks = (_type) ? 1024 : 512;
	for (int i = 0; i < maxPacks; i++)
		Wire.write(_oled_buffer[i]);
	Wire.endTransmission();
#endif
}

void GyverOLED::line(int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
	if (x0 == x1) {
		for (byte i = y0; i < y1; i++) dot(x0, i);
	} else if (y0 == y1) {
		for (byte i = x0; i < x1; i++) dot(i, y0);
	} else {
		int sx, sy, e2, err;
		int dx = abs(x1 - x0);
		int dy = abs(y1 - y0);
		if (x0 < x1) sx = 1; else sx = -1;
		if (y0 < y1) sy = 1; else sy = -1;
		err = dx - dy;
		for (;;) {
			dot(x0, y0);
			if (x0==x1 && y0==y1) return;
			e2 = err<<1;
			if (e2 > -dy) { err = err - dy; x0 = x0 + sx; }
			if (e2 < dx) { err = err + dx; y0 = y0 + sy; }
		}
	}	
}

// ======================= SERVICE =======================
boolean GyverOLED::isEnd() {
	if (_y > _numRows-1) return true;
	else return false;
}

void GyverOLED::sendCommand(byte command) {
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_COMMAND_MODE);
	Wire.write(command);
	Wire.endTransmission();
}

int GyverOLED::_stretch (int x) {
	x = (x & 0xF0) << 4 | (x & 0x0F);
	x = (x << 2 | x) & 0x3333;
	x = (x << 1 | x) & 0x5555;
	return x | x << 1;
}

void GyverOLED::_singleCMD(uint8_t x) {
	Wire.write(OLED_ONE_COMMAND_MODE);
	Wire.write(x);
}


#endif