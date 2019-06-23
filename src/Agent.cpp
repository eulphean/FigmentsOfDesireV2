#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps, string fileName) {
  // Prepare the agent's texture.
  readFile(fileName);
  assignMessages(agentProps.meshSize);
  
  // Initialize the iterator.
  curMsg = messages.begin(); // Need the message to draw
  
  createTexture(agentProps.meshSize);
  
  // Prepare agent's mesh.
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
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
  // Use box2d circle to update the mesh.
  updateMesh();
  
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
    ofPushMatrix();
      ofTranslate(centroid);
      ofNoFill();
      ofSetColor(ofColor::white);
      ofDrawCircle(0, 0, desireRadius);
    ofPopMatrix();
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

void Agent::readFile(string fileName) {
  auto buffer = ofBufferFromFile(fileName);
  auto lines = ofSplitString(buffer.getText(), "\n");
 
  for (auto l: lines) {
    auto i = l.find(":");
    if (i > 0) {
      auto s = l.substr(i+1);
      textMsgs.push_back(s);
    } else {
      auto b = textMsgs.back();
      b = b + "\n" + l; // Append the line to the last value in the vector.
      textMsgs[textMsgs.size()-1] = b;
    }
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

void Agent::assignMessages(ofPoint meshSize) {
  // Create Bogus message circles.
  for (int i = 0; i < numBogusMessages; i++) {
    // Pick a random location on the mesh.
    int w = meshSize.x; int h = meshSize.y;
    auto x = ofRandom(0, w); auto y = ofRandom(0, h);
    
    // Pick a random color for the message (anything except the background)
    int idx = ofRandom(1, palette.size());
    ofColor c = ofColor(palette.at(idx));
    
    // Pick a random size (TOOD: Based off on the length of the message).
    int size = ofRandom(5, 10);
    
    // Create a message.
    Message m = Message(glm::vec2(x, y), c, size, "~");
    messages.push_back(m);
  }
}

void Agent::createTexture(ofPoint meshSize) {
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
  secondFbo.allocate(meshSize.x, meshSize.y, GL_RGBA);
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
      v->addRepulsionForce(pos.x, pos.y, vertexRepulsionWeight * 18);
      
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
    
    auto d = glm::distance(partner->getCentroid(), getCentroid()); // Distance till the centroid
    float newWeight = ofMap(d, desireRadius * 3, 0, attractionWeight, 0, true);
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

void Agent::createMesh(AgentProperties agentProps) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshDimensions.x;
  int nCols = agentProps.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = agentProps.meshSize.x;
  int h = agentProps.meshSize.y;
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = agentProps.meshOrigin.x + w * x / (nCols - 1);
      float iy = agentProps.meshOrigin.y + h * y / (nRows - 1);
     
      mesh.addVertex({ix, iy, 0});
      
      // Height and Width of the texture is same as the width/height sent in via agentProps
      float texX = ofMap(ix - agentProps.meshOrigin.x, 0, w, 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy - agentProps.meshOrigin.y, 0, h, 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord({texX, texY});
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void Agent::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius); // ofRandom(3, agentProps.vertexRadius)
    vertex -> setFixedRotation(true);
    vertex -> setData(new VertexData(this)); // Data is passed with current Agent's pointer
    vertices.push_back(vertex);
  }
  
  int meshRows = agentProps.meshDimensions.x;
  int meshColumns = agentProps.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void Agent::updateMesh() {
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
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

