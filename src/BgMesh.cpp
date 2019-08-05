#include "BgMesh.h"

void BgMesh::setParams(ofParameterGroup params) {
    bgParams = params;
}

bool BgMesh::isAllocated() {
  return mainFbo.isAllocated();
}

// Setup background
void BgMesh::setup() {
  cout << "Setup is called." << endl; 
  auto rectWidth = bgParams.getInt("Width");
  auto rectHeight = bgParams.getInt("Height");
  
  // Load the background shader.
  shader.load("bg.vert", "bg.frag");
  
  // Allocate bg fbo and clear it for the background.
  bgFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  bgFbo.begin();
    ofClear(ofColor::white);
  bgFbo.end();
  
  // Allocate main fbo in which background is drawn.
  mainFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

  // Create mesh for this background
  createMesh();
}

// Receive agent mesh
void BgMesh::update(std::vector<ofMesh> agentMeshes) {
  // Update the main fbo.
  mainFbo.begin();
    ofClear(0, 0, 0, 0);
      // Background shader that's the meat of the background. 
      shader.begin();
        // Shader needs a fbo (a screen buffer to use the vertices and draw the pixels for)
        shader.setUniform1f("time", (float) ofGetElapsedTimeMillis()/1000);
        shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        bgFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
      shader.end();
  mainFbo.end();
  
  // Empty vector as size of background mesh's vertices.
  std::vector<glm::vec2> offsets;
  offsets.assign(mesh.getVertices().size(), glm::vec2(0, 0));
  for (auto m : agentMeshes) {
    auto vertices = m.getVertices();
    std::vector<glm::vec2> randVertices;
    randVertices.push_back(vertices[vertices.size()/2 -1]);
    for (auto v : randVertices) {
      for (int i = 0; i < mesh.getVertices().size(); i++) {
        auto meshVertex = meshCopy.getVertices()[i];
        offsets[i] = offsets.at(i) + interact(meshVertex, v, i);
      }
    }
  }
  
  // Update each mesh vertex with a displacement.
  for (int i = 0; i < mesh.getVertices().size(); i++) {
    auto newVertex = meshCopy.getVertices()[i] + offsets.at(i);
    mesh.setVertex(i, {newVertex.x, newVertex.y, 0});
  }
}

void BgMesh::draw() {
  mainFbo.getTexture().bind();
  mesh.draw();
  mainFbo.getTexture().unbind();
}

void BgMesh::destroy() {
  bgFbo.clear();
  mainFbo.clear();
}

glm::vec2 BgMesh::interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx) {
  // Get distanceVector of this vertex from the position.
  glm::vec2 distance = centroid - meshVertex;

  // Normalize distance vector.
  glm::vec2 normal = glm::normalize(distance);

  // Calculate length of distance vector.
  int distanceToCentroid = glm::length(distance);

  auto attraction = bgParams.getInt("Attraction");
  auto repulsion = bgParams.getInt("Repulsion");
  // Closer the vertex is, more distortion. Farther the vertex, less is the distortion.
  int displacement = ofMap(distanceToCentroid, 0, 800, attraction, -repulsion, true);
  
  return displacement * normal;
}

void BgMesh::createMesh() {
  // NOTE: Important to clear the mesh or else extra vertices
  // appear.
  mesh.clear();
  meshCopy.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  int rectWidth = bgParams.getInt("Width");
  int rectHeight = bgParams.getInt("Height");
  
  // Rows/Columns
  int numRows = bgFbo.getHeight()/rectHeight;
  int numCols = bgFbo.getWidth()/rectWidth;

  // Mesh size.
  int w = bgFbo.getWidth();
  int h = bgFbo.getHeight();
  
  // Mesh vertices and texture mapping.
  for (int y = 0; y < numRows; y++) {
    for (int x = 0; x < numCols; x++) {
      float ix = w * x / (numCols - 1);
      float iy = h * y / (numRows - 1);
      mesh.addVertex({ix, iy, 0});
      
      // Texture vertices (0 - 1) since textures are normalized.
      float texX = ofMap(ix, 0, bgFbo.getTexture().getWidth(), 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy, 0, bgFbo.getTexture().getHeight(), 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord(glm::vec2(texX, texY));
    }
  }
  
  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < numRows - 1; y++)
  {
      for (int x = 0; x < numCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * numCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * numCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * numCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * numCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * numCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * numCols + (x + 0));
      }
  }
  
  // Deep mesh copy.
  meshCopy = mesh; 
}
