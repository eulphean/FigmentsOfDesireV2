#pragma once
#include "ofMain.h"

class BgMesh {
  public:
    BgMesh() {}
  
    void setParams(ofParameterGroup params);
  
    // Core methods. 
    void setup();
    void update(std::vector<ofMesh> meshes);
    void draw(bool debug);
    bool isAllocated();
    void destroy();
  
  private:
    void createMesh();
    glm::vec2 interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx);
  
    ofFbo bgFbo;
    ofFbo mainFbo; 
    ofShader shader; 
  
    ofMesh mesh;
    ofMesh meshCopy;
    ofParameterGroup bgParams;
};
