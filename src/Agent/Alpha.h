// Alpha is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is RECTANGULAR. Male is also identified with a rectangle symbol.
#pragma once
#include "ofMain.h"
#include "Agent.h"

struct AlphaAgentProperties : public AgentProps {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshRowsColumns; // row, columns of the mesh.
  ofPoint jointPhysics;
}; 

class Alpha : public Agent {
  public:
    Alpha(ofxBox2d &box2d, AlphaAgentProperties agentProps);
  
    // Overriding methods. 
    void updateMesh();
    void createMesh(AlphaAgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AlphaAgentProperties softBodyProperties);
    void updateWeights(AgentProps alphaProps);
  
    void update(AgentProps alphaProps, AgentProps betaProps);
};

struct AgentProperties {
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture.
  ofPoint meshOrigin; // Derived class populates this.
  float vertexRadius;
};
