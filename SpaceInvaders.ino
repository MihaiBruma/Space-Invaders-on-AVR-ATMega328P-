#include <U8g2lib.h> 
#include <EEPROM.h>   
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// OLED Screen Parameters & Adress.
#define OledDisplayAdress 0x3C
#define ScreenWidth 128
#define ScreenHeight 64

// Right - Left - Shot  Buttons.
#define ShotButton 5
#define RightButton 4
#define LeftButton 6

// Head Ship.
#define HeadShipHeight 4
#define HeadShipWidth 16
#define HeadshipSpeed 2
#define HeadShipSpawn 250         
#define HeadShipBunusDisplay 20    

// Alien Settings.
#define AlienHeightonOLED 8
#define AlienColumns 6
#define AlienRows 3
#define XcoordinateStart 6
#define SpaceBetweenColumnsAliens 5
#define AlienWidthLargestValue 11
#define SpaceBetweenRows 9
#define InvadersAttackDrop 4                  
#define InvadersSpeedValue 15                  
#define InvadersHeightonScreen 8
#define ExplosionTime 7                
#define YcoordinateStart HeadShipHeight-1
#define AmountToDropPerLevel 4       
#define LevelToResetHeight 4    
#define AlienXcoordinateMovingValue 1               
#define ChanceofBombDropping 80             
#define BombHeight 4
#define BombWidth 2
#define MaximumValueBombs 3                     

// Bomb Collision.
#define BombDamageChance 20    
#define BombPenetrationChanceDOWNway 1          

// USER(Player) parameters.
#define TankGraphicsWidth 13
#define TankGraphicsHeight 8
#define PlayerXcoordinateMovingValue 2
#define LIVES 3                             
#define PlayerExplosionTime 10            
#define PlayerYcoordinateStart 56
#define PlayerXcoordinateStart 0
#define BaseWidth 16
#define BaseWidthValueInBytes 2
#define BaseHeight 8
#define BaseOnY 46
#define NumberOfObstacles 3
#define BaseMargins 10
#define MissileHeight 4
#define MissileWidth 1
#define MissileSpeed 4

//Playing Status.
#define ACTIVE 0
#define EXPLODING 1
#define DESTROYED 2

// Graphics converted to hex values.
const byte HeadShipGraphics[][8] PROGMEM = {
  { 0xfc, 0x3f, 0xb6, 0x6d, 0xff, 0xff, 0x9c, 0x39},
  { 0xfc, 0x00, 0x4a, 0x01, 0xff, 0x03, 0xb5, 0x02}
};

const byte TopInvaderGraphics[][8] PROGMEM = {
  { 0x18, 0x3c, 0x7e, 0xdb, 0xff, 0x24, 0x5a, 0xa5},
  { 0x18, 0x3c, 0x7e, 0xdb, 0xff, 0x5a, 0x81, 0x42}
};

const byte MiddleInvaderGraphics[][16] PROGMEM = {
  { 0x04, 0x01, 0x88, 0x00, 0xfc, 0x01, 0x76, 0x03, 0xff, 0x07, 0xfd, 0x05, 0x05, 0x05, 0xd8, 0x00},
  { 0x04, 0x01, 0x88, 0x00, 0xfd, 0x05, 0x75, 0x05, 0xff, 0x07, 0xfc, 0x01, 0x04, 0x01, 0x02, 0x02}
};

const byte BottomInvaderGraphics[][16] PROGMEM = {
  { 0xf0, 0x00, 0xfe, 0x07, 0xff, 0x0f, 0x67, 0x0e, 0xff, 0x0f, 0x9c, 0x03, 0x06, 0x06, 0x0c, 0x03},
  { 0xf0, 0x00, 0xfe, 0x07, 0xff, 0x0f, 0x67, 0x0e, 0xff, 0x0f, 0x9c, 0x03, 0x62, 0x04, 0x01, 0x08}
};

//USER(Player) Graphics.
const byte TankAttackGraphics[] PROGMEM = {
  0x40, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f
};

const byte MissileAttackGraphics[] PROGMEM = {
  0x01, 0x01, 0x01, 0x01
};

const byte AlienAttackGraphics[] PROGMEM = {
  0x01, 0x02, 0x01, 0x02
};

const byte BaseDefenceGraphics[] PROGMEM = {
  0xf8, 0x1f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xf8, 0x07, 0xe0, 0x07, 0xe0
};

const byte ExplosionGraphics[] PROGMEM = {
  0x10, 0x01, 0xa2, 0x08, 0x04, 0x04, 0x08, 0x02, 0x03, 0x18, 0x08, 0x02, 0xa4, 0x04, 0x12, 0x09
};

//Game Structs.
struct SpaceInvadersObjectStruct  {
  signed int X;
  signed int Y;
  byte Status;             
};

struct BaseDefenceStruct {
  SpaceInvadersObjectStruct Obj;
  byte Graphics[16];
};

struct AlienStruct  {
  SpaceInvadersObjectStruct Obj;
  byte ExplosionGraphicsCounter;   
};

struct UserStruct  {
  SpaceInvadersObjectStruct Obj;
  int Score;
  byte Lives;
  byte Level;
  byte AliensDestroyed;     
  byte AlienSpeed;          
  byte ExplosionGraphicsCounter;
};

// Aliens and Screen
AlienStruct Alien[AlienColumns][AlienRows];
AlienStruct HeadShip;
SpaceInvadersObjectStruct AlienBomb[MaximumValueBombs];
BaseDefenceStruct Base[NumberOfObstacles];

static const byte TotalAmountOfAliens PROGMEM = AlienColumns * AlienRows; 

byte AlienWidth[] = {8, 11, 12};           

char AlienXMoveAmount = 2;
signed char InvadersMoveCounter;          
bool AnimationFrame = false;                

// Headship
signed char HeadShipSpeed;
int HeadShipBonus;
signed int HeadShipBonusOnXPosition;             
byte HeadShipBonusCounter;                
byte HeadShipType;                        

//USER(Player) Variables.
UserStruct Player;
SpaceInvadersObjectStruct Missile;

//Game Variables.
int HighestScore;
bool GameInPlay = false;
byte FONT_Ascent;
byte FONT_Descent;

void setup() {

  oled.begin();
  oled.clear();
  oled.setBitmapMode(1);
  InitAliens(0);
  InitializePlayer();

  pinMode(ShotButton, INPUT_PULLUP);
  pinMode(RightButton, INPUT_PULLUP);
  pinMode(LeftButton, INPUT_PULLUP);

  oled.setFont(u8g2_font_t0_11b_tf);       
  FONT_Ascent = oled.getAscent();          
  FONT_Descent = -oled.getDescent();      
  oled.setDrawColor(1);                    

  EEPROM.get(0, HighestScore);
  if (HighestScore == 65535) {                     
    HighestScore = 0;
    EEPROM.put(0, HighestScore);
  }
}

void loop() {
 
  if (GameInPlay)
  {
    GamePhysics();
    UpdateDisplay();
  }
  else
    AttractScreen();
}

//Subroutines and Additional Functions.
void AttractScreen() {
byte RowHeight;
byte ColumnPos;
  
  ColumnPos = oled.getStrWidth("8");

  if (HighestScore < 10) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Hi Score ") - ColumnPos) / 2.0);
  } else if (HighestScore < 100) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Hi Score ") - ColumnPos * 2) / 2.0);
  } else if (HighestScore < 1000) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Hi Score ") - ColumnPos * 3) / 2.0);
  } else if (HighestScore < 10000) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Hi Score ") - ColumnPos * 4) / 2.0);
  } else {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Hi Score ") - ColumnPos * 5) / 2.0);
    //65535 maximum
  }

  oled.clearBuffer();
  RowHeight = FONT_Ascent + (ScreenHeight - 4 * (FONT_Ascent + FONT_Descent + 1)) / 2;
  CenterText("Space Invaders", RowHeight);
  // Debugging Process
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Author:Bruma M.", RowHeight);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Press Shot to start", RowHeight);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  
  oled.setCursor(ColumnPos, RowHeight);
  oled.print("BSW Tech,2023");
 
  oled.sendBuffer();
  if ((digitalRead(ShotButton) == 0)) {
    GameInPlay = true;
    NewGame();
  }
  if (digitalRead(ShotButton) == 0)
  {
    HighestScore = 0;
    EEPROM.put(0, HighestScore);
  }
}

void GamePhysics() {
  if (Player.Obj.Status == ACTIVE) {
    AlienControl();
    HeadShipPhysics();
    PlayerControl();
    MissileControl();
    CheckCollisions();
  }
}

byte GetScoreForAlien(int RowNumber) {
  switch (RowNumber)
  {
    case 0: return 30;
    case 1: return 20;
    case 2: return 10;
  }
}

void HeadShipPhysics() {
  if (HeadShip.Obj.Status == ACTIVE)  {   
    HeadShip.Obj.X += HeadShipSpeed;
    if (HeadShipSpeed > 0)               
    {
      if (HeadShip.Obj.X >= ScreenWidth)
      {
        HeadShip.Obj.Status = DESTROYED;
      }
    }
    else                                    
    {
      if (HeadShip.Obj.X + HeadShipWidth < 0)
      {
        HeadShip.Obj.Status = DESTROYED;
      }
    }
  }
  else  {
    if (random(HeadShipSpawn) == 1)
    {
      HeadShip.Obj.Status = ACTIVE;
     
      HeadShipType = random(2);         
      if (random(2) == 1)             
      {
        HeadShip.Obj.X = ScreenWidth;
        HeadShipSpeed = -HeadShipSpeed; 
      }
      else
      {
        HeadShip.Obj.X = -HeadShipWidth;
        HeadShipSpeed = HeadShipSpeed; 
      }
    }
  }
}

void PlayerControl()  {
  if ((digitalRead(RightButton) == 0) && (Player.Obj.X + TankGraphicsWidth < ScreenWidth))
    Player.Obj.X += PlayerXcoordinateMovingValue;
  if ((digitalRead(LeftButton) == 0) && (Player.Obj.X > 0))
    Player.Obj.X -= PlayerXcoordinateMovingValue;
  if ((digitalRead(ShotButton) == 0) && (Missile.Status != ACTIVE))
  {
    Missile.X = Player.Obj.X + (TankGraphicsWidth / 2);
    Missile.Y = PlayerYcoordinateStart;
    Missile.Status = ACTIVE;  
  }
}

void MissileControl() {
  if (Missile.Status == ACTIVE)
  {
    Missile.Y -= MissileSpeed;
    if (Missile.Y + MissileHeight < 0) 
      Missile.Status = DESTROYED;
  }
}

void AlienControl() {
  if ((InvadersMoveCounter--) < 0)
  {
    bool Dropped = false;
    if ((RightMostPos() + AlienXMoveAmount >= ScreenWidth) | (LeftMostPos() + AlienXMoveAmount < 0)) 
    {
      AlienXMoveAmount = -AlienXMoveAmount; 
      Dropped = true;                       
    }
    for (int Across = 0; Across < AlienColumns; Across++)
    {
      for (int Down = 0; Down < 3; Down++)
      {
        if (Alien[Across][Down].Obj.Status == ACTIVE)
        {
          if (Dropped == false)
            Alien[Across][Down].Obj.X += AlienXMoveAmount;
          else
            Alien[Across][Down].Obj.Y += InvadersAttackDrop;
        }
      }
    }
    InvadersMoveCounter = Player.AlienSpeed;
    AnimationFrame = !AnimationFrame;       
  }
  if (random(ChanceofBombDropping) == 1)
    DropBomb();
    MoveBombs();
}

void MoveBombs() {
  for (int i = 0; i < MaximumValueBombs; i++) {
    if (AlienBomb[i].Status == ACTIVE)
      AlienBomb[i].Y += 2;
  }
}

void DropBomb() {
  bool Free = false;
  byte ActiveCols[AlienColumns];
  byte BombIdx = 0;
  while ((Free == false) & (BombIdx < MaximumValueBombs)) {
    if (AlienBomb[BombIdx].Status == DESTROYED)
      Free = true;
    else
      BombIdx++;
  }
  if (Free)  {
    byte Columns = 0;
    byte ActiveColCount = 0;
    signed char Row;
    byte ChosenColumn;

    while (Columns < AlienColumns) {
      Row = 2;
      while (Row >= 0)  {
        if (Alien[Columns][Row].Obj.Status == ACTIVE)
        {
          ActiveCols[ActiveColCount] = Columns;
          ActiveColCount++;
          break;
        }
        --Row;
      }
      Columns++;
    }
    ChosenColumn = random(ActiveColCount); 
    Row = 2;
    while (Row >= 0)  {
      if (Alien[ActiveCols[ChosenColumn]][Row].Obj.Status == ACTIVE)  {
        AlienBomb[BombIdx].Status = ACTIVE;
        AlienBomb[BombIdx].X = Alien[ActiveCols[ChosenColumn]][Row].Obj.X + int(AlienWidth[Row] / 2);
        AlienBomb[BombIdx].X = (AlienBomb[BombIdx].X - 2) + random(0, 4);
        AlienBomb[BombIdx].Y = Alien[ActiveCols[ChosenColumn]][Row].Obj.Y + 4;
        break;
      }
      --Row;
    }
  }
}

void BombCollisions() {
  for (int i = 0; i < MaximumValueBombs; i++)
  {
    if (AlienBomb[i].Status == ACTIVE)
    {
      if (AlienBomb[i].Y > 64)            
        AlienBomb[i].Status = DESTROYED;
      else
      {
        if (Collision(AlienBomb[i], BombWidth, BombHeight, Missile, MissileWidth, MissileHeight))
        {
          AlienBomb[i].Status = EXPLODING;
          Missile.Status = DESTROYED;
        }
        else
        {
          if (Collision(AlienBomb[i], BombWidth, BombHeight, Player.Obj, TankGraphicsWidth, TankGraphicsHeight))
          {
            PlayerHit();
            AlienBomb[i].Status = DESTROYED;
          }
          else
            BombAndBasesCollision(&AlienBomb[i]);
        }
      }
    }
  }
}

void BombAndBasesCollision(SpaceInvadersObjectStruct *Bomb) {
  for (int i = 0; i < NumberOfObstacles; i++)
  {
    if (Collision(*Bomb, BombWidth, BombHeight, Base[i].Obj, BaseWidth, BaseHeight))
    {
      byte X = Bomb->X - Base[i].Obj.X;
      X = X >> 1;
      if (X > 7)  X = 0;

      signed char Bomb_Y = (Bomb->Y + BombHeight) - Base[i].Obj.Y;
      byte Base_Y = 0;

      while ((Base_Y <= Bomb_Y) & (Base_Y < BaseHeight) & (Bomb->Status == ACTIVE))
      {
        byte Idx = (Base_Y * BaseWidthValueInBytes) + (X >> 2); 
        byte TheByte = Base[i].Graphics[Idx]; 
        byte BitIdx = X & 3;
        byte Mask = 0xCD;
        Mask = Mask >> (BitIdx << 1);
        TheByte = TheByte & Mask;
        if (TheByte > 0)
        {
          Mask = ~Mask;
          Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
          if (X > 0) {     
            if (random(BombDamageChance))  
            {
              if (X != 4)   
              {
                Mask = (Mask << 1) | 1;
                Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
              }
              else          
              { 
                Base[i].Graphics[Idx - 1] = Base[i].Graphics[Idx - 1] & 0x7f;
              }
            }
          }
          if (X < 7) {      
            if (random(BombDamageChance))  
            {
              if (X != 3)   
              {
                Mask = (Mask >> 1) | 128;
                Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
              }
              else          
              {
               
                Base[i].Graphics[Idx + 1] = Base[i].Graphics[Idx + 1] & 0xfe;
              }
            }
          }
          if (random(BombPenetrationChanceDOWNway) == false) 
            Bomb->Status = EXPLODING;
        }
        else
          Base_Y++;
      }
    }
  }
}

void MissileAndBasesCollisions() {
  for (int i = 0; i < NumberOfObstacles; i++)
  {
    if (Collision(Missile, MissileWidth, MissileHeight, Base[i].Obj, BaseWidth, BaseHeight))
    {
      byte X = Missile.X - Base[i].Obj.X;
      X = X >> 1;
      if (X > 7)  X = 0;
      signed char Missile_Y = Missile.Y - Base[i].Obj.Y;
      signed char Base_Y = BaseHeight - 1;
      while ((Base_Y >= Missile_Y) & (Base_Y >= 0) & (Missile.Status == ACTIVE))
      {
        byte Idx = (Base_Y * BaseWidthValueInBytes) + (X >> 2); 
        byte TheByte = Base[i].Graphics[Idx]; 
        byte BitIdx = X & 3;  
        byte Mask = 0xCD;
        Mask = Mask >> (BitIdx << 1);
        TheByte = TheByte & Mask;
        if (TheByte > 0)
        {
          Mask = ~Mask;
          Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
          if (X > 0) {             
            if (random(BombDamageChance))  
            {
              if (X != 4)              
              {
                Mask = (Mask << 1) | 1;
                Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
              }
              else                        
              {
                //Base[i].Graphics[Idx-1]=Base[i].Graphics[Idx-1] & B11111110;
                Base[i].Graphics[Idx - 1] = Base[i].Graphics[Idx - 1] & 0x7f;
              }
            }
          }

          if (X < 7) {                   
            if (random(BombDamageChance))  
            {
              if (X != 3)                
              {
                Mask = (Mask >> 1) | 128;
                Base[i].Graphics[Idx] = Base[i].Graphics[Idx] & Mask;
              }
              else                        
              {
                //Base[i].Graphics[Idx+1]=Base[i].Graphics[Idx+1] & B01111111;
                Base[i].Graphics[Idx + 1] = Base[i].Graphics[Idx + 1] & 0xfe;
              }
            }
          }
          if (random(BombPenetrationChanceDOWNway) == false) 
            Missile.Status = EXPLODING;
        }
        else
          Base_Y--;
      }
    }
  }
}

void PlayerHit() {
  Player.Obj.Status = EXPLODING;
  Player.ExplosionGraphicsCounter = PlayerExplosionTime;
  Missile.Status = DESTROYED;
}

void CheckCollisions() {
  MissileAndAlienCollisions();
  MotherShipCollisions();
  MissileAndBasesCollisions();
  BombCollisions();
  AlienAndBaseCollisions();
}

char GetAlienBaseCollisionMask(int AlienX, int AlienWidth, int BaseX) {
  signed int DamageWidth;
  byte LeftMask, RightMask, FullMask;
  //LeftMask=B11111111;     
  //RightMask=B11111111;    
  LeftMask = 0xff;   
  RightMask = 0xff;  
  if (AlienX > BaseX)
  {
    DamageWidth = AlienX - BaseX;
    LeftMask >>= DamageWidth;
  }
  if (AlienX + AlienWidth < BaseX + (BaseWidth / 2))
  {
    DamageWidth = (BaseX + (BaseWidth / 2)) - (AlienX + AlienWidth);
    RightMask <<= DamageWidth;
  }
  return ~(LeftMask & RightMask);
}

void DestroyBase(SpaceInvadersObjectStruct *Alien, BaseDefenceStruct *Base, char Mask, int BaseByteOffset) {
  signed char Y;
  Y = (Alien->Y + AlienHeightonOLED) - Base->Obj.Y;
  if (Y > BaseHeight - 1) Y = BaseHeight - 1;
  for (; Y >= 0; Y--) {
    Base->Graphics[(Y * 2) + BaseByteOffset] = Base->Graphics[(Y * 2) + BaseByteOffset] & Mask;
  }
}

void AlienAndBaseCollisions() {
  byte Mask;
  for (int row = 2; row >= 0; row--)
  {
    for (int column = 0; column < AlienColumns; column++)
    {
      if (Alien[column][row].Obj.Status == ACTIVE)
      {
        for (int BaseIdx = 0; BaseIdx < NumberOfObstacles; BaseIdx++)
        {
          if (Collision(Alien[column][row].Obj, AlienWidth[row], AlienHeightonOLED, Base[BaseIdx].Obj, BaseWidth, BaseHeight))
          {
            Mask = GetAlienBaseCollisionMask(Alien[column][row].Obj.X, AlienWidth[row], Base[BaseIdx].Obj.X);
            DestroyBase(&Alien[column][row].Obj, &Base[BaseIdx], Mask, 0);
            Mask = GetAlienBaseCollisionMask(Alien[column][row].Obj.X, AlienWidth[row], Base[BaseIdx].Obj.X + (BaseWidth / 2));
            DestroyBase(&Alien[column][row].Obj, &Base[BaseIdx], Mask, 1);
          }
        }
      }
    }
  }
}

void MotherShipCollisions() {
  if ((Missile.Status == ACTIVE) & (HeadShip.Obj.Status == ACTIVE))
  {
    if (Collision(Missile, MissileWidth, MissileHeight, HeadShip.Obj, HeadShipWidth, HeadShipHeight))
    {
      HeadShip.Obj.Status = EXPLODING;
      HeadShip.ExplosionGraphicsCounter = ExplosionTime;
      Missile.Status = DESTROYED;
      HeadShipBonus = random(4); 
      switch (HeadShipBonus)
      {
        case 0: HeadShipBonus = 50; break;
        case 1: HeadShipBonus = 100; break;
        case 2: HeadShipBonus = 150; break;
        case 3: HeadShipBonus = 300; break;
      }
      Player.Score += HeadShipBonus + HeadShipType * 100;
      HeadShipBonusOnXPosition = HeadShip.Obj.X;
      if (HeadShipBonusOnXPosition > 100)               
        HeadShipBonusOnXPosition = 100;
      if (HeadShipBonusOnXPosition < 0)                  
        HeadShipBonusOnXPosition = 0;
      HeadShipBonusCounter = HeadShipBunusDisplay;
    }
  }
}

void MissileAndAlienCollisions() {
  for (int across = 0; across < AlienColumns; across++)
  {
    for (int down = 0; down < AlienRows; down++)
    {
      if (Alien[across][down].Obj.Status == ACTIVE)
      {
        if (Missile.Status == ACTIVE)
        {
          if (Collision(Missile, MissileWidth, MissileHeight, Alien[across][down].Obj, AlienWidth[down], InvadersHeightonScreen))
          {
            Alien[across][down].Obj.Status = EXPLODING;
            Missile.Status = DESTROYED;
            Player.Score += GetScoreForAlien(down);
            Player.AliensDestroyed++;
            Player.AlienSpeed = ((1 - (Player.AliensDestroyed / (float)TotalAmountOfAliens)) * InvadersSpeedValue);
            if (Player.AliensDestroyed == TotalAmountOfAliens - 2)
              if (AlienXMoveAmount > 0)
                AlienXMoveAmount = AlienXcoordinateMovingValue * 2;
              else
                AlienXMoveAmount = -(AlienXcoordinateMovingValue * 2);
            if (Player.AliensDestroyed == TotalAmountOfAliens - 1)
              if (AlienXMoveAmount > 0)
                AlienXMoveAmount = AlienXcoordinateMovingValue * 4;
              else
                AlienXMoveAmount = -(AlienXcoordinateMovingValue * 4);

            if (Player.AliensDestroyed == TotalAmountOfAliens) {
              oled.setCursor(0, 0);
              oled.print("NL");
              NextLevel(&Player);
            }

          }
        }
        if (Alien[across][down].Obj.Status == ACTIVE)   
        {
          if (Collision(Player.Obj, TankGraphicsWidth, TankGraphicsHeight, Alien[across][down].Obj, AlienWidth[down], AlienHeightonOLED))
            PlayerHit();
          else
          {
            if (Alien[across][down].Obj.Y + 8 > ScreenHeight)
              PlayerHit();
          }
        }
      }
    }
  }
}

bool Collision(SpaceInvadersObjectStruct Obj1, byte Width1, byte Height1, SpaceInvadersObjectStruct Obj2, byte Width2, byte Height2) {
  return ((Obj1.X + Width1 > Obj2.X) & (Obj1.X < Obj2.X + Width2) & (Obj1.Y + Height1 > Obj2.Y) & (Obj1.Y < Obj2.Y + Height2));
}

int RightMostPos() {
  int Across = AlienColumns - 1;
  int Down;
  int Largest = 0;
  int RightPos;
  while (Across >= 0) {
    Down = 0;
    while (Down < AlienRows) {
      if (Alien[Across][Down].Obj.Status == ACTIVE)
      {
        RightPos = Alien[Across][Down].Obj.X + AlienWidth[Down];
        if (RightPos > Largest)
          Largest = RightPos;
      }
      Down++;
    }
    if (Largest > 0) 
      return Largest;
    Across--;
  }
  return 0;      
}

int LeftMostPos() {
  int Across = 0;
  int Down;
  int Smallest = ScreenWidth * 2;
  while (Across < AlienColumns) {
    Down = 0;
    while (Down < 3) {
      if (Alien[Across][Down].Obj.Status == ACTIVE)
        if (Alien[Across][Down].Obj.X < Smallest)
          Smallest = Alien[Across][Down].Obj.X;
      Down++;
    }
    if (Smallest < ScreenWidth * 2) 
      return Smallest;
    Across++;
  }
  return 0;  
}

void UpdateDisplay() {
  int i;
  byte RowHeight;
//display.clearDisplay();
  oled.clearBuffer();
  RowHeight = FONT_Ascent;
  if (HeadShipBonusCounter > 0)
  {
    oled.setCursor(HeadShipBonusOnXPosition, RowHeight);
    oled.print(HeadShipBonus);
    HeadShipBonusCounter--;
  } else {
    oled.setCursor(0, RowHeight);
    oled.print(Player.Score);
    oled.setCursor(ScreenWidth - 7, RowHeight);
    oled.print(Player.Lives);
  }
  for (i = 0; i < MaximumValueBombs; i++)  {
    if (AlienBomb[i].Status == ACTIVE)
      oled.drawXBMP(AlienBomb[i].X, AlienBomb[i].Y, 2, 4, AlienAttackGraphics);
    else  {
      if (AlienBomb[i].Status == EXPLODING)
        oled.drawXBMP(AlienBomb[i].X - 4, AlienBomb[i].Y, 4, 8, ExplosionGraphics);
      AlienBomb[i].Status = DESTROYED;
    }
  }
  for (int across = 0; across < AlienColumns; across++) {
    for (int down = 0; down < AlienRows; down++) {
      if (Alien[across][down].Obj.Status == ACTIVE) {
        int j;
        if (AnimationFrame) {
          j = 0;
        } else {
          j = 1;
        }
        switch (down)  {
          case 0:
           
            oled.drawXBMP(Alien[across][down].Obj.X, Alien[across][down].Obj.Y, AlienWidth[down], InvadersHeightonScreen, TopInvaderGraphics[j]);
            break;
          case 1:
            oled.drawXBMP(Alien[across][down].Obj.X, Alien[across][down].Obj.Y, AlienWidth[down], InvadersHeightonScreen, MiddleInvaderGraphics[j]);
            break;
          default:
            oled.drawXBMP(Alien[across][down].Obj.X, Alien[across][down].Obj.Y, AlienWidth[down], InvadersHeightonScreen, BottomInvaderGraphics[j]);
        }  
      } else {
        if (Alien[across][down].Obj.Status == EXPLODING) {
          Alien[across][down].ExplosionGraphicsCounter--;
          if (Alien[across][down].ExplosionGraphicsCounter > 0)  {
            oled.drawXBMP(Alien[across][down].Obj.X, Alien[across][down].Obj.Y, 13, 8, ExplosionGraphics);
          } else
            Alien[across][down].Obj.Status = DESTROYED;
        }
      }
    }
  }
  if (Player.Obj.Status == ACTIVE)
    oled.drawXBMP(Player.Obj.X, Player.Obj.Y, TankGraphicsWidth, TankGraphicsHeight, TankAttackGraphics);
  else {
    if (Player.Obj.Status == EXPLODING)  {
      for (i = 0; i < TankGraphicsWidth; i += 2)  {
        oled.drawXBMP(Player.Obj.X + i, Player.Obj.Y, random(4) + 2, 8, ExplosionGraphics);
      }
      Player.ExplosionGraphicsCounter--;
      if (Player.ExplosionGraphicsCounter == 0)  {
        Player.Obj.Status = DESTROYED;
        delay(500);                   
        LoseLife();
      }
    }
  }
  if (Missile.Status == ACTIVE)
    oled.drawXBMP(Missile.X, Missile.Y, MissileWidth, MissileHeight, MissileAttackGraphics);
  if (HeadShip.Obj.Status == ACTIVE) {
    oled.drawXBMP(HeadShip.Obj.X, HeadShip.Obj.Y, HeadShipWidth, HeadShipHeight, HeadShipGraphics[HeadShipType]);
  } else  {
    if (HeadShip.Obj.Status == EXPLODING)
    {
      for (i = 0; i < HeadShipWidth; i += 2)  {
        oled.drawXBMP(HeadShip.Obj.X + i, HeadShip.Obj.Y, random(4) + 2, HeadShipHeight, ExplosionGraphics);
      }
      HeadShip.ExplosionGraphicsCounter--;
      if (HeadShip.ExplosionGraphicsCounter == 0)  {
        HeadShip.Obj.Status = DESTROYED;
      }
    }
  }

  for (i = 0; i < NumberOfObstacles; i++)  {
    if (Base[i].Obj.Status == ACTIVE)
      oled.drawXBM(Base[i].Obj.X, Base[i].Obj.Y, BaseWidth, BaseHeight, Base[i].Graphics);
  }
  oled.sendBuffer();
}

void LoseLife() {
  Player.Lives--;
  if (Player.Lives > 0)  {
    DisplayPlayerAndLives(&Player);
    for (int i = 0; i < MaximumValueBombs; i++)  {
      AlienBomb[i].Status = DESTROYED;
      AlienBomb[i].Y = 0;
    }
    Player.Obj.Status = ACTIVE;
    Player.Obj.X = 0;
  } else  {
    GameOver();
  }
}

void GameOver() {
  byte RowHeight;
  byte ColumnPos;
  GameInPlay = false;
  oled.clearBuffer();
  if (Player.Score > HighestScore) {
    RowHeight = FONT_Ascent;
  } else {
    RowHeight = FONT_Ascent + (ScreenHeight - 4 * (FONT_Ascent + FONT_Descent + 1)) / 2;
  }
  CenterText("Player 1", RowHeight);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Game Over", RowHeight);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  ColumnPos = oled.getStrWidth("8");
  if (HighestScore < 10) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Score ") - ColumnPos) / 2.0);
  } else if (HighestScore < 100) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Score ") - ColumnPos * 2) / 2.0);
  } else if (HighestScore < 1000) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Score ") - ColumnPos * 3) / 2.0);
  } else if (HighestScore < 10000) {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Score ") - ColumnPos * 4) / 2.0);
  } else {
    ColumnPos = int((ScreenWidth - oled.getStrWidth("Score ") - ColumnPos * 5) / 2.0);
    //65535 maximum
  }
  oled.setCursor(ColumnPos, RowHeight);
  oled.print("Score ");
  oled.print(Player.Score);
  if (Player.Score > HighestScore) {
    RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
    CenterText("NEW HIGH SCORE!!!", RowHeight);
    RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
    CenterText("**CONGRATULATIONS**", RowHeight);
  }
  oled.sendBuffer();
  if (Player.Score > HighestScore) {
    HighestScore = Player.Score;
    EEPROM.put(0, HighestScore);
  }
  delay(3000);
}

void DisplayPlayerAndLives(UserStruct *Player) {
  byte RowHeight;
  oled.clearBuffer();
  RowHeight = FONT_Ascent + (ScreenHeight - 4 * (FONT_Ascent + FONT_Descent + 1)) / 2;
  CenterText("Player 1", RowHeight);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Score ", RowHeight);
  oled.print(Player->Score);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Lives ", RowHeight);
  oled.print(Player->Lives);
  RowHeight = RowHeight + FONT_Ascent + FONT_Descent + 1;
  CenterText("Level ", RowHeight);
  oled.print(Player->Level);
  oled.sendBuffer();
  delay(2000);
  Player->Obj.X = PlayerXcoordinateStart;
}

void CenterText(const char *Text, byte RowValue) {
  oled.setCursor(int((ScreenWidth - oled.getStrWidth(Text)) / 2.0), RowValue);
  oled.print(Text);
}

void InitializePlayer() {
  Player.Obj.Y = PlayerYcoordinateStart;
  Player.Obj.X = PlayerXcoordinateStart;
  Player.Obj.Status = ACTIVE;
  Player.Lives = LIVES;
  Player.Level = 0;
  Missile.Status = DESTROYED;
  Player.Score = 0;
}

void NextLevel(UserStruct *Player) {


  int YStart;
  for (int i = 0; i < MaximumValueBombs; i++)
    AlienBomb[i].Status = DESTROYED;
  AnimationFrame = false;
  Player->Level++;
  YStart = ((Player->Level - 1) % LevelToResetHeight) * AmountToDropPerLevel;
  InitAliens(YStart);
  AlienXMoveAmount = AlienXcoordinateMovingValue;
  Player->AlienSpeed = InvadersSpeedValue;
  Player->AliensDestroyed = 0;
  HeadShip.Obj.X = -HeadShipWidth;
  HeadShip.Obj.Status = DESTROYED;
  Missile.Status = DESTROYED;
  randomSeed(100);
  InitBases();
  DisplayPlayerAndLives(Player);
}

void InitBases() {
  byte TheByte;
  int Spacing = (ScreenWidth - (NumberOfObstacles * BaseWidth)) / NumberOfObstacles;
  for (int i = 0; i < NumberOfObstacles; i++)
  {
    for (int DataIdx = 0; DataIdx < BaseHeight * BaseWidthValueInBytes; DataIdx++)
    {
      TheByte = pgm_read_byte(BaseDefenceGraphics + DataIdx);
      Base[i].Graphics[DataIdx] = TheByte;
    }
    Base[i].Obj.X = (i * Spacing) + (i * BaseWidth) + (Spacing / 2);
    Base[i].Obj.Y = BaseOnY;
    Base[i].Obj.Status = ACTIVE;
  }
}

void NewGame() {
  InitializePlayer();
  NextLevel(&Player);
}

void InitAliens(int YStart) {
  for (int across = 0; across < AlienColumns; across++)  {
    for (int down = 0; down < 3; down++)  {
      Alien[across][down].Obj.X = XcoordinateStart + (across * (AlienWidthLargestValue + SpaceBetweenColumnsAliens)) - (AlienWidth[down] / 2);
      Alien[across][down].Obj.Y = YStart + (down * SpaceBetweenRows);
      Alien[across][down].Obj.Status = ACTIVE;
      Alien[across][down].ExplosionGraphicsCounter = ExplosionTime;
    }
  }
  HeadShip.Obj.Y = 0;
  HeadShip.Obj.X = -HeadShipWidth;
  HeadShip.Obj.Status = DESTROYED;
}
