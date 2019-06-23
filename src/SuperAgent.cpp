#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
  maxExchangeCounter = 20;
  curExchangeCounter = 0;
}

void SuperAgent::update(ofxBox2d &box2d, std::vector<Memory> &memories, bool shouldBond, int maxJointForce) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    if (!shouldBond) {
      box2d.getWorld()->DestroyJoint(j->joint);
      // Get the bodies
      auto bodyA = j->joint->GetBodyA();
      auto bodyB = j->joint->GetBodyB();

      // Update bodyA's data.
      auto data = reinterpret_cast<VertexData*>(bodyA->GetUserData());
      data->hasInterAgentJoint = false;
      bodyA->SetUserData(data);

      // Update bodyB's data.
      data = reinterpret_cast<VertexData*>(bodyB->GetUserData());
      data->hasInterAgentJoint = false;
      bodyB->SetUserData(data);
      
      // Create a new memory object for each interAgentJoint and populate the vector.
      glm::vec2 locA = getBodyPosition(bodyA);
      glm::vec2 locB = getBodyPosition(bodyB);
      glm::vec2 avgLoc = (locA + locB)/2;
      
      Memory mem(box2d, avgLoc);
      memories.push_back(mem);

      return true;
    } else {
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
      curExchangeCounter -= 0.8;
    }
  }
}

void SuperAgent::draw() {
  for (auto j : joints) {
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofSetLineWidth(0.4);
      j->draw();
    ofPopStyle();
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
  b2Vec2 pos      = body->GetLocalCenter();
  b2Vec2 b2Center = b2Mul(xf, pos);
  auto p = worldPtToscreenPt(b2Center);
  return glm::vec2(p.x, p.y);
}
