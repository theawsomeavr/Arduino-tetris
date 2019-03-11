/**********************************************************
 * This is a version of annejan´s tetris code https://github.com/IJHack/tetris modified by David Rubio 
 * so that you can play tetris on 2 lcds(annejan´s tetris code) and at the same time you are hearing 
 * the tetris theme play by the same MCU/arduino board without any external Integrated Circuits (the music is outputted through pin 9)
 * and at game over present the current score (number of lines that you have eliminated) as well as the highest score (that is saved on the eeprom).
 * It is very important the you have wipe the eeprom before you upload this code https://www.arduino.cc/en/Tutorial/EEPROMClear
 *********************************************************/
#include "LedControl_for_tetris.h"
#include "PWM_Playtune_for_tetris.h"
#include <EEPROM.h>
#include "charac.h"
#include "music.h"
byte buffer[10];
PWM_Playtune pt;
long delays = 0;
short delay_ = 500;
long bdelay = 0;
short buttondelay = 200;
short btdowndelay = 80;
short btsidedelay = 200;
unsigned char blocktype;
unsigned char blockrotation;
int points;
boolean  block[8][18]; //2 extra for rotation
boolean  pile[8][16];
boolean disp[8][16];
static const int DATA_PIN = 2;
static const int CLK_PIN  = 3;
static const int CS_PIN   = 4;
static const int DISPLAYS = 2;

LedControl lc = LedControl(DATA_PIN, CLK_PIN, CS_PIN, DISPLAYS);


void setup() {
  int seed =
    (analogRead(0) + 1) *
    (analogRead(1) + 1) *
    (analogRead(2) + 1) *
    (analogRead(3) + 1);
  randomSeed(seed);
  random(10, 9610806);
  seed = seed * random(3336, 15679912) + analogRead(random(4)) ;
  randomSeed(seed);
  random(10, 98046);

  // The MAX72XX is in power-saving mode on startup,
  // we have to do a wakeup call
  lc.shutdown(0, false);
  lc.shutdown(1, false);
  // Set the brightness
  lc.setIntensity(0, 10);
  lc.setIntensity(1, 10);
  // and clear the display
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  pinMode(5, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pt.begin();
  //you can use the funtion pt.set_volume to set the volume of the music, you must input a value between 0-100.
  //pt.set_volume(50);
  newBlock();
  updateLED();


}

void loop() {

  if (delays < millis())
  {
    delays = millis() + delay_;
    movedown();
  }



  if (!pt.tune_playing) pt.tune_playscore (score);

  //buttun actions
  int button = readDirection();

  if (button == 1) //up=rotate
    rotate();
  if (button == 2) //right=moveright
    moveright();
  if (button == 3) //left=moveleft
    moveleft();
  if (button == 4) //down=movedown
    movedown();



  LEDRefresh();
}

boolean moveleft()
{
  if (space_left())
  {
    int i;
    int j;
    for (i = 0; i < 7; i++)
    {
      for (j = 0; j < 16; j++)
      {
        block[i][j] = block[i + 1][j];
      }
    }

    for (j = 0; j < 16; j++)
    {
      block[7][j] = 0;
    }

    updateLED();
    return 1;
  }

  return 0;
}

boolean moveright()
{
  if (space_right())
  {
    int i;
    int j;
    for (i = 7; i > 0; i--)
    {
      for (j = 0; j < 16; j++)
      {
        block[i][j] = block[i - 1][j];
      }
    }

    for (j = 0; j < 16; j++)
    {
      block[0][j] = 0;
    }

    updateLED();
    return 1;

  }
  return 0;
}

int readDirection()
{
  if (bdelay > millis())
  {
    return 0;
  }
  if (digitalRead(5) == 0)
  {
    //left
    bdelay = millis() + btsidedelay;
    return 3;
  }

  if (digitalRead(6) == 0)
  {
    //down
    bdelay = millis() + btdowndelay;
    return 4;
  }
  if (digitalRead(7) == 0)
  {
    //right
    bdelay = millis() + btsidedelay;
    return 2;
  }
  if (digitalRead(8) == 0)
  {
    //up
    bdelay = millis() + buttondelay;
    return 1;
  }

  return 0;
}

void updateLED()
{
  int i;
  int j;
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
      disp[i][j] = block[i][j] | pile[i][j];
    }
  }
}

void rotate()
{

  //skip for square block(3)
  if (blocktype == 3) return;

  int xi;
  int yi;
  int i;
  int j;
  //detect left
  for (i = 7; i >= 0; i--)
  {
    for (j = 0; j < 16; j++)
    {
      if (block[i][j])
      {
        xi = i;
      }
    }
  }

  //detect up
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        yi = i;
      }
    }
  }

  if (blocktype == 0)
  {
    if (blockrotation == 0)
    {


      if (!space_left())
      {
        if (space_right3())
        {
          if (!moveright())
            return;
          xi++;
        }
        else return;
      }
      else if (!space_right())
      {
        if (space_left3())
        {
          if (!moveleft())
            return;
          if (!moveleft())
            return;
          xi--;
          xi--;
        }
        else
          return;
      }
      else if (!space_right2())
      {
        if (space_left2())
        {
          if (!moveleft())
            return;
          xi--;
        }
        else
          return;
      }





      block[xi][yi] = 0;
      block[xi][yi + 2] = 0;
      block[xi][yi + 3] = 0;

      block[xi - 1][yi + 1] = 1;
      block[xi + 1][yi + 1] = 1;
      block[xi + 2][yi + 1] = 1;

      blockrotation = 1;
    }
    else
    {
      block[xi][yi] = 0;
      block[xi + 2][yi] = 0;
      block[xi + 3][yi] = 0;

      block[xi + 1][yi - 1] = 1;
      block[xi + 1][yi + 1] = 1;
      block[xi + 1][yi + 2] = 1;

      blockrotation = 0;
    }
  }

  //offset to mid
  xi ++;
  yi ++;

  if (blocktype == 1)
  {
    if (blockrotation == 0)
    {
      block[xi - 1][yi - 1] = 0;
      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;

      block[xi][yi - 1] = 1;
      block[xi + 1][yi - 1] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 1;
    }
    else if (blockrotation == 1)
    {
      if (!space_left())
      {
        if (!moveright())
          return;
        xi++;
      }
      xi--;

      block[xi][yi - 1] = 0;
      block[xi + 1][yi - 1] = 0;
      block[xi][yi + 1] = 0;

      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;
      block[xi + 1][yi + 1] = 1;

      blockrotation = 2;
    }
    else if (blockrotation == 2)
    {
      yi --;

      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;
      block[xi + 1][yi + 1] = 0;

      block[xi][yi - 1] = 1;
      block[xi][yi + 1] = 1;
      block[xi - 1][yi + 1] = 1;

      blockrotation = 3;
    }
    else
    {
      if (!space_right())
      {
        if (!moveleft())
          return;
        xi--;
      }
      block[xi][yi - 1] = 0;
      block[xi][yi + 1] = 0;
      block[xi - 1][yi + 1] = 0;

      block[xi - 1][yi - 1] = 1;
      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;

      blockrotation = 0;
    }
  }



  if (blocktype == 2)
  {
    if (blockrotation == 0)
    {
      block[xi + 1][yi - 1] = 0;
      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;

      block[xi][yi - 1] = 1;
      block[xi + 1][yi + 1] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 1;
    }
    else if (blockrotation == 1)
    {
      if (!space_left())
      {
        if (!moveright())
          return;
        xi++;
      }
      xi--;

      block[xi][yi - 1] = 0;
      block[xi + 1][yi + 1] = 0;
      block[xi][yi + 1] = 0;

      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;
      block[xi - 1][yi + 1] = 1;

      blockrotation = 2;
    }
    else if (blockrotation == 2)
    {
      yi --;

      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;
      block[xi - 1][yi + 1] = 0;

      block[xi][yi - 1] = 1;
      block[xi][yi + 1] = 1;
      block[xi - 1][yi - 1] = 1;

      blockrotation = 3;
    }
    else
    {
      if (!space_right())
      {
        if (!moveleft())
          return;
        xi--;
      }
      block[xi][yi - 1] = 0;
      block[xi][yi + 1] = 0;
      block[xi - 1][yi - 1] = 0;

      block[xi + 1][yi - 1] = 1;
      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;

      blockrotation = 0;
    }
  }

  if (blocktype == 4)
  {
    if (blockrotation == 0)
    {
      block[xi + 1][yi - 1] = 0;
      block[xi - 1][yi] = 0;

      block[xi + 1][yi] = 1;
      block[xi + 1][yi + 1] = 1;

      blockrotation = 1;
    }
    else
    {
      if (!space_left())
      {
        if (!moveright())
          return;
        xi++;
      }
      xi--;

      block[xi + 1][yi] = 0;
      block[xi + 1][yi + 1] = 0;

      block[xi - 1][yi] = 1;
      block[xi + 1][yi - 1] = 1;

      blockrotation = 0;
    }
  }


  if (blocktype == 5)
  {
    if (blockrotation == 0)
    {
      block[xi][yi - 1] = 0;
      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;

      block[xi][yi - 1] = 1;
      block[xi + 1][yi] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 1;
    }
    else if (blockrotation == 1)
    {
      if (!space_left())
      {
        if (!moveright())
          return;
        xi++;
      }
      xi--;

      block[xi][yi - 1] = 0;
      block[xi + 1][yi] = 0;
      block[xi][yi + 1] = 0;

      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 2;
    }
    else if (blockrotation == 2)
    {
      yi --;

      block[xi - 1][yi] = 0;
      block[xi + 1][yi] = 0;
      block[xi][yi + 1] = 0;

      block[xi][yi - 1] = 1;
      block[xi - 1][yi] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 3;
    }
    else
    {
      if (!space_right())
      {
        if (!moveleft())
          return;
        xi--;
      }
      block[xi][yi - 1] = 0;
      block[xi - 1][yi] = 0;
      block[xi][yi + 1] = 0;

      block[xi][yi - 1] = 1;
      block[xi - 1][yi] = 1;
      block[xi + 1][yi] = 1;

      blockrotation = 0;
    }
  }

  if (blocktype == 6)
  {
    if (blockrotation == 0)
    {
      block[xi - 1][yi - 1] = 0;
      block[xi][yi - 1] = 0;

      block[xi + 1][yi - 1] = 1;
      block[xi][yi + 1] = 1;

      blockrotation = 1;
    }
    else
    {
      if (!space_left())
      {
        if (!moveright())
          return;
        xi++;
      }
      xi--;

      block[xi + 1][yi - 1] = 0;
      block[xi][yi + 1] = 0;

      block[xi - 1][yi - 1] = 1;
      block[xi][yi - 1] = 1;

      blockrotation = 0;
    }
  }







  //if rotating made block and pile overlap, push rows up
  while (!check_overlap())
  {
    for (i = 0; i < 18; i++)
    {
      for (j = 0; j < 8; j++)
      {
        block[j][i] = block[j][i + 1];
      }
    }
    delays = millis() + delay_;
  }


  updateLED();



}

void movedown()
{
  if (space_below())
  {
    //move down
    int i;
    for (i = 15; i >= 0; i--)
    {
      int j;
      for (j = 0; j < 8; j++)
      {
        block[j][i] = block[j][i - 1];
      }
    }
    for (i = 0; i < 7; i++)
    {
      block[i][0] = 0;
    }
  }
  else
  {
    //merge and new block
    int i;
    int j;
    for (i = 0; i < 8; i++)
    {
      for (j = 0; j < 16; j++)
      {
        if (block[i][j])
        {
          pile[i][j] = 1;
          block[i][j] = 0;
        }
      }
    }
    newBlock();
  }
  updateLED();
}

boolean check_overlap()
{
  int i;
  int j;
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 7; j++)
    {
      if (block[j][i])
      {
        if (pile[j][i])
          return false;
      }
    }
  }
  for (i = 16; i < 18; i++)
  {
    for (j = 0; j < 7; j++)
    {
      if (block[j][i])
      {
        return false;
      }
    }
  }
  return true;
}

void check_gameover()
{
  int i;
  int j;
  int cnt = 0;;

  for (i = 15; i >= 0; i--)
  {
    cnt = 0;
    for (j = 0; j < 8; j++)
    {
      if (pile[j][i])
      {
        cnt ++;
      }
    }
    if (cnt == 8)
    {
      points++;
      for (j = 0; j < 8; j++)
      {
        pile[j][i] = 0;
      }
      updateLED();
      delay(50);

      int k;
      for (k = i; k > 0; k--)
      {
        for (j = 0; j < 8; j++)
        {
          pile[j][k] = pile[j][k - 1];
        }
      }
      for (j = 0; j < 8; j++)
      {
        pile[j][0] = 0;
      }
      updateLED();
      delay(50);
      i++;



    }
  }


  for (i = 0; i < 8; i++)
  {
    if (pile[i][0])
      gameover();
  }
  return;
}

void gameover()
{
  pt.tune_stopscore();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2A = 0;
  pinMode(9, 0);
  bool high_score = 0;
  if (points > EEPROM.read(0)) {
    EEPROM.write(0, points);
    high_score = 1;
  }
  int i;
  int j;
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {

      disp[i][j] = 0;
    }
  }


  LEDRefresh();
  char data1[10];
  char data2[10];

  sprintf(data1, "%i", points);
  sprintf(data2, "%i", EEPROM.read(0));

  while (true)
  {
    if (high_score == 1) {
      printStringWithShift("new high score  ", 80);
      printStringWithShift(data1, 80);
      printStringWithShift("    ", 80);
    }
    else {
      printStringWithShift("score ", 80);
      printStringWithShift(data1, 80);
      printStringWithShift("    ", 80);
      printStringWithShift("highest score ", 80);
      printStringWithShift(data2, 80);
      printStringWithShift("    ", 80);
    }
  }

}

void newBlock()
{
  check_gameover();


  blocktype = random(7);


  if (blocktype == 0)
    // 0
    // 0
    // 0
    // 0
  {
    block[3][0] = 1;
    block[3][1] = 1;
    block[3][2] = 1;
    block[3][3] = 1;
  }

  if (blocktype == 1)
    // 0
    // 0 0 0
  {
    block[2][0] = 1;
    block[2][1] = 1;
    block[3][1] = 1;
    block[4][1] = 1;
  }

  if (blocktype == 2)
    //     0
    // 0 0 0
  {
    block[4][0] = 1;
    block[2][1] = 1;
    block[3][1] = 1;
    block[4][1] = 1;
  }

  if (blocktype == 3)
    // 0 0
    // 0 0
  {
    block[3][0] = 1;
    block[3][1] = 1;
    block[4][0] = 1;
    block[4][1] = 1;
  }

  if (blocktype == 4)
    //   0 0
    // 0 0
  {
    block[4][0] = 1;
    block[5][0] = 1;
    block[3][1] = 1;
    block[4][1] = 1;
  }

  if (blocktype == 5)
    //   0
    // 0 0 0
  {
    block[4][0] = 1;
    block[3][1] = 1;
    block[4][1] = 1;
    block[5][1] = 1;
  }

  if (blocktype == 6)
    // 0 0
    //   0 0
  {
    block[3][0] = 1;
    block[4][0] = 1;
    block[4][1] = 1;
    block[5][1] = 1;
  }

  blockrotation = 0;
}

boolean space_below()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (i == 15)
          return false;
        if (pile[j][i + 1])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_left2()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 0 || j == 1)
          return false;
        if (pile[j - 1][i] | pile[j - 2][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_left3()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 0 || j == 1 || j == 2 )
          return false;
        if (pile[j - 1][i] | pile[j - 2][i] | pile[j - 3][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_left()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 0)
          return false;
        if (pile[j - 1][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_right()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 7)
          return false;
        if (pile[j + 1][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_right3()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 7 || j == 6 || j == 5)
          return false;
        if (pile[j + 1][i] | pile[j + 2][i] | pile[j + 3][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

boolean space_right2()
{
  int i;
  int j;
  for (i = 15; i >= 0; i--)
  {
    for (j = 0; j < 8; j++)
    {
      if (block[j][i])
      {
        if (j == 7 || j == 6)
          return false;
        if (pile[j + 1][i] | pile[j + 2][i])
        {
          return false;
        }
      }
    }
  }
  return true;
}

void LEDRefresh()
{
  int i;
  int k;
  //////////////////////////////////////////////

  for (i = 0; i < 8; i++)
  {
    byte upper = 0;
    int b;
    for (b = 0; b < 8; b++)
    {
      upper <<= 1;
      if (disp[b][i]) upper |= 1;
    }

    lc.setRow(1, i, upper);



    byte lower = 0;
    for (b = 0; b < 8; b++)
    {
      lower <<= 1;
      if (disp[b][i + 8]) lower |= 1;
    }

    lc.setRow(0, i, lower);

   
  }
}
void printCharWithShift(char c, int shift_speed) {   
  if (c < 32) return;
  c -= 32;
  memcpy_P(buffer, CH + 7 * c, 7);
  lc.writeSprite(DISPLAYS * 8, 0, buffer);
  lc.setColumn(DISPLAYS * 8 + buffer[0], 0);

  for (int i = 0; i < buffer[0] + 1; i++)
  {
    delay(shift_speed);
    lc.shiftLeft(false, false);
  }
}

void printStringWithShift(char* s, int shift_speed) {      
  while (*s != 0) {
    printCharWithShift(*s, shift_speed);
    s++;
  }
}

void printString(char* s)                       
{
  int col = 0;
  while (*s != 0)
  {
    if (*s < 32) continue;
    char c = *s - 32;
    memcpy_P(buffer, CH + 7 * c, 7);
    lc.writeSprite(col, 0, buffer);
    lc.setColumn(col + buffer[0], 0);
    col += buffer[0] + 1;
    s++;
  }
}
