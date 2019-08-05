// Alpha is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is RECTANGULAR. Male is also identified with a rectangle symbol.
#pragma once
#include "ofMain.h"
#include "Agent.h"

struct AlphaAgentProperties {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint textureSize; // w, h of the texture that gets mapped. 
  ofPoint meshRowsColumns; // row, columns of the mesh.
  ofPoint vertexPhysics;
  float vertexRadius;
  ofPoint jointPhysics;
  ofPoint meshOrigin;
}; 

class Alpha : public Agent {
  public:
    Alpha(ofxBox2d &box2d, AlphaAgentProperties agentProps);
  
    // Overriding methods. 
    void updateMesh();
    void createMesh(AlphaAgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AlphaAgentProperties softBodyProperties);
  
    void update();
};

struct AgentProperties {
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture.
  ofPoint meshOrigin; // Derived class populates this.
  float vertexRadius;
};
