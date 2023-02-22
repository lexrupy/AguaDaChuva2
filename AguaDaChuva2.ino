//Programa: Teste LCD 16x2 com Keypad
//Autor: Arduino e Cia
  
#include <LiquidCrystal.h>  
#include <RotaryEncoder.h>
#define BACKLIGHT_PIN 10
#define ENCODER_SW    A1


byte lvlEmpty[8] = {
	0b01110,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b11111
};

byte lvlLow[8] = {
	0b01110,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b11111,
	0b11111
};

byte lvlMid[8] = {
	0b01110,
	0b10001,
	0b10001,
	0b10001,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};


byte lvlFull[8] = {
	0b01110,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};

byte arrow[8] = {
	0b00000,
	0b00100,
	0b00110,
	0b11111,
	0b00110,
	0b00100,
	0b00000,
	0b00000
};


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  
RotaryEncoder encoder(A4,A5);

int menuSize = 5;
int subMenuSize = 4;
char* menus[] = {"Modo Trabalho   ","Motor Cisterna  ","Solenoide CASAN ","Backlight       ", "Sair            "};
char* onoff[] = {"Desligado       ","Ligado          ","Salvar          ","Cancelar        "};
char* modot[] = {"Automatico      ","Manual          ","Salvar          ","Cancelar        "};
char* mbtmr[] = {"Intensidade     ","Tempo idle      ","Voltar          "};
char* midle[] = {"1min","2min","3min","4min","5min"};


int menuPos = 0;
int subMenuPos = 0;
int lastMenuPos = 0;
int lastSubMenuPos = 0;
int lastMenuDrw = -1;
int lastStatusDrw = -1;
int lastSubMenuDrw = -1;
int encoderPos = 0;
bool inMenu = false;
bool inSubMenu = false;

bool inAjusteIntensidade = false;
bool inAjusteIdle = false;

// Nivel 0 = Vazio 1 = Baixo 2 = Médio 3 = Cheio
int nivelCaixa = 2;
int nivelCisterna = 3;

// Modo de Trabalho 0 = Automatico 1 = Manual
int modoTrabalho = 0;
int modoTrabalhoMenu = 0;


// Estado Motor 0 = Desligado 1 = Ligado
int estadoMotor = 0;
int estadoMotorMenu = 0;

unsigned long motorTimer = 0;

// Estado Solenoide 0 = Desligado 1 = Ligado
int estadoSolenoide = 0;
int estadoSolenoideMenu = 0;

unsigned long solenoideTimer = 0;


int lastEncoderSWState = HIGH;
unsigned long lastencoderSWdt = 0;
unsigned long encoderSWdebounceDelay = 50;
unsigned long currentMillis = 0;

int backlightState = 1; // 1 = Ligado, 0 = Desligado
unsigned long backlightIdleTime = 300000; // Default 5 minutos
int backlightIntensidade = 50; // 50%
unsigned long lastIterationTime = 0;

int menuIdleTime = 25000; // Sai do Menu apos 25 segundos;

  
void setup()   
{
  pinMode(BACKLIGHT_PIN, OUTPUT);
  lcdBacklitOn();
  lcd.begin(16, 2);

  lcd.createChar(0, lvlEmpty);
  lcd.createChar(1, lvlLow);
  lcd.createChar(2, lvlMid);
  lcd.createChar(3, lvlFull);
  lcd.createChar(4, arrow);

  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("  Cisterna 1.0  ");
  lcd.setCursor(0,1); 
  lcd.print("Inicializando...");
  delay(3000); //wait 3 sec
  lcd.clear();
  lcd.setCursor(0,0); 
  
  
  pinMode(ENCODER_SW, INPUT_PULLUP);
  PCICR |= (1 << PCIE1);
  //PCMSK1 |= (1 << PCINT10) | (1 << PCINT11); 
  PCMSK1 |= (1 << PCINT12) | (1 << PCINT13); 
}  

ISR(PCINT1_vect) {
  encoder.tick(); // just call tick() to check the state.
}
  
void loop()  
{ 
  currentMillis = millis();
  int newPos = encoder.getPosition();

  if (newPos > encoderPos) {
    if (inMenu) {
      if (inSubMenu) {
        if (inAjusteIntensidade) {
          if (backlightIntensidade < 100) {
            backlightIntensidade += 5;
            lcdBacklitOn();
            lastSubMenuDrw = -1;
          }
        } else if (inAjusteIdle) {
          if (backlightIdleTime < 300000) {
            backlightIdleTime += 60000;
            lastSubMenuDrw = -1;
          }
        } else {
          if (menuPos == 3) {subMenuSize = 3;} else {subMenuSize = 4;}
          lastSubMenuPos = subMenuPos;
          if (subMenuPos == subMenuSize-1) {
            subMenuPos = 0;
          } else { 
            subMenuPos++; 
          }
        }
      } else {
        lastMenuPos = menuPos;
        if (menuPos == menuSize-1) {
          menuPos = 0;
        } else { 
          menuPos++; 
        }
      }
    }
    
  } else if (newPos < encoderPos) {
    if (inMenu) {
      if (inSubMenu) {
        if (inAjusteIntensidade) {
          if (backlightIntensidade > 0) {
            backlightIntensidade -= 5; 
            lcdBacklitOn();
            lastSubMenuDrw = -1;
          }
        } else if (inAjusteIdle) {
          if (backlightIdleTime > 60000) {
            backlightIdleTime -= 60000;
            lastSubMenuDrw = -1;
          }
        } else {
          if (menuPos == 3) {subMenuSize = 3;} else {subMenuSize = 4;}
          lastSubMenuPos = subMenuPos;
          if (subMenuPos == 0) {
            subMenuPos = subMenuSize-1;
          } else { 
            subMenuPos--; 
          }
        }
      } else {
        lastMenuPos = menuPos;
        if (menuPos == 0) {
          menuPos = menuSize-1;
        } else {
          menuPos--;
        }
      }
    }
    
  }
  if (newPos != encoderPos) {
    lastIterationTime = currentMillis;
    encoderPos = newPos;
  }  
  

  checkEncoderSW();
  drawLCD();  
  checkBacklightAndMenu();
}

void checkBacklightAndMenu() {
  // Backlight
  if (currentMillis - lastIterationTime > backlightIdleTime) {
    lcdBacklitOff();
  } else {
    lcdBacklitOn();
  }
  // Menu
  if (currentMillis - lastIterationTime > menuIdleTime) {
    if (inMenu) {
      inMenu = false;
      inSubMenu = false;
      inAjusteIntensidade = false;
      inAjusteIdle = false;
      menuPos = 0;
      subMenuPos = 0;
      lastMenuPos = 0;
      lastSubMenuPos = 0;
      lastMenuDrw = -1;
      lastStatusDrw = -1;
      lastSubMenuDrw = -1;
    }
    
  }
}

void checkEncoderSW() {

  int encoderSWState = digitalRead(ENCODER_SW);

  if ( (currentMillis - lastencoderSWdt) > encoderSWdebounceDelay) {
    if ((encoderSWState != lastEncoderSWState) && (encoderSWState == LOW)) {
      lastIterationTime = currentMillis;
      if (backlightState == 0) {
        lcdBacklitOn();
      } else {
        if (!inMenu) {
          menuPos = 0;
          inMenu = true;
        } else {
          switch (menuPos) {
            case 0: // Modo Trabalho
              if (!inSubMenu) {
                  subMenuPos = 0;
                  inSubMenu = true;
                  modoTrabalhoMenu = modoTrabalho;
              } else {
                switch(subMenuPos) {
                  case 0:
                    modoTrabalhoMenu = 0;
                    break;
                  case 1:
                    modoTrabalhoMenu = 1;
                    break;
                  case 2: // Salvar
                    modoTrabalho = modoTrabalhoMenu;
                    inSubMenu = false;
                    break;
                  case 3: // Sair
                    inSubMenu = false;
                    break;
                }
                lastSubMenuDrw = -1;
                lastStatusDrw = -1;
              }          
              break;
            case 1: // Motor Cisterna
              if (!inSubMenu) {
                  subMenuPos = 0;
                  inSubMenu = true;
                  estadoMotorMenu = estadoMotor;
              } else {
                switch(subMenuPos) {
                  case 0:
                    if (modoTrabalho == 1) {
                      estadoMotorMenu = 0;
                    }
                    break;
                  case 1:
                    if (modoTrabalho == 1) {
                      estadoMotorMenu = 1;
                    }
                    break;
                  case 2: // Salvar
                    if (estadoMotor != estadoMotorMenu) {
                      if (estadoMotorMenu == 1) {
                        ligaMotor();
                      } else {
                        desligaMotor();
                      }
                    }
                    inSubMenu = false;
                    break;
                  case 3: // Sair
                    inSubMenu = false;
                    break;
                }
                lastSubMenuDrw = -1;
                lastStatusDrw = -1;
              }          
              break;
            case 2: // Solenoide Casan
              if (!inSubMenu) {
                  subMenuPos = 0;
                  inSubMenu = true;
                  estadoSolenoideMenu = estadoSolenoide;
              } else {
                switch(subMenuPos) {
                  case 0:
                    if (modoTrabalho == 1) {
                      estadoSolenoideMenu = 0;
                    }
                    break;
                  case 1:
                    if (modoTrabalho == 1) {
                      estadoSolenoideMenu = 1;
                    }
                    break;
                  case 2: // Salvar
                    if (estadoSolenoide != estadoSolenoideMenu) {
                      if (estadoSolenoideMenu == 1) {
                        ligaSolenoide();
                      } else {
                        desligaSolenoide();
                      }
                    }
                    inSubMenu = false;
                    break;
                  case 3: // Sair
                    inSubMenu = false;
                    break;
                }
                lastSubMenuDrw = -1;
                lastStatusDrw = -1;
              }          
              break;
            case 3: // LED Backlight
              if (!inSubMenu) {
                  subMenuPos = 0;
                  inSubMenu = true;
              } else {
                switch(subMenuPos) {
                  case 0:
                    inAjusteIntensidade = !inAjusteIntensidade;
                    break;
                  case 1:
                    inAjusteIdle = !inAjusteIdle;
                    break;
                  case 2: // Sair
                    inSubMenu = false;
                    break;
                }
                lastSubMenuDrw = -1;
                lastStatusDrw = -1;
              }          
              break;
            case 4:
              lastMenuDrw = -1;
              lastStatusDrw = -1;
              inMenu = false;
              break;
          }
        }
      }
      
    }
    lastEncoderSWState = encoderSWState;
    lastencoderSWdt = currentMillis;
  }
}

void drawLCD(){
  if (inMenu) {
      if (lastMenuDrw != menuPos) {
        lcd.setCursor(0,0);
        if (!inSubMenu) {
          lcd.print(" Menu           ");
          lcd.setCursor(0,1);
          lcd.write(byte(4));
          lcd.print(menus[menuPos]);
        } else {
          if (lastSubMenuDrw != subMenuPos) {
            lcd.print(menus[menuPos]);
            lcd.setCursor(0,1);
            lcd.write(byte(4));
            // Menu de Modo de Trabalho Diferente
            switch(menuPos) {
              case 0: // Modo De Trabalho
                if (subMenuPos <= 1) {
                  if (subMenuPos == modoTrabalhoMenu) {
                    lcd.print("[X]");
                  } else {
                    lcd.print("[ ]");
                  }
                }
                break;
              case 1: // Motor Cisterna
                if (subMenuPos <= 1) {
                  if (subMenuPos == estadoMotorMenu) {
                    lcd.print("[X]");
                  } else {
                    lcd.print("[ ]");
                  }
                }
                break;
              case 2: // Solenoide CASAN
                if (subMenuPos <= 1) {
                  if (subMenuPos == estadoSolenoideMenu) {
                    lcd.print("[X]");
                  } else {
                    lcd.print("[ ]");
                  }
                }
                break;
              case 3: // Backlight LED
                if (subMenuPos == 0) {

                  lcd.setCursor(0, 1);
                  char buffer[3];
                  sprintf(buffer, "%03d", backlightIntensidade);
                  lcd.print(mbtmr[subMenuPos]);
                  lcd.setCursor(11,1);
                  if (inAjusteIntensidade) {
                    lcd.write(byte(4));
                  } else {
                    lcd.print(" ");
                  }
                  lcd.print(buffer);
                  lcd.print("%");
                } else if (subMenuPos == 1) {
                  lcd.setCursor(0, 1);
                  //char buffer[2];
                  int val = backlightIdleTime / 60000;
                  //sprintf(buffer, "%02d", val);   
                  lcd.print(mbtmr[subMenuPos]);      
                  lcd.setCursor(11, 1);           
                  if (inAjusteIdle) {
                    lcd.write(byte(4));
                  } else {
                    lcd.print(" ");
                  }
                  lcd.print(val);
                  lcd.print("min");
                }
            }

            if (menuPos == 0) {
              lcd.print(modot[subMenuPos]);
            } else if (menuPos == 3) {
              // So printa o voltar
              if (subMenuPos > 1) {
                lcd.print(mbtmr[subMenuPos]);  
              } 
            } else {
              lcd.print(onoff[subMenuPos]);
            }
            lastSubMenuDrw = subMenuPos;
          }
        }
      }
      
  } else {
      int currentStatusDrw = (nivelCaixa*2 + nivelCisterna*7);
      if (currentStatusDrw != lastStatusDrw) {
        lcd.setCursor(0,0);
        lcd.print("Cx:  Ct:       ");
        lcd.setCursor(3, 0);
        switch (nivelCaixa) {
          case 0:
            lcd.write(byte(0));
            break;
          case 1:
            lcd.write(byte(1));
            break;
          case 2:
            lcd.write(byte(2));
            break;
          case 3:
            lcd.write(byte(3));
            break;
        }
        lcd.setCursor(8, 0);
        switch (nivelCisterna) {
          case 0:
            lcd.write(byte(0));
            break;
          case 1:
            lcd.write(byte(1));
            break;
          case 2:
            lcd.write(byte(2));
            break;
          case 3:
            lcd.write(byte(3));
            break;
        }
        lcd.setCursor(10,0);
        if (modoTrabalho == 0) {
          lcd.print("  AUTO");
        } else {
          lcd.print("MANUAL");
        }
        lcd.setCursor(0,1);
        if (estadoMotor == 1) {
          lcd.print("Motor      ");
        } else if (estadoSolenoide == 1) {
          lcd.print("Solenoide  ");
        } else {
          lcd.print("Sem Fluxo  --:--");
        }
        lastStatusDrw = currentStatusDrw;
      }
  }
}

void lcdBacklitOff() {
  digitalWrite(BACKLIGHT_PIN, LOW);
  backlightState = 0;
}

void lcdBacklitOn() {
  //digitalWrite(BACKLIGHT_PIN, HIGH);
  int intensidade = map(backlightIntensidade, 0,100,0,255);
  analogWrite(BACKLIGHT_PIN, intensidade);
  backlightState = 1;
}

void ligaMotor() {
  desligaSolenoide();
  estadoMotor = 1;
}

void desligaMotor() {
  estadoMotor = 0;
}

void ligaSolenoide() {
  desligaMotor();
  estadoSolenoide = 1;
}

void desligaSolenoide() {
  estadoSolenoide = 0;
}

// argument is time in milliseconds
void printTime(unsigned long t_milli)
{
    char buffer[20];
    int days, hours, mins, secs;
    int fractime;
    unsigned long inttime;

    inttime  = t_milli / 1000;
    fractime = t_milli % 1000;
    // inttime is the total number of number of seconds
    // fractimeis the number of thousandths of a second

    // number of days is total number of seconds divided by 24 divided by 3600
    days     = inttime / (24*3600);
    inttime  = inttime % (24*3600);

    // Now, inttime is the remainder after subtracting the number of seconds
    // in the number of days
    hours    = inttime / 3600;
    inttime  = inttime % 3600;

    // Now, inttime is the remainder after subtracting the number of seconds
    // in the number of days and hours
    mins     = inttime / 60;
    inttime  = inttime % 60;

    // Now inttime is the number of seconds left after subtracting the number
    // in the number of days, hours and minutes. In other words, it is the
    // number of seconds.
    secs = inttime;

    // Don't bother to print days
    sprintf(buffer, "%02d:%02d", mins, secs);
    lcd.print(buffer);
}

