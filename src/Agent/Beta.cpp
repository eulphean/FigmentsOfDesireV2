#include "Beta.h"

Beta::Beta(ofxBox2d &box2d, BetaAgentProperties agentProps) {
  palette = { ofColor::fromHex(0xFFBE0B),
              ofColor::fromHex(0xFB5607),
              ofColor::fromHex(0xFF006E),
              ofColor::fromHex(0x8338EC),
              ofColor::fromHex(0x3A86FF),
              ofColor::fromHex(0xF7FFAB),
              ofColor::fromHex(0xC0F60B)
  };
  
  visibilityRadius = agentProps.meshRadius; 
  
  updateWeights(agentProps);
  
  // Create mesh and soft body here.
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  setup(box2d, agentProps.textureSize);
}

void Beta::createMesh(BetaAgentProperties agentProps) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
  
  // Face is the texture that gets mapped onto the circular mesh.
  ofPoint textureSize = agentProps.textureSize;
  int area = textureSize.x * textureSize.y;
  float texRadius = sqrt(area/PI); // Agent's texture radius.
  float meshRadius = agentProps.meshRadius; // Agent's mesh radius.
  
  // Begin creating the mesh. 
  glm::vec3 meshOrigin = glm::vec3(agentProps.meshOrigin.x, agentProps.meshOrigin.y, 0);
 
  // Center vertex and texture coordinate.
  mesh.addVertex(meshOrigin);
  mesh.addTexCoord(glm::vec2(0.5, 0.5)); // Texture coordinates need to be between 0 and 1 (ofDisableArbTex)
  
  // Calculate # of meshPoints and set the variable to that.
  float meshCircumference = 2 * PI * meshRadius; // circumference of the agent.
  numMeshPoints = meshCircumference / (agentProps.vertexRadius * 2 + agentProps.sideJointOffset); // Total number of points on the boundary
  // TOOD: This meshPoints can be subtracted by 1 for sure here ----> Right now, it's just packed with circles

  // Add vertices around the center to form a circle.
  for(int i = 1; i < numMeshPoints; i++){
    float n = ofMap(i, 1, numMeshPoints-1, 0.0, TWO_PI, true); // Calculate angle at each boundary point.
    float x = cos(n);
    float y = sin(n);
    
    // Boundary vertex and boundary texture coordinate
    mesh.addVertex({meshOrigin.x + (x * meshRadius), meshOrigin.y + y * meshRadius, 0});
    
    // Map the texture coordinates between 0 and 1 (ofDisableArbTex) 
    float texX = ofMap(textureSize.x/2 + (x * texRadius), 0, textureSize.x, 0, 1, true);
    float texY = ofMap(textureSize.y/2 + (y * texRadius), 0, textureSize.y, 0, 1, true);
    mesh.addTexCoord(glm::vec2(texX, texY));
  }
}

void Beta::createSoftBody(ofxBox2d &box2d, BetaAgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  
  // Clear the Box2d bodies to create them again.
  vertices.clear();
  joints.clear();
  
  // Construct soft bodies at all the mesh vertices.
  for (int i = 0; i < mesh.getVertices().size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.y); // bounce, density, friction
    
    if (i == 0) { // This is the 0th vertex.
      vertex->setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius + 2);
    } else {
      vertex->setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius);
    }
    
    // Other properties. 
    vertex->setFixedRotation(true);
    vertex->setData(new VertexData(this)); // Data is passed with current Agent's pointer

    vertices.push_back(vertex);
  }
  
  // Connect center vertex to all the vertices.
  // Start from 1st vertex because the 0th vertex (center) is connected other vertices on the boundary.
  // We go 1 less than the mesh points because last point in the mesh is the same as the second point (after center).
  for(auto i=1; i< vertices.size(); i++) {
    auto joint = std::make_shared<ofxBox2dJoint>();
    joint->setup(box2d.getWorld(), vertices[0] -> body, vertices[i] -> body, agentProps.centerJointPhysics.x, agentProps.centerJointPhysics.y);
    joint->setLength(agentProps.meshRadius);
    joints.push_back(joint);
  }
  
  // Connect joints with each other.
  for(auto i=1; i < vertices.size(); i++) {
    auto joint = std::make_shared<ofxBox2dJoint>();

    // At last index, make a spring back to 0.
    int fromIdx = i; int toIdx = i+1;
    if (i == vertices.size() - 1) {
      toIdx = 1;
    }

    // Note: Outer Joint should have a seperate prop passed in for Azra's joints.
    joint->setup(box2d.getWorld(), vertices[fromIdx] -> body, vertices[toIdx] -> body, agentProps.sideJointPhysics.x, agentProps.sideJointPhysics.y);
  }
}

void Beta::updateMesh() {
 for (int i = 0; i < numMeshPoints; i++) {
    // Get ith circle's position.
    glm::vec2 pos;
   
    if (i == numMeshPoints - 1) {
      pos = vertices[1] -> getPosition();
    } else {
      pos = vertices[i] -> getPosition();
    }
  
    // Update ith mesh vertex's position.
    auto vertex = mesh.getVertices()[i];
    vertex.x = pos.x;
    vertex.y = pos.y;
    mesh.setVertex(i, vertex);
  }
}


void Beta::update(AgentProps alphaProps, AgentProps betaProps) {
  // Update local mesh.
  updateMesh();
  
  updateWeights(betaProps);
  
  // Call base class's update method.
  Agent::update(alphaProps, betaProps);
}

void Beta::updateWeights(AgentProps agentProps) {
  maxStretchWeight = agentProps.stretchWeight;
  maxRepulsionWeight = agentProps.repulsionWeight;
  maxAttractionWeight = agentProps.attractionWeight; // Can this be changed when the other agent is trying to attack me?
  maxTickleWeight = agentProps.tickleWeight;
  maxVelocity = agentProps.velocity;
}
