#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Setup OSC
  receiver.setup(PORT);
  //ofHideCursor();
  
  debugFont.load("opensansbond.ttf", 30);
  
  ofBackground(ofColor::fromHex(0x2E2F2D));
  ofSetCircleResolution(20);
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();
  
  box2d.init();
  box2d.setGravity(0, 0.0);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  debug = false;
  stopEverything = false; 
  showTexture = true;
  
  // Bounds
  bounds.x = -20; bounds.y = -20;
  bounds.width = ofGetWidth() + (-1) * bounds.x * 2; bounds.height = ofGetHeight() + (-1) * 2 * bounds.y;
  box2d.createBounds(bounds);
  
  enableSound = true;
  
  // Instantiate Midi.
  Midi::instance().setup();
  
  
  // Store params and create background. 
  bg.setParams(bgParams);
  bg.createBg();
  
  shouldBond = false; 
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence.
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  // Based on the current state of desire, what should the vertices do if they hit each other
  // How do they effect each other?
  if (agents.size() > 0) {
    if(e.a != NULL && e.b != NULL) {
      if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle
          && e.a->GetBody() && e.b->GetBody()) {
        // Extract Agent pointers.
        Agent* agentA = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData())->agent;
        Agent* agentB = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData())->agent;
        
        // DEFINE INDIVIDUAL VERTEX BEHAVIORS.
        if (agentA != agentB && agentA != NULL && agentB != NULL) {
          // Collect datas
          auto dataA = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
          auto dataB = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
          
          // Update positions for repelling.
          auto pos = getBodyPosition(e.b->GetBody());
          dataA->targetPos = pos;

          pos = getBodyPosition(e.a->GetBody());
          dataB->targetPos = pos;
          
          // Desire state is NONE! Repel the vertices from each
          // other. 
          if (agentA->desireState == None) {
            if (ofRandom(1) < 0.5) {
              dataA->applyRepulsion = true;
              e.a->GetBody()->SetUserData(dataA);
            } else {
              dataA->applyAttraction = true;
              e.a->GetBody()->SetUserData(dataA);
            }
          }
          
          if (agentB->desireState == None) {
            if (ofRandom(1) < 0.5) {
              dataA->applyRepulsion = true;
              e.a->GetBody()->SetUserData(dataA);
            } else {
              dataB->applyAttraction = true;
              e.b->GetBody()->SetUserData(dataB);
            }
          }
          
          // Desire state is ATTRACTION!
          // Repel the other agent.
          if (agentA->desireState == Attraction) {
             // Attract A's vertices
             if (!dataA->hasInterAgentJoint) {
               dataA->applyAttraction = true;
               e.a->GetBody()->SetUserData(dataA);
            }
            
            // Repel B's vertices
            if (!dataB->hasInterAgentJoint) {
              dataB->applyRepulsion = true;
              e.b->GetBody()->SetUserData(dataB);
            }
          
            // Reset agent state to None on collision.
            agentA->setDesireState(None);
          }
          
          if (agentB->desireState == Attraction) {
            // Attract B's vertice
             if (!dataB->hasInterAgentJoint) {
              dataB->applyAttraction = true;
              e.b->GetBody()->SetUserData(dataB);
            }
            
            // Repel A's vertices
            if (!dataA->hasInterAgentJoint) {
               dataA->applyRepulsion = true;
               e.a->GetBody()->SetUserData(dataA);
            }
            
            // Reset agent state to None on collision.
            agentB->setDesireState(None);
          }

          // Should the agents be evaluated for bonding?
          if (shouldBond) {
            evaluateBonding(e.a->GetBody(), e.b->GetBody(), agentA, agentB);
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
  processOsc();
  
  // Update super agents
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, memories, shouldBond, maxJointForce);
    return sa.shouldRemove;
  });
  
  // GUI props.
  updateAgentProps();
  
  std::vector<ofMesh> meshes;
  // Update agents
  for (auto &a : agents) {
    a -> update();
    meshes.push_back(a->getMesh());
  }
  
  // Create super agents based on collision bodies.
  createSuperAgents();
  
  // Update background
  bg.updateWithVertices(meshes);
  
  // Update memories.
  ofRemove(memories, [&](Memory &m) {
    m.update();
    return m.shouldRemove;
  });
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Draw background.
  if (!debug) {
   bg.draw();
  }
  
  // Draw box2d bounds.
  ofPushStyle();
    ofSetColor(ofColor::fromHex(0x341517));
    ofFill();
    ofDrawRectangle(0, 0, bounds.x, ofGetHeight());
    ofDrawRectangle(0, ofGetHeight() - bounds.x, ofGetWidth(), bounds.x);
    ofDrawRectangle(ofGetWidth()-bounds.x, 0, bounds.x, ofGetHeight());
  ofPopStyle();

  // Draw all what's inside the super agents.
  for (auto sa: superAgents) {
    sa.draw();
  }
  
  // Draw Agent is the virtual method for derived class. 
  for (auto a: agents) {
    a -> draw(debug, showTexture);
  }
  
  // Draw memories
  for (auto m : memories) {
    m.draw();
  }

  // Health parameters
  if (hideGui) {
     ofDrawBitmapString(ofGetFrameRate(), 300, 50);
    gui.draw();
  }
}

void ofApp::processOsc() {
  while(receiver.hasWaitingMessages()){
    // get the next message
    ofxOscMessage m;
    receiver.getNextMessage(m);
    
    // ABLETON messages.
    // Process these OSC messages and based on which agent this needs to be delivered,
    if(m.getAddress() == "/Attract"){
      float val = m.getArgAsFloat(0);
      // Pick a random figment and set applyAttraction to true
      Agent *curAgent;
      auto p = ofRandom(1);
      if (p < 0.5) {
        curAgent = agents[0];
      } else {
        curAgent = agents[1];
      }
      
      // Enable attraction in the figment.
      curAgent->setDesireState(Attraction);
    }
    
    if(m.getAddress() == "/Repel"){
      float val = m.getArgAsFloat(0);
      
      for (auto &a: agents) {
        a->setDesireState(Repulsion);
      }
    }
    
    if(m.getAddress() == "/Stretch") {
      float val = m.getArgAsFloat(0);
      
      // Populate random agents
      std::vector<Agent *> curAgents;
      auto p = ofRandom(1);
        if (agents.size()>0) {
        if (p < 0.33) {
          curAgents.push_back(agents[0]); // Agent A
        } else if (p < 0.66){
          curAgents.push_back(agents[1]); // Agent B
        } else { // Both agents.
          curAgents.push_back(agents[0]);
          curAgents.push_back(agents[1]);
        }
      }
      
      
      // Enable stretch in the figment. 
      for (auto &a : curAgents) {
        if (a->desireState != Repulsion) {
          a->setStretch();
        }
      }
    }
    
    // STATE CHANGER!
    if(m.getAddress() == "/Melody"){
      float val = m.getArgAsFloat(0);
      shouldBond = (val > 0); // At 1, don't bond anymore
    }
    
// ------------------ GUI OSC Messages -----------------------
    if(m.getAddress() == "/clear"){
      float val = m.getArgAsFloat(0);
      clearScreen();
    }
    
    if(m.getAddress() == "/new"){
      float val = m.getArgAsFloat(0);
      createAgents();
    }
    
    if(m.getAddress() == "/leftBack"){
      float val = m.getArgAsFloat(0);
      Midi::instance().sendMidiControlChangeRotary(0, val);
    }
    
    if(m.getAddress() == "/leftFront"){
      float val = m.getArgAsFloat(0);
      Midi::instance().sendMidiControlChangeRotary(1, val);
    }
    
    if(m.getAddress() == "/rightBack"){
      float val = m.getArgAsFloat(0);
       Midi::instance().sendMidiControlChangeRotary(2, val);
    }
    
    if(m.getAddress() == "/rightFront"){
      float val = m.getArgAsFloat(0);
        Midi::instance().sendMidiControlChangeRotary(3, val);
    }
    
    if(m.getAddress() == "/rain"){
      float val = m.getArgAsFloat(0);
       Midi::instance().sendMidiControlChangeRotary(4, val);
    }
    
    if(m.getAddress() == "/rightBackMix"){
      float val = m.getArgAsFloat(0);
       Midi::instance().sendMidiControlChangeRotary(5, val);
    }
    
    if(m.getAddress() == "/leftFrontMix"){
      float val = m.getArgAsFloat(0);
       Midi::instance().sendMidiControlChangeRotary(6, val);
    }
  }
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
}

void ofApp::createAgents() {
  // Create Amay & Azra
  Amay *a = new Amay(box2d, agentProps);
  Azra *b = new Azra(box2d, agentProps);
  
  // Set partners
  a->partner = b;
  b->partner = a;
  
  // Push agents in the array.
  agents.push_back(a);
  agents.push_back(b);
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("Inter Mesh Settings");
  
    // Mesh parameters.
    meshParams.setName("Mesh Params");
    meshParams.add(meshRows.set("Mesh Rows", 5, 5, 100)); // Add the current value
    meshParams.add(meshColumns.set("Mesh Columns", 5, 5, 100));
    meshParams.add(meshWidth.set("Mesh Width", 100, 10, ofGetWidth()));
    meshParams.add(meshHeight.set("Mesh Height", 100, 10, ofGetHeight()));
  
    // Vertex parameters
    vertexParams.setName("Vertex Params");
    vertexParams.add(vertexRadius.set("Vertex Radius", 6, 1, 30));
    vertexParams.add(vertexDensity.set("Vertex Density", 1, 0, 5));
    vertexParams.add(vertexBounce.set("Vertex Bounce", 0.3, 0, 1));
    vertexParams.add(vertexFriction.set("Vertex Friction", 1, 0, 1));
  
    // Joint parameters
    jointParams.setName("Joint Params");
    jointParams.add(jointFrequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    jointParams.add(jointDamping.set("Joint Damping", 1.0f, 0.0f, 5.0f));
  
    // InterAgentJoint parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(frequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    interAgentJointParams.add(damping.set("Joint Damping", 1.0f, 0.0f, 10.0f));
    interAgentJointParams.add(maxJointForce.set("Max Joint Force", 6.f, 1.f, 100.0f));
  
    // Background group
    bgParams.setName("Background Params");
    bgParams.add(rectWidth.set("Width", 20, 10, 50));
    bgParams.add(rectHeight.set("Height", 20, 10, 50));
    bgParams.add(attraction.set("Attraction", 20, -200, 200));
    bgParams.add(repulsion.set("Repulsion", -20, -200, 200));
    bgParams.add(shaderScale.set("Scale", 1.f, 0.f, 10.f));
    rectWidth.addListener(this, &ofApp::widthChanged);
    rectHeight.addListener(this, &ofApp::heightChanged);
    attraction.addListener(this, &ofApp::updateForce);
    repulsion.addListener(this, &ofApp::updateForce);
    shaderScale.addListener(this, &ofApp::updateParams);
  
    settings.add(meshParams);
    settings.add(vertexParams);
    settings.add(jointParams);
    settings.add(interAgentJointParams);
    settings.add(bgParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::clearScreen() {
  // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
    // middle of a step function. Disabling and renabling the events work as a good solution for now.
  box2d.disableEvents();
  collidingBodies.clear();

  // Clear SuperAgents
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  // Clean agents
  for (auto &a : agents) {
    a -> clean(box2d);
    delete a;
  }
  agents.clear();

  box2d.enableEvents();
}

void ofApp::removeUnbonded() {
  ofRemove(agents, [&](Agent *a) {
//    if (a->getPartner() == NULL) {
//      a->clean(box2d);
//      return true;
//    }
    
    return false;
  });
}

void ofApp::removeJoints() {
  box2d.disableEvents();

  // Clear superAgents only
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  superAgents.clear();
  box2d.enableEvents();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){  
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgents(); 
  }
  
  if (key == 'c') {
    clearScreen();
  }
  
  if (key == 'j') {
    removeJoints();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a -> setTickle(1.0);
    }
  }
  
  if (key == 's') {
    enableSound = !enableSound;
  }
  
  if (key == ' ') {
    stopEverything = !stopEverything;
  }
  
  if (key == 't') {
    showTexture = !showTexture; 
  }
}

void ofApp::exit() {
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
}

// Massive important function that determines when the 2 bodies actually bond.
void ofApp::evaluateBonding(b2Body *bodyA, b2Body *bodyB, Agent *agentA, Agent *agentB) {
  collidingBodies.clear();
  
  // Vertex level checks. Is this vertex bonded to anything except itself?
  bool a = canVertexBond(bodyA, agentA);
  bool b = canVertexBond(bodyB, agentB);
  if (a && b) {
    // Prepare for bond.
    collidingBodies.push_back(bodyA);
    collidingBodies.push_back(bodyB);
  }
}

bool ofApp::canVertexBond(b2Body* body, Agent *curAgent) {
  // If it joins anything except itself, then it cannot join.
  auto curEdge = body->GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge) {
    // Other agent that this joint is joined to.
    auto data = reinterpret_cast<VertexData*>(curEdge->other->GetUserData());
    if (data != NULL) {
      auto otherAgent = data->agent;
      if (otherAgent != curAgent) {
        return false;
      }
    }
    curEdge = curEdge->next;
  }

  return true;
}

void ofApp::widthChanged (int & newWidth) {
  // New background
  bg.setParams(bgParams);
  bg.createBg();
}

void ofApp::heightChanged (int & newHeight) {
  // New background
  bg.setParams(bgParams);
  bg.createBg();
}

void ofApp::updateParams(float & newVal) {
  bg.setParams(bgParams);
}

void ofApp::updateForce(int & newVal) {
  bg.setParams(bgParams);
}

glm::vec2 ofApp::getBodyPosition(b2Body* body) {
  auto xf = body->GetTransform();
  b2Vec2 pos      = body->GetLocalCenter();
  b2Vec2 b2Center = b2Mul(xf, pos);
  auto p = worldPtToscreenPt(b2Center);
  return glm::vec2(p.x, p.y);
}

void ofApp::createSuperAgents() {
  // Joint creation based on when two bodies collide at certain vertices.
  if (collidingBodies.size()>0) {
      // Find the agent of this body.
      auto agentA = reinterpret_cast<VertexData*>(collidingBodies[0]->GetUserData())->agent;
      auto agentB = reinterpret_cast<VertexData*>(collidingBodies[1]->GetUserData())->agent;
    
      // If both the agents have that state, then they'll bond.
      SuperAgent superAgent; bool found = false;
      std::shared_ptr<ofxBox2dJoint> j;
      // Check for existing joints.
      for (auto &sa : superAgents) {
        if (sa.contains(agentA, agentB)) {
          j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
          sa.joints.push_back(j);
          found = true;
        }
      }
    
      if (!found) {
        j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
        superAgent.setup(agentA, agentB, j); // Create a new super agent.
        superAgents.push_back(superAgent);
      }
    
      collidingBodies.clear();
  }
}

std::shared_ptr<ofxBox2dJoint> ofApp::createInterAgentJoint(b2Body *bodyA, b2Body *bodyB) {
    auto j = std::make_shared<ofxBox2dJoint>();
    float f = ofRandom(0.3, frequency);
    float d = ofRandom(1, damping);
    j->setup(box2d.getWorld(), bodyA, bodyB, f, d); // Use the interAgentJoint props.
  
    // Joint length
    int jointLength = ofRandom(250, 300);
    j->setLength(jointLength);
  
    // Enable interAgentJoint
    auto data = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    data->hasInterAgentJoint = true;
    bodyA->SetUserData(data);
  
    data = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    data->hasInterAgentJoint = true;
    bodyB->SetUserData(data);
  
    return j;
}
