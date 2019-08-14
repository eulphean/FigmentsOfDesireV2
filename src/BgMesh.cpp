#include "BgMesh.h"

void BgMesh::setParams(ofParameterGroup params) {
    bgParams = params;
}

bool BgMesh::isAllocated() {
  return mainFbo.isAllocated();
}

// Setup background
void BgMesh::setup() {
  // Load the background shader.
  shader.load("background/bg.vert", "background/bg.frag");
    ofLoadImage(bgTex, "bg.png");
  
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
        bgFbo.draw(0, 0);
      shader.end();
  mainFbo.end();
}

// Receive agent mesh
void BgMesh::update(std::vector<ofMesh> agentMeshes) {
  // Main fbo
  mainFbo.begin();
    // Background shader that's the meat of the background.
    shader.begin();
    // Shader needs a fbo (a screen buffer to use the vertices and draw the pixels for)
    shader.setUniform1f("time", (float) ofGetElapsedTimeMillis()/1000);
    bgFbo.draw(0, 0);
  shader.end();
  mainFbo.end();
}

void BgMesh::draw(bool debug) {
  if (!debug) {
    bgTex.draw(0, 0, ofGetWidth(), ofGetHeight());
    mainFbo.draw(0, 0);
  }
}

void BgMesh::destroy() {
  bgFbo.clear();
  mainFbo.clear();
}
