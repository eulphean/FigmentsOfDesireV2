#include "Memory.h"
#include "Agent.h" 

Memory::Memory(ofxBox2d &box2d, glm::vec2 location) {
  mem = std::make_shared<ofxBox2dCircle>();
  mem -> setPhysics(0.3, 0.3, 0.3); // bounce, density, friction
  mem -> setup(box2d.getWorld(), location.x, location.y, ofRandom(4, 8));
  mem -> setFixedRotation(true);
  mem -> setVelocity(ofRandom(-5, 5), ofRandom(-5, 5)); // Random velocity
  mem -> setData(new VertexData(NULL)); // No agent pointer for this.
  
  curTime = ofGetElapsedTimeMillis();
  maxTime = ofRandom(5000, 10000);
  shouldRemove = false;
  finalColor = ofColor(0xDBDBDB);
  color = ofColor(0x525151);
}

void Memory::update() {
  elapsedTime = ofGetElapsedTimeMillis() - curTime;
  if (elapsedTime >= maxTime) {
    shouldRemove = true; 
  }
}

void Memory::draw() {
  ofPushMatrix();
    ofTranslate(mem->getPosition());
    ofPushStyle();
      color = color.lerp(finalColor, 1.0);
      auto opacity = ofMap(elapsedTime, 0, maxTime, 255, 50, true);
      ofSetColor(color, opacity);
      ofDrawCircle(0, 0, mem->getRadius());
    ofPopStyle();
  ofPopMatrix();
}

