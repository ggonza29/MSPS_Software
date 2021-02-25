#ifndef t1 //the ifndef and define is used for that if this header is added multiple times to ignore this header to not receive an error
#define t1

//this block of code is to verify that we add the arduino library to the header files.
//since header files do not include built in arduino function.
#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
//to add arduino functions to the header


class TestLibrary {//good formating will create the class name with the same name as the .c file name
  public://the main file has access to these values/functions
    //constructor
    TestLibrary(bool displayMsg = false);

    //Methods
    long getRandomNumber();

  private://only this header file has access to these values/functions
};









#endif
