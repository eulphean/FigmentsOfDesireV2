#include "Message.h"

Message::Message(glm::vec2 loc, ofColor col, float s) {
  location = loc;
  color = col;
  size = s;
}

void Message::draw(ofTrueTypeFont font) {
  ofPushMatrix();
    ofTranslate(location);
      ofPushStyle();
        ofColor c = ofColor(color, 250);
        ofSetColor(c);
        ofDrawCircle(0, 0, size);
      ofPopStyle();
  ofPopMatrix();
}
