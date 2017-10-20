#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#define SHARP_SCK   13
#define SHARP_MOSI  11
#define SHARP_SS    10

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 96, 96);

#define BLACK 0
#define WHITE 1

#define FIELD_WIDTH 16
#define SQUARE_SIZE 96 / FIELD_WIDTH
#define NUM_FIELD_BYTES (FIELD_WIDTH * FIELD_WIDTH) / 8

//  allocate memory for the fields
unsigned char fieldA[NUM_FIELD_BYTES];
unsigned char fieldB[NUM_FIELD_BYTES];

//  make a pointer to current field and nextField
unsigned char *currentField;
unsigned char *nextField;
unsigned char *swapField;

void setup() {

  //  seed the random generator
  randomSeed( analogRead(0) );

  //  set the pointer to the current field
  currentField = fieldA;
  nextField = fieldB;

  //  fill the field with random values
  for( unsigned char i = 0; i < NUM_FIELD_BYTES; i++ )
  {
    currentField[i] = random( 256 );
  }

  
  display.begin();
  display.clearDisplay();
  display.refresh();
}

void loop() {

  //  draw the field
  drawField();
  display.refresh();

  //  apply conways game of life
  applyLogic();
  
  //  swap the fields
  swapFields();
}


//  function to get the value of a bit based on x and y
boolean getBit( unsigned char x, unsigned char y )
{
  //  get the bit index for a one dimensional array
  unsigned int index = x + y * FIELD_WIDTH;
  //  get which char the bit is in
  unsigned char charNumber = index / 8;
  //  get which bit of 8 in the single char is the one we're looking for
  unsigned char charSubIndex = index - (8 * charNumber);
  //  get the bit value
  unsigned char bitChar = currentField[charNumber];
  boolean bitValue =  bitRead( bitChar, charSubIndex );  //( bitChar >> charSubIndex ) & 00000001 ;

  return bitValue;
}

//  function to set the value of a bit based on x and y
void setBit( unsigned char x, unsigned char y, boolean newBitValue )
{
  //  get the bit index for a one dimensional array
  unsigned int index = x + y * FIELD_WIDTH;
  //  get which char the bit is in
  unsigned char charNumber = index / 8;
  //  get which bit of 8 in the single char is the one we're looking for
  unsigned char charSubIndex = index - (8 * charNumber);
  //  get the bit value
  unsigned char bitChar = nextField[charNumber];

  if( newBitValue == true )
  {
    //  create the mask
    byte mask = 00000001 << charSubIndex;
    //  apply the mask
    nextField[charNumber] = bitChar | mask ;
  }
  else
  {
    //  create the negative mask
    byte mask = ~( 00000001 << charSubIndex);
    //  apply the mask
    nextField[charNumber] = bitChar & mask ;
  }
}

//  function to draw the field
void drawField()
{
  unsigned char squareX = 0;
  unsigned char squareY = 0;
  for( unsigned char y = 0; y < FIELD_WIDTH; y++ )
  {
    squareX = 0;
    for( unsigned char x = 0; x < FIELD_WIDTH; x++ )
    {
      
      boolean bitValue = getBit( x, y );
      display.fillRect( squareX, squareY, SQUARE_SIZE, SQUARE_SIZE, bitValue );
      squareX += SQUARE_SIZE;
    }
    squareY += SQUARE_SIZE;
  }
}

//  function to swap the field pointers
void swapFields()
{
  swapField = currentField;
  currentField = nextField;
  nextField = swapField;
}

//  apply cellular automota logic to entire field
void applyLogic()
{
  
  //  go through each cell
  for( signed char y = 0; y < FIELD_WIDTH; y++ )
  {
    for( signed char x = 0; x < FIELD_WIDTH; x++ )
    {
      unsigned char neighborTotal = 0;
      
      //  total the number of alive neighbors
      //  //  check top left neighbor
      if( checkUpperBounds( x - 1, y - 1 ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check upper neighbor
      if( checkUpperBounds( x , y - 1 ) )
      {
        if( getBit( checkWrapAround( x ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check top right neighbor
      if( checkUpperBounds( x + 1, y - 1 ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y - 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check left neighbor
      if( checkUpperBounds( x - 1, y ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y )) )
        {
          neighborTotal++;
        }
      }
      //  //  check right neighbor
      if( checkUpperBounds( x + 1, y ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y )) )
        {
          neighborTotal++;
        }
      }
      //  //  check bottom left neighbor
      if( checkUpperBounds( x - 1, y + 1 ) )
      {
        if( getBit( checkWrapAround( x - 1 ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check lower neighbor
      if( checkUpperBounds( x, y + 1 ) )
      {
        if( getBit( checkWrapAround( x ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  //  check bottom right neighbor
      if( checkUpperBounds( x + 1, y + 1 ) )
      {
        if( getBit( checkWrapAround( x + 1 ), checkWrapAround( y + 1 )) )
        {
          neighborTotal++;
        }
      }
      //  check rule
      boolean cellValue = getBit( x, y );
      boolean nextCellValue = false;
      if( cellValue ) //  if cell is currently alive
      {
        //  2 or 3 neighbors lives on
        if( neighborTotal == 2 || neighborTotal == 3 )
        {
          nextCellValue = true;  
        }
        //  fewer than 2 neighbors means death
        //  more than 3 neighbors means death
        else
        {
          nextCellValue = false;
        }
        
      }
      else  //  if cell is currently dead
      {
        //  3 neighbors becomes alive
        if( neighborTotal == 3 )
        {
          nextCellValue = true;  
        }
        else
        {
          nextCellValue = false;  
        }
      }

      //  set next state in next field frame
      setBit( x, y, nextCellValue );
    }
  }
}


//  wrap around coordinates
unsigned char checkWrapAround( signed char a )
{
  if( a < 0 )
  {
    return FIELD_WIDTH + a;
  }
  else
  {
    return a;
  }
}

//  check upper bounds
boolean checkUpperBounds( signed char x, signed char y )
{
    if( (x < FIELD_WIDTH) && (y < FIELD_WIDTH) )
    {
      return true;  
    }
    else
    {
      return false;
    }
}
