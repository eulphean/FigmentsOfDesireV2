// Alpha is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is RECTANGULAR. Male is also identified with a rectangle symbol.
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Alpha : public Agent {
  public:
    Alpha(ofxBox2d &box2d, AgentProperties agentProps);
  
    // Overriding methods. 
    void updateMesh();
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
  
    void update();
};
