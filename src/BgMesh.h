#pragma once
#include "ofMain.h"
#include "ofxPostProcessing.h"

class BgMesh {
  public:
    BgMesh() {
      post.init(ofGetWidth(), ofGetHeight());
      post.createPass<FxaaPass>()->setEnabled(true);
      //post.createPass<DofAltPass>()->setEnabled(true);
      post.createPass<DofPass>()->setEnabled(true);
    }
  
    void setParams(ofParameterGroup params);
  
    // Core methods. 
    void setup();
    void update(std::vector<ofMesh> meshes);
    void draw();
  
  private:
    void createMesh();
    glm::vec2 interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx);
  
    ofFbo bgFbo;
    ofFbo mainFbo; 
    ofShader shader; 
  
    ofMesh mesh;
    ofMesh meshCopy;
    ofParameterGroup bgParams;

    ofxPostProcessing post; 
};
