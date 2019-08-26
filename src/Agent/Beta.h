// Beta is a type of synthetic agent with specific behaviors and traits. It is organic in nature
// and is ROUND. Female is also identified with a circular symbol. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Beta : public Agent {
  public:
    Beta(ofxBox2d &box2d, BetaAgentProperties agentProps);
  
    // Overridden methods.
    void updateMesh();
    void createMesh(BetaAgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, BetaAgentProperties softBodyProperties);
    void updateWeights(BetaAgentProperties betaProps);
  
    void update(AlphaAgentProperties alphaProps, BetaAgentProperties betaProps);
  
    int numMeshPoints;
    void update(AgentProps alphaProps, AgentProps betaProps);
};
