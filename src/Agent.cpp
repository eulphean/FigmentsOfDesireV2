#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps) {
  // Initialize the iterator.
  createTexture(agentProps.meshSize);
  curMsg = messages.begin(); // Need the message to draw
  
  // Assign corner and boundary indices for applying forces on vertices. 
  assignIndices(agentProps);
  
  // Calculate a desireRadius based on the size of the mesh
  auto area = agentProps.meshSize.x * agentProps.meshSize.y;
  desireRadius = sqrt(area/PI);
  
  // Target position
  seekTargetPos = glm::vec2(ofRandom(150, ofGetWidth() - 200), ofRandom(50 , 250));
  
  // These are actions. But, what are the desires?
  applyStretch = true;
  applyTickle = false;
  applyAttraction = false;
  applyRepulsion = false; 
  
  // Current desire state. 
  desireState = None;
}

void Agent::update() {  
  // Print the velocity of vertices.
  for (auto &v : vertices) {
    auto vel = v->getVelocity().length();
    if (vel > maxVelocity) {
      // Normalize current velocity and multiply it by 20
      auto n = v->getVelocity().normalize();
      n = n * maxVelocity; // Max velocity weight
      v->setVelocity(n.x, n.y);
    }
  }
  
  applyBehaviors();
}

void Agent::draw(bool debug, bool showTexture) {
  // Draw the meshes.
  // Draw the soft bodies.
  ofPushStyle();
    for(auto v: vertices) {
      ofPushMatrix();
        ofTranslate(v->getPosition());
        ofSetColor(ofColor::red);
        ofFill();
        ofDrawCircle(0, 0, v->getRadius());
      ofPopMatrix();
    }
  ofPopStyle();
  
  if (showTexture) {
    secondFbo.getTexture().bind();
      mesh.draw();
    secondFbo.getTexture().unbind();
  } else {
    ofPushStyle();
    for(auto j: joints) {
      ofPushMatrix();
        ofSetColor(ofColor::green);
        j->draw();
      ofPopMatrix();
    }
    ofPopStyle();
  }

  if (debug) {
    auto centroid = mesh.getCentroid();
    ofPushStyle();
      ofPushMatrix();
        ofTranslate(centroid);
        ofNoFill();
        ofSetColor(ofColor::white);
        ofDrawCircle(0, 0, desireRadius);
      ofPopMatrix();
    ofPopStyle();
  }
}

void Agent::assignIndices(AgentProperties agentProps) {
  // Store the corner indices in this array to access it when applying forces.
  auto rows = agentProps.meshDimensions.x; auto cols = agentProps.meshDimensions.y;

  // Corners
  cornerIndices[0] = 0; cornerIndices[1] = (cols-1) + 0 * (cols-1);
  cornerIndices[2] = 0 + (rows-1) * (cols-1); cornerIndices[3] = (cols-1) + (rows-1) * (cols-1);
  
  // Boundaries.
  
  // TOP
  int x; int y = 0;
  for (x = 0; x < cols-1; x++) {
    int idx = x + y * (cols-1);
    boundaryIndices.push_back(idx);
  }
  
  // BOTTOM
  y = rows-1;
  for (x = 0; x < cols-1; x++) {
    int idx = x + y * (cols-1);
    boundaryIndices.push_back(idx);
  }
  
  // LEFT
  x = 0;
  for (y = 0; y < rows-1; y++) {
    int idx = x + y * (cols-1);
    boundaryIndices.push_back(idx);
  }
  
  // RIGHT
  x = cols-1;
  for (y = 0; y < rows-1; y++) {
    int idx = x + y * (cols-1);
    boundaryIndices.push_back(idx);
  }
}

ofPoint Agent::getTextureSize() {
  return ofPoint(secondFbo.getWidth(), secondFbo.getHeight());
}

void Agent::clean(ofxBox2d &box2d) {
  // Remove joints.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  // Remove vertices
  ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> c){
    return true;
  });

  // Clear all.
  joints.clear();
  vertices.clear();
}

void Agent::createTexture(ofPoint meshSize) {
  // Create spots on the agent's body
  for (int i = 0; i < numBogusMessages; i++) {
    // Pick a random location on the mesh.
    int w = meshSize.x; int h = meshSize.y;
    auto x = ofRandom(0, w); auto y = ofRandom(0, h);
    
    // Pick a random color for the message (anything except the background)
    int idx = ofRandom(1, palette.size());
    ofColor c = ofColor(palette.at(idx));
    
    // Pick a random size (TOOD: Based off on the length of the message).
    int size = ofRandom(10, 15);
    
    // Create a message.
    Message m = Message(glm::vec2(x, y), c, size);
    messages.push_back(m);
  }
  
  // Create 1st fbo and draw all the messages. 
  firstFbo.allocate(meshSize.x*2, meshSize.y*2, GL_RGBA);
  firstFbo.begin();
    ofClear(0, 0, 0, 0);
  
    // Assign background.
    int randIdx = ofRandom(palette.size());
    ofColor c = ofColor(palette.at(randIdx), 250);
    ofBackground(c);
  
    // Draw assigned messages.
    for (auto m : messages) {
      m.draw(font);
    }

  firstFbo.end();
  
  // Create 2nd fbo and draw with filter and postProcessing
  secondFbo.allocate(100, 100, GL_RGBA);
  secondFbo.begin();
    ofClear(0, 0, 0, 0);
    filterChain->begin();
      firstFbo.getTexture().drawSubsection(0, 0, meshSize.x, meshSize.y, 0, 0);
    filterChain->end();
  secondFbo.end();
}

void Agent::applyBehaviors()  {
  // ----Current actions/behaviors---
  handleStretch();
  handleRepulsion();
  handleAttraction();
  handleTickle();
  
  // Behavior of individual bodies on the agent (all circles mostly)
  handleVertexBehaviors();
}

void Agent::handleVertexBehaviors() {
  for (auto &v : vertices) {
    auto data = reinterpret_cast<VertexData*>(v->getData());
    if (data->applyRepulsion) {
      // Repel this vertex from it's partner's centroid especially
      //auto pos = glm::vec2(partner->getCentroid().x, partner->getCentroid().y);
      auto pos = data->targetPos;
      v->addRepulsionForce(pos.x, pos.y, vertexRepulsionWeight * 15);
      
      // Reset repulsion parameter on the vertex.
      data->applyRepulsion = false;
      v->setData(data);
    }
    
    if (data->applyAttraction) {
      auto data = reinterpret_cast<VertexData*>(v->getData());
      auto pos = glm::vec2(data->targetPos.x, data->targetPos.y);
      //auto pos = data->targetPos;
      v->addAttractionPoint({pos.x, pos.y}, attractionWeight * 20);
      
      // Reset repulsion parameter on the vertex.
      data->applyAttraction = false;
      v->setData(data);
    }
  }
}

void Agent::handleRepulsion() {
  // Go through all the vertices.
  // Get the data and check if it has.
  if (applyRepulsion) {
    repulsionWeight = ofLerp (repulsionWeight, vertexRepulsionWeight, 0.1);
    for (auto &v : vertices) {
      auto data = reinterpret_cast<VertexData*>(v->getData());
      if (data->hasInterAgentJoint) {
         v->addRepulsionForce(partner->getCentroid().x, partner->getCentroid().y, repulsionWeight);
      }
    }
    
    desireState = None;
    
    if (vertexRepulsionWeight-repulsionWeight < vertexRepulsionWeight/2) {
      applyRepulsion = false;
      repulsionWeight = 0;
    }
  }
}

void Agent::handleAttraction() {
  // Find the closest vertex from the boundary of indices and attract it to the
  // centroid of the other mesh
  if (applyAttraction) {
    float minD = 9999; int minIdx;
    // Find minimum distance idx.
    for (auto idx : boundaryIndices) {
      auto v = vertices[idx];
      auto data = reinterpret_cast<VertexData*>(v->getData());
      
      // If it has a bond, don't add the attraction force.
      if (!data->hasInterAgentJoint) {
        auto p = glm::vec2(v->getPosition().x, v->getPosition().y);
        auto d = glm::distance(p, partner->getCentroid());
        if (d < minD) {
          minD = d; minIdx = idx;
        }
      }
    }
    
    auto vertexPos = vertices[minIdx]->getPosition();
    auto d = glm::distance(partner->getCentroid(), glm::vec2(vertexPos.x, vertexPos.y)); // Distance till the centroid.
    float newWeight = ofMap(d, desireRadius * 5, 0, attractionWeight, 0, true);
    auto pos = glm::vec2(partner->getCentroid().x, partner->getCentroid().y);
    vertices[minIdx]->addAttractionPoint({pos.x, pos.y}, newWeight);
  }
}

void Agent::handleStretch() {
  // Check for counter.
  if (applyStretch) { // Time to apply a stretch.
    stretchWeight = ofLerp(stretchWeight, maxStretchWeight, 0.1);
    for (auto &v : vertices) {
      auto data = reinterpret_cast<VertexData*>(v->getData());
      if (!data->hasInterAgentJoint) {
        if (ofRandom(1) < 0.2 ) {
          v->addAttractionPoint({mesh.getCentroid().x, mesh.getCentroid().y}, stretchWeight);
        } else {
          v->addRepulsionForce(mesh.getCentroid().x, mesh.getCentroid().y, stretchWeight);
        }
        v->setRotation(ofRandom(150));
      }
    }
    
    if (maxStretchWeight - stretchWeight < 0.5) {
      stretchWeight = 0;
      applyStretch = false;
    }
  }
}

void Agent::handleTickle() {
  // Does the agent want to tickle? Check with counter conditions.
  if (applyTickle == true) {
    // Apply the tickle.
    for (auto &v: vertices) {
      glm::vec2 force = glm::vec2(ofRandom(-5, 5), ofRandom(-5, 5));
      v -> addForce(force, tickleWeight);
    }
    applyTickle = false;
  }
}

glm::vec2 Agent::getCentroid() {
  return mesh.getCentroid();
}

ofMesh& Agent::getMesh() {
  return mesh;
}


// Repulse the vertices constantly
void Agent::repulseBondedVertices() {
  for (auto &v : vertices) {
    auto data = reinterpret_cast<VertexData*>(v->getData());
    if (data->hasInterAgentJoint) {
      data->applyRepulsion = true;
      v->setData(data);
    }
  }
}

void Agent::setTickle(float avgForceWeight) {
  applyTickle = true;
  tickleWeight = avgForceWeight;
}

void Agent::setStretch() {
  applyStretch = true;
}

void Agent::setDesireState(DesireState newState) {
  desireState = newState;
  
  if (desireState == None) {
    applyAttraction = false;
  }
  
  if (desireState == Attraction) {
    applyAttraction = true;
  }
  
  if (desireState == Repulsion) {
    applyRepulsion = true;
  }
}

