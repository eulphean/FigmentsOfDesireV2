#include "Message.h"

Message::Message(glm::vec2 loc, ofColor col, float s, string msg) {
  location = loc;
  color = col;
  size = s;
  message = msg;
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
  }
}
