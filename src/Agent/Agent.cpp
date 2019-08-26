#include "Agent.h"


// ------------------------------ Message --------------------------------------- //

Message::Message(glm::vec2 loc, ofColor col, float s) {
  location = loc;
  color = col;
  size = s;
}

void Message::draw() {
  ofPushMatrix();
    ofTranslate(location);
      ofPushStyle();
        ofColor c = ofColor(color, 250);
        ofSetColor(c);
        ofDrawCircle(0, 0, size);
      ofPopStyle();
  ofPopMatrix();
}

// ------------------------------ Agent --------------------------------------- //

void Agent::setup(ofxBox2d &box2d, ofPoint textureSize) {
  // Setup post-process filter for the texture.
  
  // ACTIVE filter
  filter = new PerlinPixellationFilter(textureSize.x, textureSize.y, 15.f);
  
  // Num messages to inscribe on the texture.
  this->numMessages = 100; 
  
  // This is common for both the agents. 
  createTexture(textureSize);
  curMsg = messages.begin(); // Need the message to draw
  
  // Behaviors. 
  applyStretch = true;
  applyTickle = false;
  applyAttraction = false;
  applyRepulsion = false; 
  
  // Current desire state. 
  desireState = None;
  
  // Some initial force values.
  repulsionWeight = 0;
  stretchWeight = 0;
  attractionWeight = 0;
  coolDown = 0;
  maxCoolDown = 100; // Wait time before the agent actually is ready to take more forces. 
}

void Agent::update(AgentProps alphaProps, AgentProps betaProps) {
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

void Agent::draw(bool showVisibilityRadius, bool showTexture) {
 // Draw the mesh vertices as soft bodies
 // Get the radius of the soft body and change the size.
 auto bodyRadius = this->vertices[0]->getRadius();
 glPointSize(bodyRadius*2);
 
 ofPushStyle();
  ofSetColor(ofColor::red);
  mesh.draw(OF_MESH_POINTS);
 ofPopStyle();
  
  if (secondFbo.isAllocated()) {
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
  }
  
  if (showVisibilityRadius) {
    ofPushStyle();
      ofNoFill();
      ofSetColor(ofColor::yellow);
      ofDrawCircle(getCentroid(), visibilityRadius);
    ofPopStyle();
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

void Agent::createTexture(ofPoint textureSize) {
  // Create spots on the agent's body
  for (int i = 0; i < numMessages; i++) {
    // Pick a random location on the mesh.
    int w = textureSize.x; int h = textureSize.y;
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
  firstFbo.allocate(textureSize.x*2, textureSize.y*2, GL_RGBA);
  firstFbo.begin();
    ofClear(0, 0, 0, 0);
  
    // Assign background.
    int randIdx = ofRandom(palette.size());
    ofColor c = ofColor(palette.at(randIdx), 250);
    ofBackground(c);
  
    // Draw assigned messages.
    for (auto m : messages) {
      m.draw();
    }

  firstFbo.end();
  
  if (firstFbo.isAllocated()) {
    // Create 2nd fbo and draw with filter and postProcessing
    secondFbo.allocate(textureSize.x, textureSize.y, GL_RGBA);
    secondFbo.begin();
      ofClear(0, 0, 0, 0);
      filter->begin();
        firstFbo.getTexture().drawSubsection(0, 0, textureSize.x, textureSize.y, 0, 0);
      filter->end();
    secondFbo.end();
  }
}

void Agent::applyBehaviors()  {
  // ----Current actions/behaviors---
  handleStretch();
  handleRepulsion();
  handleAttraction();
  handleTickle();
  
  // Behavior of individual bodies on the agent (all circles mostly)
  handleVertexBehaviors();
  
  // Should the agent be cooling down?
  if (coolDown > 0) {
    coolDown--;
  }
}

void Agent::handleVertexBehaviors() {
  for (auto &v : vertices) {
    auto data = reinterpret_cast<VertexData*>(v->getData());
    
    // Repulsion.
    if (data->applyRepulsion) {
      auto pos = data->targetPos;
      v->addRepulsionForce(pos.x, pos.y, maxRepulsionWeight);
      
      // Reset repulsion parameter on the vertex.
      data->applyRepulsion = false;
      v->setData(data);
    }
    
    // Attraction.
    if (data->applyAttraction) {
      auto pos = data->targetPos;
      v->addAttractionPoint({pos.x, pos.y}, maxAttractionWeight);
      
      // Reset repulsion parameter on the vertex.
      data->applyAttraction = false;
      v->setData(data);
    }
  }
}

// Unimplemented.
void Agent::handleRepulsion() {
  if (applyRepulsion) {
    repulsionWeight = ofLerp(repulsionWeight, maxRepulsionWeight, 0.1);
    for (auto &v : vertices) {
      auto data = reinterpret_cast<VertexData*>(v->getData());
      if (!data->hasInterAgentJoint) {
         v->addRepulsionForce(targetPos.x, targetPos.y, repulsionWeight);
      }
    }
    
    applyRepulsion = false;
    
    if (maxRepulsionWeight-repulsionWeight < maxRepulsionWeight/2) {
      repulsionWeight = 0;
    }
  }
}

// Attraction.
void Agent::handleAttraction() {
  // Find the closest vertex to the targetPos.
  if (applyAttraction && coolDown == 0) {
      float minD = 9999; int minIdx;
      for (int idx = 0; idx < vertices.size(); idx++) {
        auto v = vertices[idx];
        auto p = glm::vec2(v->getPosition().x, v->getPosition().y);
        auto d = glm::distance(p, targetPos);
        if (d < minD) {
          minD = d; minIdx = idx;
        }
      }
    
      // This weight is 100 times less than the maxAttraction weight because it
      // acts successively on only one vertex. Also, this force is lerped so the
      // creature has a stretch/bacteria like feeling. The force is still applied
      // on the closest vertex from the person. 
      float newMaxWeight = maxAttractionWeight/100;
      attractionWeight = ofLerp(attractionWeight, newMaxWeight, 0.01);
      vertices[minIdx]->addAttractionPoint(targetPos, attractionWeight);
      if (newMaxWeight - attractionWeight <= 0.01) {
        applyAttraction = false;
        attractionWeight = 0;
        coolDown = maxCoolDown;
      }
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
//        v->setRotation(ofRandom(150));
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
      glm::vec2 force = glm::vec2(ofRandom(-2, 2), ofRandom(-2, 2));
      v -> addForce(force, maxTickleWeight);
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

void Agent::tickle() {
  applyTickle = true;
}

void Agent::stretch() {
  applyStretch = true;
}

void Agent::setDesireState(DesireState newState, glm::vec2 newPos) {
  desireState = newState;
  targetPos = newPos;
  
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

void Agent::assignIndices(ofPoint textureSize) {
//  // Store the corner indices in this array to access it when applying forces.
//  auto rows = agentProps.meshDimensions.x; auto cols = agentProps.meshDimensions.y;
//
//  // Corners
//  cornerIndices[0] = 0; cornerIndices[1] = (cols-1) + 0 * (cols-1);
//  cornerIndices[2] = 0 + (rows-1) * (cols-1); cornerIndices[3] = (cols-1) + (rows-1) * (cols-1);
//
//  // Boundaries.
//
//  // TOP
//  int x; int y = 0;
//  for (x = 0; x < cols-1; x++) {
//    int idx = x + y * (cols-1);
//    boundaryIndices.push_back(idx);
//  }
//
//  // BOTTOM
//  y = rows-1;
//  for (x = 0; x < cols-1; x++) {
//    int idx = x + y * (cols-1);
//    boundaryIndices.push_back(idx);
//  }
//
//  // LEFT
//  x = 0;
//  for (y = 0; y < rows-1; y++) {
//    int idx = x + y * (cols-1);
//    boundaryIndices.push_back(idx);
//  }
//
//  // RIGHT
//  x = cols-1;
//  for (y = 0; y < rows-1; y++) {
//    int idx = x + y * (cols-1);
//    boundaryIndices.push_back(idx);
//  }
}
