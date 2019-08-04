#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxGui.h"
#include "Agent.h"
#include "Amay.h"
#include "Azra.h"
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
  
    // Interactive elements
		void keyPressed(int key);
    void exit();
  
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
    AgentProperties agentProps;
  
    // GUI
    ofxPanel gui;
    ofParameterGroup settings; 
  
    // Mesh group
    ofParameterGroup meshParams;
    ofParameter<int> meshColumns;
    ofParameter<int> meshRows;
    ofParameter<int> meshWidth;
    ofParameter<int> meshHeight;
  
    // Vertex group
    ofParameterGroup vertexParams;
    ofParameter<float> vertexRadius;
    ofParameter<float> vertexDensity;
    ofParameter<float> vertexBounce;
    ofParameter<float> vertexFriction;
  
    // Agent joint (joints inside the agent)
    ofParameterGroup jointParams;
    ofParameter<float> jointFrequency;
    ofParameter<float> jointDamping;
  
    // InterAgentJoint
    ofParameterGroup interAgentJointParams;
    ofParameter<float> frequency;
    ofParameter<float> damping;
    ofParameter<int> maxJointForce;
  
    // Background group.
    ofParameterGroup bgParams;
    ofParameter<int> rectWidth;
    ofParameter<int> rectHeight;
    // Callbacks to create new background. 
    void widthChanged(int & newWidth);
    void heightChanged(int & newHeight);
    void updateForce(int & newVal);
    void updateParams(float & newVal);
    ofParameter<int> attraction;
    ofParameter<int> repulsion;
    ofParameter<float> shaderScale;
  
    // Screengrab fbo
    ofFbo screenGrabFbo;
    int screenCaptureIdx = 0;
  
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
