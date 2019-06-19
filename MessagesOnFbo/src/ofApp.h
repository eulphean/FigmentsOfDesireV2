#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
  
		void keyPressed(int key);
    void readFile(string fileName, std::vector<string> &array);
    void populateFbo(ofFbo &fbo, std::vector<string> array, ofTrueTypeFont font);
  
    // Amay's messages.
    std::vector<string> amay;
    ofTrueTypeFont amayFont;
    ofFbo amayTex;
  
    // Azra's messages.
    std::vector<string> azra;
    ofTrueTypeFont azraFont;
    ofFbo azraTex;
  
    bool curTex = true; 
};
