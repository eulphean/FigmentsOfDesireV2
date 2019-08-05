#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxGui.h"
#include "Agent.h"
#include "Alpha.h"
#include "Beta.h"
#include "SuperAgent.h"
#include "ofxOsc.h"
#include "Midi.h"
#include "BgMesh.h"
#include "Memory.h"

#define PORT 8000

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
    void keyPressed(int key);
    void exit();
  
    void setupGui();
    void createAgents();
    void clearAgents();
    void updateAgentProps();
  
    // Behavior activating functions.
    void attract();
    void repel();
    void stretch();
    void enableBonding();
  
    // Contact listening callbacks.
    void contactStart(ofxBox2dContactArgs &e);
    void contactEnd(ofxBox2dContactArgs &e);
  
    // Flags to turn/turn off certain features
    bool hideGui;
    bool debug;
    bool stopEverything;
    bool showTexture;
    bool drawFbo; // For saving
    bool shouldBond;
  
    // Box2d world handle.
    ofxBox2d box2d;
  
    // Agents
    std::vector<Agent *> agents;
    // GUI parameters for Alpha/Beta agents. 
    AlphaAgentProperties alphaAgentProps;
    BetaAgentProperties betaAgentProps;
  
    // GUI
    ofxPanel gui;
    ofParameterGroup settings;
  
    // Screengrab fbo
    ofFbo screenGrabFbo;
    int screenCaptureIdx = 0;
  
    // Alpha Agent Group params. 
    ofParameterGroup alphaAgentParams;
    ofParameter<int> aMeshColumns;
    ofParameter<int> aMeshRows;
    ofParameter<int> aMeshWidth;
    ofParameter<int> aMeshHeight;
    ofParameter<int> aTextureWidth;
    ofParameter<int> aTextureHeight;
    ofParameter<float> aVertexDensity;
    ofParameter<float> aVertexBounce;
    ofParameter<float> aVertexFriction;
    ofParameter<float> aVertexRadius;
    ofParameter<float> aJointFrequency;
    ofParameter<float> aJointDamping;
  
    // Beta Agent GUI params.
    ofParameterGroup betaAgentParams;
    ofParameter<int> bTextureWidth;
    ofParameter<int> bTextureHeight;
    ofParameter<float> bMeshRadius;
    ofParameter<float> bVertexDensity;
    ofParameter<float> bVertexBounce;
    ofParameter<float> bVertexFriction;
    ofParameter<float> bVertexRadius;
    ofParameter<float> bCenterJointFrequency;
    ofParameter<float> bCenterJointDamping;
    ofParameter<float> bSideJointFrequency;
    ofParameter<float> bSideJointDamping;
  
    // InterAgentJoint GUI params.
    ofParameterGroup interAgentJointParams;
    ofParameter<float> iJointFrequency;
    ofParameter<float> iJointDamping;
    ofParameter<int> iMinJointLength;
    ofParameter<int> iMaxJointLength; 
  
    // Background GUI params.
    ofParameterGroup bgParams;
    ofParameter<int> bgRectWidth;
    ofParameter<int> bgRectHeight;
    ofParameter<int> bgAttraction;
    ofParameter<int> bgRepulsion;
    void bgUpdateParams(int & newVal); // For attraction/repulsion
    void bgUpdateSize(int & newVal); // For rectWidth/rectHeight
  
  private:
    std::vector<Memory> memories;
    std::vector<b2Body *> collidingBodies;
  
    // Helper methods.
    void processOsc();
    void clearScreen();
    void removeJoints();
    void removeUnbonded();
    void drawSequence();
    glm::vec2 getBodyPosition(b2Body* body);
    void createWorld(bool createBonds); 
  
    // Super Agents (Inter Agent Bonding Logic)
    void createSuperAgents();
    std::shared_ptr<ofxBox2dJoint> createInterAgentJoint(b2Body *bodyA, b2Body *bodyB);
    void evaluateBonding(b2Body* bodyA, b2Body* bodyB, Agent *agentA, Agent *agentB);
    bool canVertexBond(b2Body* body, Agent *curAgent);
  
    // SuperAgents => These are abstract agents that have a bond with each other. 
    std::vector<SuperAgent> superAgents;
  
    // Bounds
    ofRectangle bounds;
  
    // Background
    BgMesh bg;
  
    // Serial
    ofSerial serial;
  
    // OSC remote.
    ofxOscReceiver receiver;
};
