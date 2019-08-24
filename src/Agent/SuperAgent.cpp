#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
  maxExchangeCounter = 20;
  curExchangeCounter = 20; // Note: Currently diabling the exchange of the body texture. Set it 0 to enable it. 
}

void SuperAgent::update(ofxBox2d &box2d, std::vector<Memory> &memories, bool shouldBond) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    // Body A data.
    auto bodyA = j->joint->GetBodyA();
    auto dataA = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    glm::vec2 locA = getBodyPosition(bodyA);
    
    // Body B data.
    auto bodyB = j->joint->GetBodyB();
    auto dataB = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    glm::vec2 locB = getBodyPosition(bodyB);
    
    if (!shouldBond) {
      box2d.getWorld()->DestroyJoint(j->joint);

      // Update bodyA's data.
      dataA->hasInterAgentJoint = false;
      bodyA->SetUserData(dataA);

      // Update bodyB's data.
      dataB->hasInterAgentJoint = false;
      bodyB->SetUserData(dataB);
      
      // Create a new memory object for each interAgentJoint and populate the vector.
      glm::vec2 avgLoc = (locA + locB)/2;
      
      Memory mem(box2d, avgLoc);
      memories.push_back(mem);

      return true;
    } else {
      // Update SuperAgent's jointMesh using the current positions of the vertices in the current joint.
      auto idxA = dataA->jointMeshIdx;
      auto idxB = dataB->jointMeshIdx;
      
      // Update vertex at idxA with body A's location
      auto vertex = SuperAgent::jointMesh.getVertex(idxA);
      vertex.x = locA.x; vertex.y = locA.y;
      SuperAgent::jointMesh.setVertex(idxA, vertex);
      
      // Update vertex at idxB with body B's location
      vertex = SuperAgent::jointMesh.getVertex(idxB);
      vertex.x = locB.x; vertex.y = locB.y;
      SuperAgent::jointMesh.setVertex(idxB, vertex);
      
      return false;
    }
  });
  
  if (joints.size() == 0) {
    shouldRemove = true;
  } else {
    // When it's a super agent, that means it's bonded.
    // Check if it's ready to swap messages.
    if (curExchangeCounter <= 0) {
      std::vector<Message>::iterator aMessage = agentA -> curMsg;
      std::vector<Message>::iterator bMessage = agentB -> curMsg;
      
      // Save the temp message.
      Message swap = Message(aMessage->location, aMessage->color, aMessage->size);
      
      // Assign A message
      aMessage->color = bMessage->color;
      aMessage->size = bMessage->size;
      
      // Assign B message
      bMessage->color = swap.color;
      bMessage->size = swap.size;
      
      // Change the iteretor to point to a unique message now
      aMessage = agentA->messages.begin() + (int) ofRandom(0, agentA -> messages.size() - 1);
      bMessage = agentB->messages.begin() + (int) ofRandom(0, agentB -> messages.size() - 1);
      
      // Update iterators for the swap.
      agentA -> curMsg = aMessage;
      agentB -> curMsg = bMessage;
      
      // Create new textures the two agents as they have just gone through a swap.
      agentA->createTexture(agentA->getTextureSize());
      agentB->createTexture(agentB->getTextureSize());
      
      // Reset exchange counter since
      curExchangeCounter = maxExchangeCounter;
    } else {
      // curExchangeCounter -= 0.8;
      // Note: Disabling the exchange counter for now. Evaluate if and when this feature is really required.
      // Is it really required?
      // This is a good question. 
    }
  }
}

// Check if super body already exists. 
bool SuperAgent::contains(Agent *agent1, Agent *agent2) {
  if (agent1 == agentA) {
    if (agent2 == agentB) {
      return true;
    } else {
      return false;
    }
  } else if (agent2 == agentB) {
    if (agent1 == agentA) {
      return true;
    } else {
      return false;
    }
  }
}

void SuperAgent::clean(ofxBox2d &box2d) {
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  joints.clear();
}

glm::vec2 SuperAgent::getBodyPosition(b2Body* body) {
  auto xf = body->GetTransform();
  b2Vec2 pos = body->GetLocalCenter();
  b2Vec2 b2Center = b2Mul(xf, pos);
  auto p = worldPtToscreenPt(b2Center);
  return glm::vec2(p.x, p.y);
}

// Initialize the static variable
ofMesh SuperAgent::jointMesh;
int SuperAgent::curMeshIdx = 0;

void SuperAgent::initJointMesh() {
  SuperAgent::jointMesh.clear();
  SuperAgent::curMeshIdx = 0; 
  SuperAgent::jointMesh.setMode(OF_PRIMITIVE_LINES);
}

void SuperAgent::insertJointMesh(glm::vec3 v1, glm::vec3 v2) {
  // Get the bodies from the joint and add them in the mesh
  SuperAgent::jointMesh.addVertex(v1);
  SuperAgent::jointMesh.addVertex(v2);
}

void SuperAgent::drawJointMesh() {
  ofPushStyle();
    ofSetColor(ofColor::red);
    SuperAgent::jointMesh.draw();
  ofPopStyle(); 
}
