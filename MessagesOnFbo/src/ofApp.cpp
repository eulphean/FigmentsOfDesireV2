#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Amay's messages.
  amayFont.load("caviar.ttf", 30);
  readFile("amay.txt", amay);
  populateFbo(amayTex, amay, amayFont);

  // Azra's messages.
  azraFont.load("marvelos.otf", 25);
  readFile("azra.txt", azra);
  
  populateFbo(azraTex, azra, azraFont);
  
  ofEnableSmoothing();
  ofEnableAntiAliasing();
  
  
  ofTrueTypeFont::setGlobalDpi(50);
}

//--------------------------------------------------------------
void ofApp::update(){
  
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Draw Amay's FBO or Azra's FBO based on key press input.
  
  if (curTex) {
    amayTex.draw(0, 0);
  } else {
    azraTex.draw(0, 0);
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == ' ') {
    curTex = !curTex;
  }
}

void ofApp::populateFbo(ofFbo &fbo, std::vector<string> msgs, ofTrueTypeFont font) {
  fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  fbo.begin();
    ofClear(0, 0, 0, 0);
    ofBackground(0); // Set a background.
    ofEnableAlphaBlending();
    for (auto m : msgs) {
      // Go through each message, and draw on the fbo.
        glm::vec2 loc = glm::vec2(ofRandom(5, ofGetWidth()-100), ofRandom(5, ofGetHeight()));
        ofPushMatrix();
          ofColor c = ofColor::fromHsb(ofRandom(255), 255, 255);
          auto deg = ofRandom(-60, 60);
          ofSetColor(c, 175);
          ofTranslate(loc);
            ofRotateDeg(deg);
            font.drawString(m, 0, 0);
        ofPopMatrix();
    }
    ofDisableAlphaBlending();
  fbo.end();
}

void ofApp::readFile(string fileName, std::vector<string> &array) {
  auto buffer = ofBufferFromFile(fileName);
  auto lines = ofSplitString(buffer.getText(), "\n");
 
  for (auto l: lines) {
    auto i = l.find(":");
    if (i > 0) {
      auto s = l.substr(i+1);
      array.push_back(s);
    } else {
      auto b = array.back();
      b = b + "\n" + l; // Append the line to the last value in the vector.
      array[array.size()-1] = b;
    }
  }
}
