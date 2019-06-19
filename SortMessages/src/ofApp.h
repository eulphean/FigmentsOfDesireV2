#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
  
    // Files to store the messages in. 
    ofFile amay;
    ofFile azra;
  
    string lastSender; 
};
