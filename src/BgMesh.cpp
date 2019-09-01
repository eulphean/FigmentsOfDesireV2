#include "BgMesh.h"

bool BgMesh::isAllocated() {
  return mainFbo.isAllocated();
}

// Setup background
void BgMesh::setup() {
  // Load the background shader.
  shader.load("background/bg.vert", "background/bg.frag");
  ofLoadImage(bgTex, "bg.jpg");
  
  // Allocate bg fbo and clear it for the background.
  bgFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  bgFbo.begin();
    ofClear(ofColor::white);
  bgFbo.end();
  
  // Allocate main fbo in which background is drawn.
  mainFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  
  // Setup the main fbo.
  mainFbo.begin();
    ofClear(0, 0, 0, 0);
      // Background shader that's the meat of the background.
      shader.begin();
        // Shader needs a fbo (a screen buffer to use the vertices and draw the pixels for)
        shader.setUniform1f("time", (float) ofGetElapsedTimeMillis()/1000);
        shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        shader.setUniform1f("isOccupied", false);
        bgFbo.draw(0, 0);
      shader.end();
  mainFbo.end();
}

// Receive agent mesh
void BgMesh::update(bool skipBgUpdate, bool isOccupied) {
  if (!skipBgUpdate) {
    mainFbo.begin();
      // Background shader that's the meat of the background.
      shader.begin();
        // Shader needs a fbo (a screen buffer to use the vertices and draw the pixels for)
         shader.setUniform1f("time", (float) ofGetElapsedTimeMillis());
        shader.setUniform1f("isOccupied", isOccupied); 
        bgFbo.draw(0, 0);
      shader.end();
    mainFbo.end();
  }
}


void BgMesh::draw(bool debug) {
  if (!debug) {
    mainFbo.getTexture().draw(0, 0);
  }
}

void BgMesh::destroy() {
  bgFbo.clear();
  mainFbo.clear();
}
