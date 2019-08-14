#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"

enum DesireState {
  None,
  Attraction,
  Repulsion
};

// Every agent is inscribed with certain messages. Right now they are just
// used to create a texture for the agent. They might be used as certain
// communicative elements between messages. Not sure what it could be but
// could be something interesting. 
class Message {
  public:
    Message(glm::vec2 loc, ofColor col, float size);
    void draw(ofTrueTypeFont font);
  
    glm::vec2 location;
    ofColor color;
    float size;
};

// Common Agent Props
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
};

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, ofPoint textureSize);
    void draw(bool debug, bool showTexture);
    virtual void update(AgentProps alphaProps, AgentProps betaProps);
  
    // Clean the agent
    void clean(ofxBox2d &box2d);
  
    // Behaviors
    void applyBehaviors();
    void handleRepulsion();
    void handleAttraction();
    void handleStretch();
    void handleVertexBehaviors();
    void handleTickle();
  
    // Enabling behaviors
    void tickle();
    void stretch();
    void repulseBondedVertices();
  
    // Helpers
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    void setDesireState(DesireState state);
  
    // Vertices and Joints
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
  
    // Texture
    void createTexture(ofPoint meshSize);
    ofPoint getTextureSize();
  
    // Public iterator to access messages. 
    std::vector<Message>::iterator curMsg;
    std::vector<Message> messages;
  
    // Agent's partner
    Agent *partner = NULL;
  
    // Desires. 
    float desireRadius;
    DesireState desireState;

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
  
    // Each derived class will override these methods as they define their own
    // meshes and soft bodies.
    virtual void updateMesh() {};
    virtual void updateWeights(AgentProps props) {}; 
  
    // Mesh must be accessible in the derived class. 
    ofMesh mesh;
    
  private:
    void assignIndices(ofPoint textureSize);
  
    // ----------------- Data members -------------------
  
    // Tickle.
    bool applyTickle;
  
    // Stretch out.
    bool applyStretch;
  
    // Attraction.
    bool applyAttraction;
  
    // Repulsion
    bool applyRepulsion;
  
    // Texture
    ofFbo firstFbo;
    ofFbo secondFbo;
  
    // Figment's corner indices
    int cornerIndices[4];
    vector<int> boundaryIndices;
  
    ofTrueTypeFont font;
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
