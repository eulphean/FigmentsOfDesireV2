// Beta is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is ROUND. Female is also identified with a circular symbol. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Beta : public Agent {
  public:
    Beta(ofxBox2d &box2d, AgentProperties agentProps);
  
    void updateMesh();
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
  
    float faceRadius; float faceCircumference; int meshPoints;
    float softJointLength = 1.5;
  
    void update();
};
