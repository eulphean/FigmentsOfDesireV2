#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  agentProps.vertexRadius = 7; 
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x540D6E),
              ofColor::fromHex(0x982A41),
              ofColor::fromHex(0xFFEEB9),
              ofColor::fromHex(0x3BCEAC),
              ofColor::fromHex(0x0EAD69),
              ofColor::fromHex(0xFF4A4A),
              ofColor::fromHex(0x0BF6CD)
  };
  
  this->numBogusMessages = 100;
  
  // Force weight for body actions. This is heavier, so more weight.
  maxStretchWeight = 1.5;
  stretchWeight = 0;
  
  vertexRepulsionWeight = 2.5;
  repulsionWeight = 0;
  attractionWeight = 1.5; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 20;
  
  // Post process filters.
  //filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
  //filter = new EmbossFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
  filter = new GaussianBlurFilter(agentProps.meshSize.x, agentProps.meshSize.y, 7.f, 1.f);
  
  filterChain = new FilterChain(agentProps.meshSize.x, agentProps.meshSize.y, "Chain");
  filterChain->addFilter(new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 15.f));
//  filterChain->addFilter(new LookupFilter(agentProps.meshSize.x, agentProps.meshSize.y, "img/lookup_amatorka.png"));
//  filterChain->addFilter(new PoissonBlendFilter("img/grid.jpg", agentProps.meshSize.x, agentProps.meshSize.y, 0.6, 2));
  
  
  setup(box2d, agentProps);
}
