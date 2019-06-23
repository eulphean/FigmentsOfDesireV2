#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"
#include "ofxPostProcessing.h"
#include "Message.h"

struct AgentProperties {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshDimensions; // row, columns of the mesh.
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture.
  ofPoint meshOrigin; // Derived class populates this. 
  float vertexRadius;
};

enum DesireState {
  None,
  Attraction,
  Repulsion
};

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void update();
    void draw(bool debug, bool showTexture);
  
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
    void setTickle(float weight);
    void setStretch();
    void repulseBondedVertices();
  
    // Helpers
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    void setDesireState(DesireState state);
    void enableAttraction(); 
  
    // Vertices
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
  
    // Texture
    void createTexture(ofPoint meshSize);
    ofPoint getTextureSize();
  
    // Pubic iterator to access messages. 
    std::vector<Message>::iterator curMsg;
    std::vector<Message> messages;
  
    // Agent's partner
    Agent *partner = NULL;
  
    // Desires. 
    float desireRadius;
    DesireState desireState;

  protected:
    // Derived class needs to have access to these. 
    int numBogusMessages;
    std::vector<ofColor> palette;
    AbstractFilter *filter;
    FilterChain *filterChain;
    ofxPostProcessing post;
  
    // Weights
    float tickleWeight;
    float maxStretchWeight;
    float stretchWeight;
    float repulsionWeight;
    float vertexRepulsionWeight; 
    float attractionWeight;
    float seekWeight;
    float maxVelocity;
    
  private:
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
    void assignIndices(AgentProperties agentProps);
  
    // ----------------- Data members -------------------
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
  
    // Mesh.
    ofMesh mesh;
  
    // Seek
    glm::vec2 seekTargetPos;
  
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
  
    // Messages for this agent.
    std::vector<string> textMsgs;
  
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
    }
  
    Agent * agent;
    bool applyRepulsion;
    bool hasInterAgentJoint;
    bool applyAttraction;
    glm::vec2 targetPos; 
};
