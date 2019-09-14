#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000
#include <util/delay.h>



#define INF 1e9
#define GAME_DELAY_VAL 4 // more delay_val , more delay
#define JUMP_DELAY_VAL 2
#define JUMP_HEIGHT 6 // more JUMP_LHEIGHT more height
#define SIAM_HEIGHT 3


int pattern[16][16] ;

const int totGreenBar = 50;
const int totBullet = 5;
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
			else if( pattern[r][c] >= 2 ){
				disableGreen();
				PORTD = (c);
				enableRed();
			}
			else disableGreen();
			
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

void buzzerSound() {
	
	
	PORTB |= 0x04;
	_delay_ms(100);
	PORTB  = 0;
}

struct greenBars {
	int len , ro , col;
} bars[50];

struct Player {	
	int baseRo , baseCol , jmpLeft ;
	int lifeLeft;
} siam , enemy[50];

struct Bullet {
	
	int ro , col , alive;
}bullet[5];


void setGreenBars() {
		
	for(int i =0 ; i < totGreenBar ; i++ ) {
		if(i %3 == 0 ) bars[i].ro = 14;
		else if(i %3 == 1 ) bars[i].ro = 10;
		else if(i %3 == 2 ) bars[i].ro = 8;
		
		if(i > 0 && i%3 == 0) bars[i].col = bars[i-1].col + bars[i-1].len  ;
		else if( i > 0  && i %3 == 1) bars[i].col = bars[i-1].col + bars[i-1].len +2 ;
		else if( i > 0 && i%3 == 2) bars[i].col = bars[i-1].col + bars[i-1].len + 3 ;
		
		
		if(i %3 == 0 ) bars[i].len = 10;
		else if(i %3 == 1 ) bars[i].len = 10;
		else if(i %3 == 2 ) bars[i].len = 10;
		
	}
}


void setEnemies() {
	
	for(int i =0 ; i < totGreenBar ; i++ ) {
		
		enemy[i].baseRo = bars[i].ro-1;
		enemy[i].baseCol = bars[i].col + bars[i].len - 5;
		enemy[i].lifeLeft = 1;
		enemy[i].jmpLeft = 0;
	}
}

void setSiam() {
	
	siam.baseRo = 2;
	siam.baseCol = 1;
	siam.jmpLeft = 0;
	siam.lifeLeft = 7;
}



int barInsideFrame(int barID) {
	
	int frm = bars[barID].col;
	int to = frm + bars[barID].len;

	int leftFrame = nowCol;
	int rightFrame = nowCol + 15;

	if(frm >= leftFrame && to <= rightFrame) return 1;
	if(frm <= leftFrame && to >= leftFrame) return 1;
	if(frm <= rightFrame && to >= rightFrame) return 1;

	return 0;
}


void setOnPatternArrayBar(int barID) {
	
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
	
		if(barInsideFrame(i) )	 {
			
			if(i < currentGreenBar)  currentGreenBar = i;
			setOnPatternArrayBar(i);
		}
	}
}

int enemyInsideFrame(int enemyID) {

	int leftFrame = nowCol;
	int rightFrame = nowCol + 15;

	if(enemy[enemyID].baseCol >= leftFrame && enemy[enemyID].baseCol <= rightFrame) return 1;
	if(enemy[enemyID].baseCol + 1 >= leftFrame && enemy[enemyID].baseCol + 1 <= rightFrame) return 1;

	return 0;
}


void setOnPatternArrayEnemy(int enemyID) {
	
	
	int enemyRo = enemy[enemyID].baseRo;
	
	int printFrm = nowCol;
	if( enemy[enemyID].baseCol > printFrm)  printFrm = enemy[enemyID].baseCol;
	
	int printTo = nowCol + 15;
	if(enemy[enemyID].baseCol + 1 < printTo  ) printTo = enemy[enemyID].baseCol + 1;
	
	int safe = 1;
	for(int i = printFrm ; i <= printTo ; i++ ) {
		if(pattern[enemyRo][i - nowCol] == 4) safe = 0;
		if(pattern[enemyRo-1][i - nowCol] == 4 ) safe = 0;
	}
	if(safe == 0 ) {
		enemy[enemyID].lifeLeft = 0;
		return ;
	}
	
	/*if(pattern[enemyRo][printFrm-nowCol-1] == 2 || pattern[enemyRo-1][printFrm-nowCol-1] == 2) {
		enemy[enemyID].lifeLeft = 0;
		killSiam();
		recoverSiam();
		return ;
	}
	*/		
	else if(pattern[enemyRo-2][printFrm-nowCol] == 2 || pattern[enemyRo-2][printTo-nowCol] == 2) {
		enemy[enemyID].lifeLeft = 0;
		return ;
	}		
	
	
	for(int i = printFrm ; i <= printTo ; i++ ) {
		pattern[enemyRo][i - nowCol] = 3;
		pattern[enemyRo-1][i - nowCol] = 3;
	}
	//pattern[enemyRo-1][printFrm-nowCol - 1] = 3;
}


void UpdateEnemies() {
	
	for(int i = currentGreenBar  ; i< currentGreenBar + 5 ; i++ ) {
		if(enemy[i].lifeLeft && enemyInsideFrame(i)) {
			setOnPatternArrayEnemy(i);
		}
	}
}

int bulletInsideFrame(int bulletID) {
	
	int leftFrame = nowCol;
	int rightFrame = nowCol + 15;
	
	if(bullet[bulletID].col >=leftFrame && bullet[bulletID].col <= rightFrame ) return 1;
	return 0;
}


void setOnPatternArrayBullet(int bulletID) {
	int row = bullet[bulletID].ro;
	int col = bullet[bulletID].col;
	
	pattern[row][col - nowCol] = 4;
}


void UpdateBullet() {
	
	for(int i =0 ; i < totBullet ; i++ ) {
		
		if(bullet[i].alive == 0 ) continue;
		
		//pattern[bullet[i].ro][bullet[i].col] = 0;
		bullet[i].col++;
		
		if(bulletInsideFrame(i)) {
			
			setOnPatternArrayBullet(i);
		}
		else {
			bullet[i].alive = 0;
		}			
	}
}


int right_move_condition_found() {
	
	if(siam.lifeLeft == 0 ) return 0;
	
	int ret = 1;
	for(int i =0 ; i < SIAM_HEIGHT ; i++ ) {
		if(pattern[siam.baseRo-i][siam.baseCol + 1] ) ret = 0;
	}
	
	if( ret == 0 )  return 0;
	
	return  (bit_is_set(PINB, 0) && game_delay == 0 );
}

int shoot_condition_found() {
	return (bit_is_set(PINB , 3));
}


void recoverSiam() {
	
	
	siam.baseRo = 2;
	//siam.baseCol = bars[currentGreenBar].col + 1;
	siam.jmpLeft = 0;
}

void killSiam() {
	
	siam.lifeLeft--;
	buzzerSound();
}

void jumpCheckSiam() {
	
	
	if(jump_delay != 0) return;
	
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
	if(siam.baseRo - SIAM_HEIGHT + 1 < 0 ) siam.baseRo = SIAM_HEIGHT -1;
	
	if(siam.baseRo >= 15) {
		killSiam();
		recoverSiam();
	}		
	
}
void UpdateSiam() {
	
	if(siam.lifeLeft == 0) return;
	
	jumpCheckSiam();

	
	for(int i =0 ; i < SIAM_HEIGHT; i ++ ) {
		
		pattern[siam.baseRo-i][siam.baseCol] = 2;
	}
	
	
	for(int i = 0 ; i < siam.lifeLeft ; i++) {
		pattern[0][15-i] = 2;
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
	DDRB = 0b11110100;	
}


int jump_condition_found() {
	
	if(siam.lifeLeft > 0 && pattern[siam.baseRo + 1][siam.baseCol]  && bit_is_set(PINB, 1) && jump_delay == 0 ) return 1;
	return 0;
}

void jumpUp() {
	siam.jmpLeft = JUMP_HEIGHT;
}


void moveRight() { 
	nowCol++;	
}



void shoot() {
	for(int i = 0 ; i < totBullet ; i++ ) {
		if(bullet[i].alive ) continue;
		bullet[i].alive = 1;
		bullet[i].ro = siam.baseRo-1 ;
		bullet[i].col = nowCol;
		return;
	}
}


struct Digit{
	int ro, col;
	int ara[5][3];
}digit[10];

void createDigit()
{
	//digit 0
	digit[0].ara[0][0] = 1, digit[0].ara[0][1] = 1, digit[0].ara[0][2] = 1;
	digit[0].ara[1][0] = 1, digit[0].ara[1][1] = 0, digit[0].ara[1][2] = 1;
	digit[0].ara[2][0] = 1, digit[0].ara[2][1] = 0, digit[0].ara[2][2] = 1;
	digit[0].ara[3][0] = 1, digit[0].ara[3][1] = 0, digit[0].ara[3][2] = 1;
	digit[0].ara[4][0] = 1, digit[0].ara[4][1] = 1, digit[0].ara[4][2] = 1;
	
	//digit 1
	digit[1].ara[0][0] = 0, digit[1].ara[0][1] = 1, digit[1].ara[0][2] = 0;
	digit[1].ara[1][0] = 1, digit[1].ara[1][1] = 1, digit[1].ara[1][2] = 0;
	digit[1].ara[2][0] = 0, digit[1].ara[2][1] = 1, digit[1].ara[2][2] = 0;
	digit[1].ara[3][0] = 0, digit[1].ara[3][1] = 1, digit[1].ara[3][2] = 0;
	digit[1].ara[4][0] = 1, digit[1].ara[4][1] = 1, digit[1].ara[4][2] = 1;

	//digit 2
	digit[2].ara[0][0] = 1, digit[2].ara[0][1] = 1, digit[2].ara[0][2] = 1;
	digit[2].ara[1][0] = 0, digit[2].ara[1][1] = 0, digit[2].ara[1][2] = 1;
	digit[2].ara[2][0] = 1, digit[2].ara[2][1] = 1, digit[2].ara[2][2] = 1;
	digit[2].ara[3][0] = 1, digit[2].ara[3][1] = 0, digit[2].ara[3][2] = 0;
	digit[2].ara[4][0] = 1, digit[2].ara[4][1] = 1, digit[2].ara[4][2] = 1;
		
	//digit 3
	digit[3].ara[0][0] = 1, digit[3].ara[0][1] = 1, digit[3].ara[0][2] = 1;
	digit[3].ara[1][0] = 0, digit[3].ara[1][1] = 0, digit[3].ara[1][2] = 1;
	digit[3].ara[2][0] = 0, digit[3].ara[2][1] = 1, digit[3].ara[2][2] = 1;
	digit[3].ara[3][0] = 0, digit[3].ara[3][1] = 0, digit[3].ara[3][2] = 1;
	digit[3].ara[4][0] = 1, digit[3].ara[4][1] = 1, digit[3].ara[4][2] = 1;

	//digit 4
	digit[4].ara[0][0] = 1, digit[4].ara[0][1] = 0, digit[4].ara[0][2] = 1;
	digit[4].ara[1][0] = 1, digit[4].ara[1][1] = 0, digit[4].ara[1][2] = 1;
	digit[4].ara[2][0] = 1, digit[4].ara[2][1] = 1, digit[4].ara[2][2] = 1;
	digit[4].ara[3][0] = 0, digit[4].ara[3][1] = 0, digit[4].ara[3][2] = 1;
	digit[4].ara[4][0] = 0, digit[4].ara[4][1] = 0, digit[4].ara[4][2] = 1;
	//digit 5
	digit[5].ara[0][0] = 1, digit[5].ara[0][1] = 1, digit[5].ara[0][2] = 1;
	digit[5].ara[1][0] = 1, digit[5].ara[1][1] = 0, digit[5].ara[1][2] = 0;
	digit[5].ara[2][0] = 1, digit[5].ara[2][1] = 1, digit[5].ara[2][2] = 1;
	digit[5].ara[3][0] = 0, digit[5].ara[3][1] = 0, digit[5].ara[3][2] = 1;
	digit[5].ara[4][0] = 1, digit[5].ara[4][1] = 1, digit[5].ara[4][2] = 1;
	//digit 6
	digit[6].ara[0][0] = 1, digit[6].ara[0][1] = 1, digit[6].ara[0][2] = 1;
	digit[6].ara[1][0] = 1, digit[6].ara[1][1] = 0, digit[6].ara[1][2] = 0;
	digit[6].ara[2][0] = 1, digit[6].ara[2][1] = 1, digit[6].ara[2][2] = 1;
	digit[6].ara[3][0] = 1, digit[6].ara[3][1] = 0, digit[6].ara[3][2] = 1;
	digit[6].ara[4][0] = 1, digit[6].ara[4][1] = 1, digit[6].ara[4][2] = 1;
	//digit 7
	digit[7].ara[0][0] = 1, digit[7].ara[0][1] = 1, digit[7].ara[0][2] = 1;
	digit[7].ara[1][0] = 0, digit[7].ara[1][1] = 0, digit[7].ara[1][2] = 1;
	digit[7].ara[2][0] = 0, digit[7].ara[2][1] = 0, digit[7].ara[2][2] = 1;
	digit[7].ara[3][0] = 0, digit[7].ara[3][1] = 1, digit[7].ara[3][2] = 0;
	digit[7].ara[4][0] = 0, digit[7].ara[4][1] = 1, digit[7].ara[4][2] = 0;
	//digit 8
	digit[8].ara[0][0] = 1, digit[8].ara[0][1] = 1, digit[8].ara[0][2] = 1;
	digit[8].ara[1][0] = 1, digit[8].ara[1][1] = 0, digit[8].ara[1][2] = 1;
	digit[8].ara[2][0] = 1, digit[8].ara[2][1] = 1, digit[8].ara[2][2] = 1;
	digit[8].ara[3][0] = 1, digit[8].ara[3][1] = 0, digit[8].ara[3][2] = 1;
	digit[8].ara[4][0] = 1, digit[8].ara[4][1] = 1, digit[8].ara[4][2] = 1;
	//digit 9
	digit[9].ara[0][0] = 1, digit[9].ara[0][1] = 1, digit[9].ara[0][2] = 1;
	digit[9].ara[1][0] = 1, digit[9].ara[1][1] = 0, digit[9].ara[1][2] = 1;
	digit[9].ara[2][0] = 1, digit[9].ara[2][1] = 1, digit[9].ara[2][2] = 1;
	digit[9].ara[3][0] = 0, digit[9].ara[3][1] = 0, digit[9].ara[3][2] = 1;
	digit[9].ara[4][0] = 1, digit[9].ara[4][1] = 1, digit[9].ara[4][2] = 1;
}



//	prints the number to 3 digits


void printScore(int num)
{
	
	memset(pattern,0,sizeof pattern);
	
	for(int i =0 ; i < 16;i++) {
		pattern[i][0] = 2;
		pattern[0][i] = 2;
		pattern[15-i][0] = 2;
	    pattern[0][15-i] = 2;
		pattern[15][15-i] = 2;
		pattern[15-i][15] = 2;
	}
	int r = 6, col = 3;
	int mul = 100;
	while(mul){
		int curDig = num / mul;
		
		for(int i = 0; i < 5;i++){
			for(int j = 0; j < 3; j++){
				pattern[r+i][col+j] = digit[curDig].ara[i][j];
			}
		}
		
		num %= mul;
		mul /= 10;
		col += 4;
	}
		
	int cnt = 100;
	while(1){
		printScreen();	
	}

}

void startScreen()
{

	int r = 5, col = 6;
	int step = 3;
	do{
		memset(pattern,0,sizeof pattern);
		
		for(int i = 0; i < 16; i++){
			pattern[i][0] = 2;
			pattern[0][i] = 2;
			pattern[15-i][0] = 2;
			pattern[0][15-i] = 2;
			pattern[15][15-i] = 2;
			pattern[15-i][15] = 2;
		}
		
		for(int i = 0; i < 5; i++){
			for(int j = 0; j < 3; j++){
				pattern[r+i][col+j] = digit[step].ara[i][j];
			}
		}
		int cnt = 200;
		while(cnt--){
			printScreen();
		}	
		step--;
	}while(step>=0);
	
	memset(pattern,0,sizeof pattern);
	
	for(int i = 0; i < 16; i++){
		pattern[i][0] = 2;
		pattern[0][i] = 2;
		pattern[15-i][0] = 2;
		pattern[0][15-i] = 2;
		pattern[15][15-i] = 2;
		pattern[15-i][15] = 2;
	}
	
	r = 5, col = 3;
	
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 3; j++){
			pattern[r+i][col+j] = digit[0].ara[i][j] ;
		}
	}
	pattern[r+1][col+2] = 0;
	pattern[r+2][col+1] = 1;
	col += 4;
	
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 3; j++){
			pattern[r+i][col+j] = digit[0].ara[i][j] ;
		}
	}
	
	col += 5;
	
	for(int i = r; i <= r+2; i++){
		pattern[i][col] = 1;
	}
	pattern[r+4][col] = 1;
	
	int cnt = 100;
	
	while(cnt--){
		printScreen();
	}
}

int main()
{
	
	portInit();	
	/*
	setGreenBars();
	setEnemies();
	setSiam();
	*/
	
	createDigit();
	
	/*
	
	while(1) {
		
		
		game_delay++;
		jump_delay++;
		game_delay %= GAME_DELAY_VAL;	
		jump_delay %= JUMP_DELAY_VAL;
		
		UpdateGreenBars();
		UpdateBullet();
		
		UpdateSiam();//ALWAYS UPDATE SIAM & enemies AFTER GREEN BARS
		UpdateEnemies();
		
		
		printScreen();
		
		if(siam.lifeLeft == 0 ) {
			printScore(currentGreenBar);	
			continue;
		}
		if(right_move_condition_found()) {
			moveRight();
		}
		printScreen();

		if(jump_condition_found()) {
			jumpUp();
		}
		
		printScreen();
		
		if(shoot_condition_found()) {
			shoot();
		}
		
		printScreen();
		
		
	}
	
	for(int i = 0; i < 16; i++){
		for(int j = 0; j < 16; j++) pattern[i][j] = 1;
	}
	
	*/
	
	startScreen();
	
	while(1){
		printScreen();
	}
	//printScore(983);
	
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
				//_delay_ms(20);
			}
		}
	}
	return 0;
}
*/