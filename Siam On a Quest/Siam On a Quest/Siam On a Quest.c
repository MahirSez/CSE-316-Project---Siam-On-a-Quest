#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000
#define INF 1e9
#include <util/delay.h>


int pattern[16][16] ;

const int totGreenBar = 180;
int nowCol;
int delay;
int currentGreenBar;

void enableRed() {
	
	PORTC &= 0b11111101;
	PORTC |= 0b00000001;	
}

void disableRed()
{
	PORTC |= 0x02;
	PORTC |= 0x01;	
}
void enableGreen() {
	
	PORTC &= 0b11111110;
	PORTC |= 0b00000010;
}
void disableGreen()
{
	PORTC |= 0x03;
}


void printScreen() {
	
	for(int r = 0; r < 16; r++){
		PORTA = r;
		for(int c = 0; c < 16; c++){
			if(pattern[r][c] == 1) {
				disableRed();
				PORTD = (c<<4);
				enableGreen();
				
			}
			
			else if( pattern[r][c] == 2 ){
				disableGreen();
				PORTD = (c);
				enableRed();
			}
			
		}
		disableGreen();
	}
}

void printErrorBar() {
	
	for(int i = 4 ; i < 10 ; i++ ) {
		pattern[3][i] = 2;
	}
}

void printGreen() {	
	for(int i =0 ; i < 16 ; i++ ) {	
		PORTA = i;
		for(int j =0 ; j < 16 ; j++ ) {	
			PORTD = ( j <<4);
		}
	}
}

void printRed() {	
	for(int i =0 ; i < 16 ; i++ ) {
		
			
		PORTA = 0;
		
		for(int j =0 ; j < 16 ; j++ ) {
			
			PORTD = ( j );
		}
	}
}

struct greenBars {
	int len , ro , col;
} bars[180];


void setGreenBars() {
		
	for(int i =0 ; i < totGreenBar ; i++ ) {
		if(i %3 == 0 ) bars[i].ro = 14;
		else if(i %3 == 1 ) bars[i].ro = 9;
		else if(i %3 == 2 ) bars[i].ro = 6;
		
		if( i > 0  ) bars[i].col = bars[i-1].col + bars[i-1].len ;
		
		if(i %3 == 0 ) bars[i].len = 5;
		else if(i %3 == 1 ) bars[i].len = 3;
		else if(i %3 == 2 ) bars[i].len = 5;
		
	}
}


int insideFrame(int barID) {
	
	int frm = bars[barID].col;
	int to = frm + bars[barID].len;

	int leftFrame = nowCol;
	int rightFrame = nowCol + 15;

	if(frm >= leftFrame && to <= rightFrame) return 1;
	if(frm <= leftFrame && to >= leftFrame) return 1;
	if(frm <= rightFrame && to >= rightFrame) return 1;

	return 0;
}


void setOnPatternArray(int barID) {
	
	int barRo = bars[barID].ro;
	int barBeginCol = bars[barID].col;
	int barEndCol = barBeginCol + bars[barID].len - 1;
	
	int printFrm = nowCol;
	if( barBeginCol > printFrm)  printFrm = barBeginCol;
	
	int printTo = nowCol + 15;
	if(barEndCol < printTo  ) printTo = barEndCol;
	
	
	
	for(int i = printFrm ; i <= printTo ; i++ ) {
		pattern[barRo][i - nowCol] = 1;
	}
	
	
	
}

void UpdateGreenBars() {
	
	memset(pattern , 0 , sizeof pattern);
	int tmp = currentGreenBar;
	currentGreenBar  = INF;
	for(int i = tmp ; i < tmp + 5 ; i++  ) {
	
		if(insideFrame(i) )	 {
			
			if(i < currentGreenBar)  currentGreenBar = i;
			setOnPatternArray(i);
		}
	}
	
}


void portInit() {
	
		
	//PORTA = row(A0 to A3)
	DDRA = 0xFF;
	
	//PORTD = column ; red -> D0 to D7 ; green -> D8 to D15
	DDRD = 0xFF;
	
	//PORTC -> last 2 bit controls color C0 -> greeen C1 -> red
	DDRC = 0xFF;
	
	
	
	
	DDRB = 0b11111110;	
}

int move_condition_found() {
	delay++;
	delay %= 5;	
	return  (bit_is_set(PINB, 0) && delay == 1 );
}

int main()
{
	
	portInit();	
	
	setGreenBars();
	
	
	
	
			
	while(1) {
		
		UpdateGreenBars();
		printScreen();
		
		if(move_condition_found()) {
			nowCol++;
		}
		
	}

	return 0;
}


/*

//LED TEST CODE

int main()
{
	
	portInit();
	
	int cnt = 0;

	
	
	while(1) {
		 
		for(int r = 0; r < 16; r++){
			PORTA = r;
			for(int c = 0; c < 16; c++){
				if(cnt < 256) {
					disableGreen();
					PORTD = (c);
					enableRed();
				
				}
				else {
					disableRed();
					PORTD = (c<<4);
					enableGreen();
				}			
				cnt++;
				cnt %= 512;
				_delay_ms(20);
			}
		}
	}

	return 0;
}
*/