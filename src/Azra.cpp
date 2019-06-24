#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  agentProps.vertexRadius = 4.5;
  
  palette = { ofColor::fromHex(0xFFBE0B),
              ofColor::fromHex(0xFB5607),
              ofColor::fromHex(0xFF006E),
              ofColor::fromHex(0x8338EC),
              ofColor::fromHex(0x3A86FF),
              ofColor::fromHex(0xF7FFAB),
              ofColor::fromHex(0xC0F60B)
  };
  
  this->numBogusMessages = 70;
  
  // Force weights for body actions
  maxStretchWeight = 1.0;
  stretchWeight = 0;
  
  vertexRepulsionWeight = 2.5;
  repulsionWeight = 0;
  attractionWeight = 1.0; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 15;
  
  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);

  filterChain = new FilterChain(agentProps.meshSize.x, agentProps.meshSize.y, "Chain");
  filterChain->addFilter(new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 15.f));
  //filterChain->addFilter(new LookupFilter(agentProps.meshSize.x, agentProps.meshSize.y, "img/lookup_miss_etikate.png"));
  //filterChain->addFilter(new PoissonBlendFilter("img/tex.jpg", agentProps.meshSize.x, agentProps.meshSize.y, 0.6, 2));
  //filterChain->addFilter(new PerlinNoiseFilter(2.0));
  
  setup(box2d, agentProps);
}
