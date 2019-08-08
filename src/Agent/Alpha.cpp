#include "Alpha.h"

Alpha::Alpha(ofxBox2d &box2d, AlphaAgentProperties agentProps) {  
  // Assign a color palette
  palette = { ofColor::fromHex(0x540D6E),
              ofColor::fromHex(0x982A41),
              ofColor::fromHex(0xFFEEB9),
              ofColor::fromHex(0x3BCEAC),
              ofColor::fromHex(0x0EAD69),
              ofColor::fromHex(0xFF4A4A),
              ofColor::fromHex(0x0BF6CD)
  };
  
  // Force weight for body actions. This is heavier, so more weight.
  maxStretchWeight = 1.5;
  stretchWeight = 0;
  
  vertexRepulsionWeight = 2.5;
  repulsionWeight = 0;
  attractionWeight = 1.0; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 15;
  
  // Create Mesh
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  // Let the parent class setup the rest of the Agent (especially Texture)
  setup(box2d, agentProps.textureSize);
}

void Alpha::createMesh(AlphaAgentProperties agentProps) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshRowsColumns.x;
  int nCols = agentProps.meshRowsColumns.y;
  
  // Width, height for the mesh.
  ofPoint meshSize = agentProps.meshSize; // Width, Height (mesh)
  ofPoint textureSize = agentProps.textureSize; // Width, Height (texture)
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = agentProps.meshOrigin.x + meshSize.x * x / (nCols - 1); float tx = textureSize.x * x / (nCols - 1);
      float iy = agentProps.meshOrigin.y + meshSize.y * y / (nRows - 1); float ty = textureSize.y * y / (nRows - 1);
     
      mesh.addVertex({ix, iy, 0});
      
      // Height and Width of the texture is same as the width/height sent in via agentProps
      float texX = ofMap(tx, 0, textureSize.x, 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(ty, 0, textureSize.y, 0, 1, true); // Map the calculated y coordinate from 0 - 1
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

void Alpha::createSoftBody(ofxBox2d &box2d, AlphaAgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex->setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex->setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius);
    vertex->setFixedRotation(true);
    vertex->setData(new VertexData(this)); // Data is passed with current Agent's pointer
    vertices.push_back(vertex);
  }
  
  int meshRows = agentProps.meshRowsColumns.x;
  int meshColumns = agentProps.meshRowsColumns.y;
  
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

void Alpha::updateMesh() {
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

void Alpha::update() {
  // Update local mesh. 
  updateMesh();
  
  // Call base class's update method.
  Agent::update();
}
