/*
 * main.cpp
 * DSP Program
 *
 * Tiffany Liu
 * 5 February 2026
 * 
 * Description: Implementation of main program
*/

//Libraries
#include <cstdio>
#include <cstdlib>
#include <alsa/asoundlib.h>
#include <limits>
#include <string>
#include <poll.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>

//#include "../include/menu.h"
#include "../include/callback.h"
#include "../include/types.h"
#include "../include/audio.h"
#include "../include/Engine.h"

using namespace std;
const char* INPUT_DEVICE_NAME = "hw:2,0";
const char* OUTPUT_DEVICE_NAME = "plughw:0,0";


// main function
int main(){
    
    Engine engine(INPUT_DEVICE_NAME, OUTPUT_DEVICE_NAME); 
    engine.start();
    
    while(1) {}
    
}





