#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000
#include <util/delay.h>



#define INF 1e9
#define GAME_DELAY_VAL 10 // more delay_val , more delay
#define JUMP_DELAY_VAL 5
#define JUMP_HEIGHT 5 // more JUMP_LHEIGHT more height
#define SIAM_HEIGHT 3


int pattern[16][16] ;

const int totGreenBar = 180;
int nowCol;
int game_delay , jump_delay;
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
			else disableGreen();
			
		}
		disableGreen();
	}
	//disableGreen();
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

struct Player {	
	int baseRo , baseCol , jmpLeft , alive;
} siam;


void setGreenBars() {
		
	for(int i =0 ; i < totGreenBar ; i++ ) {
		if(i %3 == 0 ) bars[i].ro = 14;
		else if(i %3 == 1 ) bars[i].ro = 10;
		else if(i %3 == 2 ) bars[i].ro = 6;
		
		if( i > 0  ) bars[i].col = bars[i-1].col + bars[i-1].len ;
		
		if(i %3 == 0 ) bars[i].len = 5;
		else if(i %3 == 1 ) bars[i].len = 3;
		else if(i %3 == 2 ) bars[i].len = 5;
		
	}
}

void setSiam() {
	
	siam.alive = 1;
	siam.baseRo = bars[0].ro - 1;
	siam.baseCol = bars[0].col + 1;
	siam.jmpLeft = 0;
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


int right_move_condition_found() {
	
	if(siam.alive == 0 ) return 0;
	
	int ret = 1;
	for(int i =0 ; i < SIAM_HEIGHT ; i++ ) {
		if(pattern[siam.baseRo-i][siam.baseCol + 1] == 1 ) ret = 0;
	}
	
	if( ret == 0 )  return 0;
	
	return  (bit_is_set(PINB, 0) && game_delay == 1 );
}


void jumpCheckSiam() {
	
	
	if(jump_delay != 1) return;
	
	if(siam.jmpLeft > 0) {
		siam.baseRo--;
		siam.jmpLeft--;
		
		for(int i =0 ; i < SIAM_HEIGHT ; i++ ) {
			if(pattern[siam.baseRo-i][siam.baseCol] == 1 ) {
				siam.jmpLeft = 0;
				siam.baseRo++;
			}				
		}		
	}
	else {
		if(pattern[siam.baseRo+1][siam.baseCol] == 0) siam.baseRo++;
	}	
	if(siam.baseRo > 15) siam.baseRo = 15; //	DELETE THIS
	if(siam.baseRo - SIAM_HEIGHT + 1 < 0 ) siam.baseRo = SIAM_HEIGHT -1;
	
}
void UpdateSiam() {
	
	if(siam.alive == 0) return;
	
	jumpCheckSiam();
	for(int i =0 ; i < SIAM_HEIGHT; i ++ ) {
		
		pattern[siam.baseRo-i][siam.baseCol] = 2;
	}
}

void portInit() {
	
		
	//PORTA = row(A0 to A3)
	DDRA = 0xFF;
	
	//PORTD = column ; red -> D0 to D7 ; green -> D8 to D15
	DDRD = 0xFF;
	
	//PORTC -> last 2 bit controls color C0 -> greeen C1 -> red
	DDRC = 0xFF;
	
	//PORTB -> b0 = input for left right navigation ; b1 =  input for jump
	DDRB = 0b11111100;	
}


int jump_condition_found() {
	
	if(siam.alive && pattern[siam.baseRo + 1][siam.baseCol] == 1 && bit_is_set(PINB, 1) && jump_delay == 1 ) return 1;
	if(bit_is_set(PINB, 1) && jump_delay == 1 &&  siam.baseRo == 15) return 1; //DELETE THIS
	return 0;
}

void jumpUp() {
	siam.jmpLeft = JUMP_HEIGHT;
}


void moveRight() { 
	nowCol++;
}


int main()
{
	
	portInit();	
	
	setGreenBars();
	setSiam();
	
	while(1) {
		game_delay++;
		jump_delay++;
		game_delay %= GAME_DELAY_VAL;	
		jump_delay %= JUMP_DELAY_VAL;
		
		UpdateGreenBars();
		UpdateSiam();//ALWAYS UPDATE SIAM AFTER GREEN BARS
		
		printScreen();
		
		if(right_move_condition_found()) {
			moveRight();
		}
		printScreen();
		if(jump_condition_found()) {
			jumpUp();
		}
		
		printScreen();
		
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