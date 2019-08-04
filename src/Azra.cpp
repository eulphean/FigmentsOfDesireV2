#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  agentProps.vertexRadius = 8;
  
  palette = { ofColor::fromHex(0xFFBE0B),
              ofColor::fromHex(0xFB5607),
              ofColor::fromHex(0xFF006E),
              ofColor::fromHex(0x8338EC),
              ofColor::fromHex(0x3A86FF),
              ofColor::fromHex(0xF7FFAB),
              ofColor::fromHex(0xC0F60B)
  };
  
  this->numBogusMessages = 100;
  
  // Force weights for body actions
  maxStretchWeight = 1.0;
  stretchWeight = 0;
  
  vertexRepulsionWeight = 2.5;
  repulsionWeight = 0;
  attractionWeight = 0.5; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 15;
  
  // Post process filters.
  // DEAD filter
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);

  // ACTIVE filter
  filterChain = new FilterChain(agentProps.meshSize.x, agentProps.meshSize.y, "Chain");
  filterChain->addFilter(new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 15.f));
  
  // Create mesh and soft body here.
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  setup(box2d, agentProps);
}

void Azra::createMesh(AgentProperties agentProps) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
  
  // Face is the texture that gets mapped onto the circular mesh.
  float faceWidth = agentProps.meshSize.x;
  float faceHeight = agentProps.meshSize.y;
  float faceArea = faceWidth * faceHeight;
  glm::vec2 faceCenter = glm::vec2(faceWidth/2, faceHeight/2);
  
  // Calculate face radius which we will use to define the mesh radius.
  faceRadius = sqrt(faceArea/PI);
  
  // Begin creating the mesh. 
  glm::vec3 meshOrigin = glm::vec3(agentProps.meshOrigin.x, agentProps.meshOrigin.y, 0);
 
  // Center vertex and texture coordinate.
  mesh.addVertex(meshOrigin);
  mesh.addTexCoord(glm::vec2(0.5, 0.5)); // Texture coordinates need to be between 0 and 1 (ofDisableArbTex)
  
  // Ratio between the texture and the circular mesh.
  float sizeRatio = faceWidth/(faceRadius*2);
  
  // Calculate # of meshPoints and set the variable to that.
  faceCircumference = 2 * PI * faceRadius; // circumference
  meshPoints = faceCircumference / (agentProps.vertexRadius * 2); // Total number of points on the boundary

  // Add vertices around the center to form a circle.
  for(int i = 1; i < meshPoints; i++){
    float n = ofMap(i, 1, meshPoints-1, 0.0, TWO_PI, true); // Calculate angle at each boundary point.
    float x = cos(n);
    float y = sin(n);
    
    // Boundary vertex and boundary texture coordinate
    mesh.addVertex({meshOrigin.x + (x * faceRadius), meshOrigin.y + y * faceRadius, 0});
    
    // Map the texture coordinates between 0 and 1 (ofDisableArbTex) 
    float texX = ofMap(faceWidth/2 + (x * faceRadius), 0, faceWidth, 0, 1, true);
    float texY = ofMap(faceHeight/2 + (y * faceRadius), 0, faceHeight, 0, 1, true);
    mesh.addTexCoord(glm::vec2(texX, texY));
  }
}

void Azra::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  
  // Clear the Box2d bodies to create them again.
  vertices.clear();
  joints.clear();
  
  // We must have the latest value of meshPoints right now.
  // We want to make sure we create a mesh before creating Box2D springs.
  
  // Construct soft bodies at all the mesh vertices.
  for (int i = 0; i < mesh.getVertices().size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.y); // bounce, density, friction
    
    if (i == 0) { // This is the 0th vertex.
      vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius + 2);
    } else {
      vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius);
    }
    
    // Other properties. 
    vertex -> setFixedRotation(true);
    vertex -> setData(new VertexData(this)); // Data is passed with current Agent's pointer

    vertices.push_back(vertex);
  }
  
  // Note: Inner Joint should have a seperate prop passed in for Azra's joints.
  // Connect center vertex to all the vertices.
  // Start from 1st vertex because the 0th vertex (center) is connected other vertices on the boundary.
  // We go 1 less than the mesh points because last point in the mesh is the same as the second point (after center).
  for(auto i=1; i< vertices.size(); i++) {
    auto joint = std::make_shared<ofxBox2dJoint>();
    joint -> setup(box2d.getWorld(), vertices[0] -> body, vertices[i] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
    joint->setLength(faceRadius);
    joints.push_back(joint);
  }
  
  // Connect joints with each other.
  // We go 1 less than the mesh points because last point in the mesh is the
  // same as the second point (after center).
  float totalJointLength = faceCircumference / softJointLength;
  float length = totalJointLength / meshPoints;
  for(auto i=1; i < vertices.size(); i++) {
    auto joint = std::make_shared<ofxBox2dJoint>();

    // At last index, make a spring back to 0.
    int fromIdx = i; int toIdx = i+1;
    if (i == vertices.size() - 1) {
      toIdx = 1;
    }

    // Note: Outer Joint should have a seperate prop passed in for Azra's joints.
    joint -> setup(box2d.getWorld(), vertices[fromIdx] -> body, vertices[toIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
    joints.push_back(joint);
  }
}

void Azra::updateMesh() {
 for (int i = 0; i < meshPoints; i++) {
    // Get ith circle's position.
    glm::vec2 pos;
   
    if (i == meshPoints - 1) {
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


void Azra::update() {
  // Update local mesh.
  updateMesh();
  
  // Call base class's update method.
  Agent::update();
}
