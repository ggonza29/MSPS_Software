#include "TestLibrary.h"

TestLibrary::TestLibrary(bool displayMsg){
  //anything that you need when instantiating your object goes here
}
long TestLibrary::getRandomNumber(){
  unsigned long specialNumber = random(5, 1000);

  specialNumber *= PI;
  
  specialNumber -= 5;

  return specialNumber;
  
};
