#include "Message.h"

Message::Message(glm::vec2 loc, ofColor col, float s, string msg) {
  location = loc;
  color = col;
  size = s;
  message = msg;
  angle = ofRandom(-60, 60);
}

void Message::draw(ofTrueTypeFont font) {
  if (message == "~") { // Draw bogus circle message.
    ofPushMatrix();
    ofTranslate(location);
      ofPushStyle();
        ofColor c = ofColor(color, 250);
        ofSetColor(c);
        ofDrawCircle(0, 0, size);
      ofPopStyle();
  ofPopMatrix();
  } else { // Draw actual string message.
    ofPushMatrix();
      ofTranslate(location);
      ofPushStyle();
        ofSetColor(color);
        font.drawString(message, 0, 0);
      ofPopStyle();
    ofPopMatrix();
  }
}
