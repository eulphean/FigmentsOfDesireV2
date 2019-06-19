// This is a class for a generic text message that was exchanged between Amay and Azra
// It has a location, color, and shape that gets drawn on the mesh.

#pragma once
#include "ofMain.h"

class Message {
  public:
    Message(glm::vec2 loc, ofColor col, float size, string msg);
    void draw(ofTrueTypeFont font);
  
    glm::vec2 location;
    ofColor color;
    float size;
    string message;
    float angle;
};
