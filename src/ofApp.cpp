#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Setup OSC
  receiver.setup(PORT);
//  ofHideCursor();
  
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
  
  showGui = false;
  debug = false;
  showTexture = true;
  drawFbo = false;
  shouldBond = false;
  hideKinectGui = false; 
  
  // Instantiate Midi.
  Midi::instance().setup();
  
  // Setup Kinect.
  kinect.setup();
  
  // [NOTE] Press w to create a new world after setting the bounds of the program
  // in the extended monitor. 
}

void ofApp::update(){
  box2d.update();
//  processOsc();
  kinect.update();
  
  // Update super agents
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, memories, shouldBond); // Possibly update the mesh here as well (for the interAgentJoints)
    return sa.shouldRemove;
  });
  
  // GUI props.
  updateAgentProps();
  
  std::vector<ofMesh> meshes;
  
  // Update agents
  for (auto &a : agents) {
    a->update();
    // meshes.push_back(a->getMesh());
  }
  
  // Create super agents based on collision bodies.
  createSuperAgents();
  
  // Update background
  if (bg.isAllocated()) {
    bg.update(meshes);
  }

  // Update memories.
  ofRemove(memories, [&](Memory &m) {
    m.update();
    return m.shouldRemove;
  });
  
  // Start drawing in the screen grab fbo.
  if (drawFbo) {
    screenGrabFbo.begin();
      ofClear(ofColor::black);
      drawSequence();
    screenGrabFbo.end();
  }
}

void ofApp::draw(){
  if (drawFbo) {
    screenGrabFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
  } else {
    drawSequence();
  }
}

void ofApp::drawSequence() {
  // Draw background.
  if (bg.isAllocated()) {
    bg.draw(debug);
  }

  // Draw all what's inside the super agents.
  for (auto sa: superAgents) {
    sa.draw();
  }

  // Draw Agent is the virtual method for derived class.
  for (auto a: agents) {
    a->draw(debug, showTexture);
  }

  // Draw memories
  for (auto m : memories) {
    m.draw();
  }
  
  // Show the current frame rate.
  ofPushMatrix();
    ofScale(2, 2);
    ofPushStyle();
      ofSetColor(ofColor::black);
      ofDrawBitmapString(ofGetFrameRate(), 50, 50);
    ofPopStyle();
  ofPopMatrix();
  
  // Health parameters
  if (showGui) {
    gui.draw();
  }
  
  // Draw Kinect content.
  kinect.draw(debug, showGui);
}

void ofApp::keyPressed(int key){
  // ------------------ Interactive Gestures --------------------- //
  
  // Attract
  if (key == 'a') {
    attract();
  }
  
  // Repel
  if (key == 'r') {
    repel();
  }
  
  // Stretch
  if (key == 's') {
    stretch();
  }
  
  // Enable/Disable Bonding
  if (key == 'b') {
    enableBonding();
  }
  
  // Tickle the agents.
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a -> setTickle(1.0);
    }
  }
  
  // ------------------ Interactive Gestures --------------------- //
  
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
    showGui = !showGui;
  }
  
  if (key == 't') {
    showTexture = !showTexture; 
  }
  
  if (key == 'f') {
    drawFbo = !drawFbo;
  }
  
  if (key == 'w') {
    createWorld(true);
  }
  
  if (key == 'k') {
    hideKinectGui = !hideKinectGui;
  }
  
  // Save a screen grab of the high quality fbo that is getting drawn currently. 
  if (key == ' ') {
    ofPixels pix;
    screenGrabFbo.readToPixels(pix);
    auto fileName = "High_Res" + ofToString(screenCaptureIdx) + ".png";
    screenCaptureIdx++;
    ofSaveImage(pix, fileName, OF_IMAGE_QUALITY_BEST);
  }
}

void ofApp::exit() {
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
  kinect.gui.saveToFile("Kinect.xml");
}

// ------------------------------ Critical Helper Routines --------------------------------------- //

void ofApp::createWorld(bool createBounds) {
  if (bg.isAllocated()) {
    cout << "Destroying the background." << endl;
    bg.destroy();
  }
  
  if (createBounds) {
    cout << "Creating new bounds." << endl;
    // Bounds
    bounds.x = -20; bounds.y = -20;
    bounds.width = ofGetWidth() + (-1) * bounds.x * 2; bounds.height = ofGetHeight() + (-1) * 2 * bounds.y;
    box2d.createBounds(bounds);
    
    // Allocate the fbo for screen grabbing.
    if (screenGrabFbo.isAllocated()) {
      screenGrabFbo.clear();
      screenGrabFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    }
  }
  
  cout << "Create new background." << endl;
  // Store params (must) and create background.
  bg.setParams(bgParams);
  bg.setup();
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("The Nest GUI");
  
    // Background GUI parameters.
    bgParams.setName("Background Params");
    bgParams.add(bgAttraction.set("Attraction", 20, -200, 200));
    bgParams.add(bgRepulsion.set("Repulsion", -20, -200, 200));
    bgAttraction.addListener(this, &ofApp::bgUpdateParams);
    bgRepulsion.addListener(this, &ofApp::bgUpdateParams);
  
    // Alpha Agent GUI parameters
    alphaAgentParams.setName("Alpha Agent Params");
    alphaAgentParams.add(aMeshRows.set("Mesh Rows", 5, 5, 100));
    alphaAgentParams.add(aMeshColumns.set("Mesh Columns", 5, 5, 100));
    alphaAgentParams.add(aMeshWidth.set("Mesh Width", 50, 10, 300));
    alphaAgentParams.add(aMeshHeight.set("Mesh Height", 50, 10, 300));
    alphaAgentParams.add(aTextureWidth.set("Texture Width", 50, 10, 500));
    alphaAgentParams.add(aTextureHeight.set("Texture Height", 50, 10, 500));
    alphaAgentParams.add(aVertexDensity.set("Vertex Density", 1, 0, 5));
    alphaAgentParams.add(aVertexBounce.set("Vertex Bounce", 0.1, 0, 1));
    alphaAgentParams.add(aVertexFriction.set("Vertex Friction", 1, 0, 1));
    alphaAgentParams.add(aVertexRadius.set("Vertex Radius", 5, 1, 10));
    alphaAgentParams.add(aJointFrequency.set("Joint Frequency", 2, 0, 20));
    alphaAgentParams.add(aJointDamping.set("Joint Damping", 1, 0, 5));

    // Beta Agent GUI parameters
    betaAgentParams.setName("Beta Agent Params");
    betaAgentParams.add(bMeshRadius.set("Mesh Radius", 10, 5, 100));
    betaAgentParams.add(bTextureWidth.set("Texture Width", 50, 10, 300));
    betaAgentParams.add(bTextureHeight.set("Texture Height", 50, 10, 300));
    betaAgentParams.add(bVertexDensity.set("Vertex Density", 1, 0, 5));
    betaAgentParams.add(bVertexBounce.set("Vertex Bounce", 0.1, 0, 1));
    betaAgentParams.add(bVertexFriction.set("Vertex Friction", 1, 0, 1));
    betaAgentParams.add(bVertexRadius.set("Vertex Radius", 5, 1, 10));
    betaAgentParams.add(bCenterJointFrequency.set("Center Joint Frequency", 2, 0, 20));
    betaAgentParams.add(bCenterJointDamping.set("Center Joint Damping", 1, 0, 5));
    betaAgentParams.add(bSideJointFrequency.set("Side Joint Frequency", 2, 0, 20));
    betaAgentParams.add(bSideJointDamping.set("Side Joint Damping", 1, 0, 5));
  
    // InterAgentJoint GUI parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(iJointFrequency.set("Joint Frequency", 2, 0, 20));
    interAgentJointParams.add(iJointDamping.set("Joint Damping", 1, 0, 10));
    interAgentJointParams.add(iMinJointLength.set("Min Joint Length", 250, 50, 600));
    interAgentJointParams.add(iMaxJointLength.set("Max Joint Length", 300, 50, 600));

    settings.add(bgParams);
    settings.add(alphaAgentParams);
    settings.add(betaAgentParams);
    settings.add(interAgentJointParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::updateAgentProps() {
  // Alpha Agent GUI param payload.
  alphaAgentProps.meshSize = ofPoint(aMeshWidth, aMeshHeight);
  alphaAgentProps.meshRowsColumns = ofPoint(aMeshRows, aMeshColumns);
  alphaAgentProps.textureSize = ofPoint(aTextureWidth, aTextureHeight);
  alphaAgentProps.vertexPhysics = ofPoint(aVertexBounce, aVertexDensity, aVertexFriction);
  alphaAgentProps.vertexRadius = aVertexRadius;
  alphaAgentProps.jointPhysics = ofPoint(aJointFrequency, aJointDamping);
  
  // Beta Agent GUI param payload.
  betaAgentProps.meshRadius = bMeshRadius;
  betaAgentProps.textureSize = ofPoint(bTextureWidth, bTextureHeight);
  betaAgentProps.vertexPhysics = ofPoint(bVertexBounce, bVertexDensity, bVertexFriction);
  betaAgentProps.vertexRadius = bVertexRadius;
  betaAgentProps.centerJointPhysics = ofPoint(bCenterJointFrequency, bCenterJointDamping);
  betaAgentProps.sideJointPhysics = ofPoint(bSideJointFrequency, bSideJointDamping);
}

void ofApp::processOsc() {
  while(receiver.hasWaitingMessages()){
    // get the next message
    ofxOscMessage m;
    receiver.getNextMessage(m);
    
    // ------------------ PIPES/GUI OSC Messages -----------------------
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

glm::vec2 ofApp::getBodyPosition(b2Body* body) {
  auto xf = body->GetTransform();
  b2Vec2 pos = body->GetLocalCenter();
  b2Vec2 b2Center = b2Mul(xf, pos);
  auto p = worldPtToscreenPt(b2Center);
  return glm::vec2(p.x, p.y);
}


// ------------------------------ Interactive Routines --------------------------------------- //

void ofApp::createAgents() {
  ofPoint origin = ofPoint(ofGetWidth()/2, ofGetHeight()/2);
  Agent *agent;
  // Based on a probablity, create a new agent.
  if (ofRandom(1) < 0.8) {
    alphaAgentProps.meshOrigin = origin;
    agent = new Alpha(box2d, alphaAgentProps);
  } else {
    betaAgentProps.meshOrigin = origin;
    agent = new Beta(box2d, betaAgentProps);
  }
  
  agents.push_back(agent);
}

void ofApp::attract() {
  // Pick a random figment and enable attraction for that figment.
  int randIdx = ofRandom(agents.size());
  Agent *curAgent = agents[randIdx];

  // Enable attraction in the figment.
  curAgent->setDesireState(Attraction);
}

void ofApp::repel() {
  // Repel each figment away from each other
  for (auto &a: agents) {
    a->setDesireState(Repulsion);
  }
}

void ofApp::stretch() {
  // Either one or all the agents stretch based on a probability.
  if (ofRandom(1) < 0.5) {
    // Pick a random agent and make stretch.
    int randIdx = ofRandom(agents.size());
    auto agent = agents[randIdx];
    agent->setStretch();
  } else {
    // Stretch them all.
    for (auto &a : agents) {
      if (a->desireState != Repulsion) {
        a->setStretch();
      }
    }
  }
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

void ofApp::enableBonding() {
  shouldBond = !shouldBond;
}

void ofApp::removeUnbonded() {
  ofRemove(agents, [&](Agent *a) {
  
    return false;
  });
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

// ------------------------------ Background Parameter Update Routine ------------------------------- // 

void ofApp::bgUpdateParams(int & newVal) {
  bg.setParams(bgParams);
}

// ------------------------------ Agent Body Contact Routines --------------------------------------- //

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

// ------------------------------ Inter-Agent Bonding Routines ------------------------------------ //

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
        // Is there a SuperAgent that already exists?
        if (sa.contains(agentA, agentB)) {
          j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
          sa.joints.push_back(j);
          found = true;
        }
      }
    
      // Create a new Super Agent. 
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
//    float f = ofRandom(0.3, iJointFrequency);
//    float d = ofRandom(1, iJointDamping);
    j->setup(box2d.getWorld(), bodyA, bodyB, iJointFrequency, iJointDamping); // Use the interAgentJoint props.
  
    // Joint length (determine with probability)
    int jointLength = ofRandom(iMinJointLength, iMaxJointLength);
    j->setLength(jointLength);
  
    // Enable interAgentJoint
  
    // Update Body A
    auto data = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    data->hasInterAgentJoint = true;
    bodyA->SetUserData(data);
  
    // Update Body B
    data = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    data->hasInterAgentJoint = true;
    bodyB->SetUserData(data);
  
    return j;
}
