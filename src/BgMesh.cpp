#include "BgMesh.h"

void BgMesh::setParams(ofParameterGroup params) {
    bgParams = params;
}

// Setup background
void BgMesh::createBg() {
  auto rectWidth = bgParams.getInt("Width");
  auto rectHeight = bgParams.getInt("Height");
  
  bgImage.allocate(ofGetWidth()*2, ofGetHeight()*2, GL_RGBA);
  bgImage.begin();
    ofClear(0, 0, 0, 0);
  int numRows = bgImage.getHeight()/rectHeight;
  int numCols = bgImage.getWidth()/rectWidth;
  
    int a = 0;
    for (int y = 0; y < numRows; y++) {
      for (int x = 0; x < numCols; x++) {
        if (a % 2 == 0) {
          ofSetColor(ofColor::fromHex(0xDBDBDB));
        } else {
          ofSetColor(ofColor::fromHex(0x525151));
        }
        ofPushMatrix();
        ofTranslate(x * rectWidth, y * rectHeight);
          ofDrawRectangle(0, 0, rectWidth, rectHeight) ;
        ofPopMatrix();
        a++;
      }

    a++;
  }
  
  bgImage.end();
  
  testImage.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  testImage.begin();
    ofClear(0, 0, 0, 0);
    post.begin();
    filter->begin();
      bgImage.getTexture().drawSubsection(0, 0, ofGetWidth(), ofGetHeight(), 0, 0);
    filter->end();
    post.end();
  testImage.end();
  
  //testImage.getTexture().enableMipmap();
  //testImage.getTexture().setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
  
  // Create mesh for this background
  createMesh();
}

// Receive agent mesh
void BgMesh::updateWithVertices(std::vector<ofMesh> agentMeshes) {
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

void BgMesh::update(std::vector<glm::vec2> centroids) {
  // Calculate net displacement due to each centroid and store in offsets.
  std::vector<glm::vec2> offsets;
  offsets.assign(mesh.getVertices().size(), glm::vec2(0, 0));
  for (auto &c : centroids) {
    for (int i = 0; i < mesh.getVertices().size(); i++) {
      auto meshVertex = meshCopy.getVertices()[i];
      offsets[i] = offsets.at(i) + interact(meshVertex, c, i);
    }
  }
  
  // Update each mesh vertex with a displacement.
  for (int i = 0; i < mesh.getVertices().size(); i++) {
    auto newVertex = meshCopy.getVertices()[i] + offsets.at(i);
    mesh.setVertex(i, {newVertex.x, newVertex.y, 0});
  }
  
  // Set filter parameter
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

void BgMesh::draw() {
  testImage.getTexture().bind();
  mesh.draw();
  testImage.getTexture().unbind();
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
  int numRows = testImage.getHeight()/rectHeight;
  int numCols = testImage.getWidth()/rectWidth;

  // Mesh size.
  int w = testImage.getWidth();
  int h = testImage.getHeight();
  
  // Mesh vertices and texture mapping.
  for (int y = 0; y < numRows; y++) {
    for (int x = 0; x < numCols; x++) {
      float ix = w * x / (numCols - 1);
      float iy = h * y / (numRows - 1);
      mesh.addVertex({ix, iy, 0});
      
      // Texture vertices (0 - 1) since textures are normalized.
      float texX = ofMap(ix, 0, testImage.getTexture().getWidth(), 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy, 0, testImage.getTexture().getHeight(), 0, 1, true); // Map the calculated y coordinate from 0 - 1
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
