#pragma once
#include "ofMain.h"
#include "ofxFilterLibrary.h"
#include "ofxPostProcessing.h"

class BgMesh {
  public:
    BgMesh() {
      filter = new PerlinPixellationFilter(ofGetWidth(), ofGetHeight(), 45.f);
      post.init(ofGetWidth(), ofGetHeight());
      post.createPass<FxaaPass>()->setEnabled(true);
      //post.createPass<DofAltPass>()->setEnabled(true);
      post.createPass<DofPass>()->setEnabled(true);
    }
  
    void setParams(ofParameterGroup params);
    void createBg();
    void update(std::vector<glm::vec2> centroids);
    void updateWithVertices(std::vector<ofMesh> meshes);
    void draw();
  
  private:
    void createMesh();
    glm::vec2 interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx);
    
    ofFbo bgImage;
    ofFbo testImage; 
    ofMesh mesh;
    ofMesh meshCopy;
    ofParameterGroup bgParams;
  
    AbstractFilter * filter;
    ofxPostProcessing post; 
};
