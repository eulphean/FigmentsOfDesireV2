#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "Agent.h"
#include "Alpha.h"
#include "Beta.h"
#include "BgMesh.h"
#include "Kinect.h"
#include "Memory.h"
#include "Midi.h"
#include "SuperAgent.h"

#define PORT 8000

class ofApp : public ofBaseApp{

	public:
    // OF methods.
		void setup();
		void update();
		void draw();
    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    void exit();
  
    // Public helpers.
    void setupGui();
    void createAgents();
    void clearAgents();
    void updateAgentProps();
    void handleInteraction(); 
  
    // Behavior methods.
    void setBehavior(std::vector<glm::vec2> people);
    void clearInterAgentBonds(); 
  
    // Contact listening callbacks.
    void contactStart(ofxBox2dContactArgs &e);
    void contactEnd(ofxBox2dContactArgs &e);
  
    // Flags to turn/turn off certain features
    bool showGui;
    bool debug;
    bool hideKinectGui; 
    bool stopEverything;
    bool showTexture;
    bool drawFbo; // For saving frames.
    bool shouldBond;
    bool showVisibilityRadius;
  
    // Box2d world handle.
    ofxBox2d box2d;
  
    // Agents
    std::vector<Agent *> agents;
    // GUI parameters for Alpha/Beta agents. 
    AlphaAgentProperties alphaAgentProps;
    BetaAgentProperties betaAgentProps;
  
    // Screengrab fbo
    ofFbo screenGrabFbo;
    int screenCaptureIdx = 0;

  
    // GUI
    ofxPanel gui;
    ofParameterGroup settings;
  
    // General settings
    ofParameterGroup generalParams;
    ofParameter<float> alphaAgentProbability;
    ofParameter<float> audienceVisibilityRadius;
    ofParameter<int> numAgentsToCreate; 
  
    // Alpha Agent Group params. 
    ofParameterGroup alphaAgentParams;
    ofParameter<float> aVisibilityRadiusFactor;
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

    // Weights.
    ofParameter<float> aStretchWeight;
    ofParameter<float> aRepulsionWeight;
    ofParameter<float> aAttractionWeight;
    ofParameter<float> aTickleWeight;
    ofParameter<float> aVelocity;
  
    // Beta Agent GUI params.
    ofParameterGroup betaAgentParams;
    ofParameter<float> bVisibilityRadiusFactor; 
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
    ofParameter<float> bSideJointOffset;
    // Weights.
    ofParameter<float> bStretchWeight;
    ofParameter<float> bRepulsionWeight;
    ofParameter<float> bAttractionWeight;
    ofParameter<float> bTickleWeight;
    ofParameter<float> bVelocity;
  
    // InterAgentJoint GUI params.
    ofParameterGroup interAgentJointParams;
    ofParameter<float> iJointFrequency;
    ofParameter<float> iJointDamping;
    ofParameter<int> iMinJointLength;
    ofParameter<int> iMaxJointLength; 
  
  private:
  
    // Helper methods.
    void processOsc();
    void clearScreen();
    void removeJoints();
    void removeUnbonded();
    void drawSequence();
    glm::vec2 getBodyPosition(b2Body* body);
    void createWorld(bool createBonds);
    Agent *getClosestAgent(std::vector<Agent *> targetAgents, glm::vec2 targetPos);
    std::vector<Agent *> getVisibleAgents(glm::vec2 person);
    std::vector<glm::vec2> getInvisibleTargets(std::vector<glm::vec2> targets, Agent* a);
    void seek(); 
  
    std::vector<Memory> memories;
    std::vector<b2Body *> collidingBodies;
  
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
  
    // Kinect handle
    Kinect kinect;
    
    bool isOccupied;
  
    std::vector<glm::vec2> testPeople; 
};
