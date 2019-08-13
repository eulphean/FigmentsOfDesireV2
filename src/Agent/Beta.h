// Beta is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is ROUND. Female is also identified with a circular symbol. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

struct BetaAgentProperties {
  ofPoint textureSize;
  ofPoint vertexPhysics;
  float vertexRadius;
  float meshRadius;
  float sideJointOffset; 
  ofPoint centerJointPhysics;
  ofPoint sideJointPhysics;
  ofPoint meshOrigin;
}; 

class Beta : public Agent {
  public:
    Beta(ofxBox2d &box2d, BetaAgentProperties agentProps);
  
    void updateMesh();
    void createMesh(BetaAgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, BetaAgentProperties softBodyProperties);
  
    int numMeshPoints;
    void update();
};
