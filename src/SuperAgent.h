#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "Agent.h"
#include "Memory.h"
#include "Midi.h"

// Subsection body that is torn apart from the actual texture and falls on the ground.
// The entire thing acts like one unique bond now. 
class SuperAgent {
  public:
    void setup(Agent *agentA, Agent *agentB, std::shared_ptr<ofxBox2dJoint>);
    void update(ofxBox2d &box2d, std::vector<Memory> &memories, bool shouldBond, int maxJointForce);
    void draw();
    bool contains(Agent *agentA, Agent *agentB);
    void clean(ofxBox2d &box2d);
    glm::vec2 getBodyPosition(b2Body *body);
    void createMemory();
  
    Agent *agentA;
    Agent *agentB;
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints;  // These are interAgent joints.
    bool shouldRemove = false;
  
    float curExchangeCounter;
    float maxExchangeCounter;
};
