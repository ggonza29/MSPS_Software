#ifndef t1
#define t1

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else 
  #inlcude "WProgram.h"
#endif

class screen {
  public :
  screen(bool displayMsg = false);
  //void screen();
  void SETUP();
  void displayLCD();

  private:
  
  
};
#endif
