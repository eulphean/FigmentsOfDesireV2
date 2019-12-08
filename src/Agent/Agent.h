#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"
#include "Instrument.h"

// Current behavior of the agent.
enum Behavior {
  None,
  Attract,
  Repel,
  SpecialRepel,
  Stretch,
  Shock
};

// Every agent is inscribed with certain messages. Right now they are just
// used to create a texture for the agent. They might be used as certain
// communicative elements between messages. Not sure what it could be but
// could be something interesting. 
class Message {
  public:
    Message(glm::vec2 loc, ofColor col, float size);
    void draw();
  
    glm::vec2 location;
    ofColor color;
    float size;
};

// Agent Props.
struct AgentProps {
  ofPoint meshOrigin;
  ofPoint textureSize; // w, h of the texture that gets mapped.
  ofPoint vertexPhysics;
  float vertexRadius; 
  // Weights
  float stretchWeight;
  float repulsionWeight;
  float attractionWeight;
  float tickleWeight;
  float velocity;
  // Radius
  float visibilityRadiusFactor; 
};

struct AlphaAgentProperties : public AgentProps {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshRowsColumns; // row, columns of the mesh.
  ofPoint jointPhysics;
};

struct BetaAgentProperties : public AgentProps {
  float meshRadius;
  float sideJointOffset;
  ofPoint centerJointPhysics;
  ofPoint sideJointPhysics;
};

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, ofPoint textureSize);
    void draw(bool showVisibilityRadius, bool showTexture);
    virtual void update(AlphaAgentProperties alphaProps, BetaAgentProperties betaProps);
  
    // Clean the agent
    void clean(ofxBox2d &box2d);
  
    // Behaviors
    void handleBehaviors();
    void handleAttraction();
    void handleRepulsion();
    void handleSpecialRepulsion(); 
    void handleStretch();
    void handleShock();
    void handleVertexBehaviors();
    void agentStretchSound(bool on);
    void handleExplosion();
  
    // Helpers
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    void setBehavior(Behavior behavior, std::vector<glm::vec2> pos = {}, bool overrideCoolDown = false);
    bool canExplode();
  
    // Vertices and Joints
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
  
    // Texture
    void createTexture(ofPoint meshSize);
    ofPoint getTextureSize();
  
    // Public iterator to access messages. 
    std::vector<Message>::iterator curMsg;
    std::vector<Message> messages;
  
    // Current behavior of the agent.
    Behavior currentBehavior;
  
    // Visibility Radius
    float visibilityRadius;
  
    // Counter to keep track of the time spent on each agent when interacting with it. 
    int stretchCounter;
    int maxStretchCounter;
  
    // Instrument control.
    Instrument instrument;
    pdsp::TriggerControl gate_ctrl;

  protected:
    // Derived class needs to have access to these. 
    int numMessages; // Number of messages each agent has
    std::vector<ofColor> palette;
    AbstractFilter *filter;
  
    // Weights
    float maxStretchWeight;
    float maxRepulsionWeight;
    float maxTickleWeight;
    float maxAttractionWeight;
    float maxVelocity;
  
    // These weights are lerped. 
    float stretchWeight;
    float repulsionWeight;
    float attractionWeight; 
  
    // Each derived class will override these methods as they define their own
    // meshes and soft bodies.
    virtual void updateMesh() {};
    virtual void updateWeights(AgentProps props) {}; 
  
    // Mesh must be accessible in the derived class. 
    ofMesh mesh;
    
  private:
    // ----------------- Data members -------------------
    // Texture
    ofFbo firstFbo;
    ofFbo secondFbo;
    ofFbo textFbo;
    ofTrueTypeFont font;
  
    // Figment's corner indices
    int cornerIndices[4];
    vector<int> boundaryIndices;
  
    // Target position for behaviors
    std::vector<glm::vec2> targetPositions;
  
    // Wait time before being able to be applied with another force. 
    int coolDown;
    int maxCoolDown;
};

// Data Structure to hold a pointer to the agent instance
// to which this vertex belongs to.
class VertexData {
  public:
    VertexData(Agent *ptr) {
      agent = ptr;
      applyRepulsion = false;
      applyAttraction = false;
      hasInterAgentJoint = false;
      jointMeshIdx = -1;
    }
  
    // Point to this bodies' agent.
    Agent *agent;
    bool applyRepulsion;
    bool applyAttraction;
  
    bool hasInterAgentJoint;
    int jointMeshIdx; // Idx for the mesh used to draw interAgent joints.
  
    glm::vec2 targetPos;
};
