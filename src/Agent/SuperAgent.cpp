#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
  maxExchangeCounter = 20;
  curExchangeCounter = 20; // Note: Currently diabling the exchange of the body texture. Set it 0 to enable it. 
}

void SuperAgent::update(ofxBox2d &box2d,
                          std::vector<Memory> &memories,
                            std::vector<int> &removeVertices, bool shouldBond) {
  auto cleanJoint = !shouldBond || agentA->canExplode() || agentB->canExplode();

  // Go through all the joints and delete them if shouldn't bond.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    // Body A data.
    auto bodyA = j->joint->GetBodyA();
    auto dataA = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    glm::vec2 locA = getBodyPosition(bodyA);
    
    // Body B data.
    auto bodyB = j->joint->GetBodyB();
    auto dataB = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    glm::vec2 locB = getBodyPosition(bodyB);
    
    if (cleanJoint) {
      box2d.getWorld()->DestroyJoint(j->joint);

      // Update bodyA's data.
      dataA->hasInterAgentJoint = false;
      bodyA->SetUserData(dataA);
      removeVertices.push_back(dataA->jointMeshIdx);

      // Update bodyB's data.
      dataB->hasInterAgentJoint = false;
      bodyB->SetUserData(dataB);
      removeVertices.push_back(dataB->jointMeshIdx); 
      
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
  }
}

void SuperAgent::updateMeshIdx() {
   // For all the joints
   // Readd the vertices in the mesh
   // Update the vertex joint mesh idx.
   for (auto &j : joints) {
     auto bodyA = j->joint->GetBodyA();
     auto dataA = reinterpret_cast<VertexData*>(bodyA->GetUserData());
     glm::vec2 locA = getBodyPosition(bodyA);
     dataA->jointMeshIdx = curMeshIdx;
     bodyA->SetUserData(dataA);

     // Body B data.
     auto bodyB = j->joint->GetBodyB();
     auto dataB = reinterpret_cast<VertexData*>(bodyB->GetUserData());
     glm::vec2 locB = getBodyPosition(bodyB);
     dataB->jointMeshIdx = curMeshIdx + 1;
     bodyB->SetUserData(dataB);
     
     SuperAgent::insertJointMesh(glm::vec3(locA.x, locA.y, 0), glm::vec3(locB.x, locB.y, 0));
     SuperAgent::curMeshIdx += 2;
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
  SuperAgent::jointMesh.enableIndices();
  SuperAgent::jointMesh.setupIndicesAuto();
}

void SuperAgent::insertJointMesh(glm::vec3 v1, glm::vec3 v2) {
  // Get the bodies from the joint and add them in the mesh
  SuperAgent::jointMesh.addVertex(v1);
  SuperAgent::jointMesh.addVertex(v2);
}

void SuperAgent::insertIndices(int idx1, int idx2) {
  SuperAgent::jointMesh.addIndex(idx1);
  SuperAgent::jointMesh.addIndex(idx2); 
}

void SuperAgent::drawJointMesh() {
  ofPushStyle();
    ofSetColor(ofColor::red);
    SuperAgent::jointMesh.draw();
  ofPopStyle(); 
}
