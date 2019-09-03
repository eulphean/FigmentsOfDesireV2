// This is a class for memory object that gets created for each joint.
#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"

class Memory {
  public:
    Memory(ofxBox2d &box2d, glm::vec2 location, bool isAgent = false);
    void update();
    void draw();
    void destroy();
    bool shouldRemove;
    ofColor finalColor;
    ofColor color; 
  
  private:
    std::shared_ptr<ofxBox2dCircle> mem;
    unsigned long curTime;
    unsigned long maxTime;
    unsigned long elapsedTime; 
};
