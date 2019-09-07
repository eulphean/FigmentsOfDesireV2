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
  
  isOccupied = false;
  showGui = false;
  debug = false;
  showTexture = true;
  drawFbo = false;
  shouldBond = false;
  hideKinectGui = false;
  skipBgUpdate = true;
  
  // Pending deleted agents.
  pendingAgentsNum = 0;
  pendingAgentTime = 0;
  
  // Instantiate Midi.
  Midi::instance().setup();
  
  // Setup Kinect.
  kinect.setup();
  
  glEnable(GL_POINT_SMOOTH);
  
  // Init the joint mesh to start with.
  SuperAgent::initJointMesh();
  
  // Variable to keep track of who enters/exits the sapce
  prevPeopleSize = 0;
}

void ofApp::update(){
  //  processOsc();
  box2d.update();
  kinect.update();
  
  // Update super agents
  std::vector<int> removeIndices;
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, brokenBonds, removeIndices, shouldBond); // Possibly update the mesh here as well (for the interAgentJoints)
    return sa.shouldRemove;
  });
  
  // Update agents and remove them if their stretch
  // counter goes crazy.
  box2d.disableEvents();
  ofRemove(agents, [&](Agent *a) {
    a->update(alphaAgentProps, betaAgentProps);
    if (a->canExplode()) {
      // Fill exploded agents
      for (int i = 0; i < a->vertices.size()/4; i++) {
        Memory m (box2d, a->getCentroid(), true);
        explodedAgent.push_back(m);
      }
      
      // Go through colliding bodies and see if this agent's body is one
      // of the colliding bodies => If it is, clear that colliding body
      bool found = false;
      for (auto b : collidingBodies) {
         auto agent = reinterpret_cast<VertexData*>(b->GetUserData())->agent;
         if (agent == a) {
          found = true;
          break;
         }
      }
      
      // Clear
      if (found) {
        collidingBodies.clear();
      }
      
      ofRemove(collidingBodies, [&](b2Body *b) {
         auto agentA = reinterpret_cast<VertexData*>(b->GetUserData())->agent;
         return agentA == a;
      });
      
      a->clean(box2d); // Clean all the vertices and joints.
      
      // Midi hook
      a->enableStretchMidi(false);
      Midi::instance().sendAgentExplosionMidi();
      
      if (pendingAgentsNum == 0) {
          pendingAgentTime = ofGetElapsedTimeMillis(); // Reset time if it's the first time a new agent is deleted.
      }

      pendingAgentsNum++;
    
      return true;
    }
    return false;
  });
  box2d.enableEvents();
  
  // Track time
  if (ofGetElapsedTimeMillis() - pendingAgentTime > 40000 && pendingAgentsNum > 0) { // 30 seconds.
    cout << "Time elaped: Creating Agents: " << pendingAgentsNum << endl;
    createAgents(pendingAgentsNum);
    pendingAgentsNum = 0;
  }
  
  if (removeIndices.size() > 0) {
    // If I have removed something, update the mesh.
    SuperAgent::jointMesh.clear();
    SuperAgent::curMeshIdx = 0;
    for (auto &sa : superAgents) {
      sa.updateMeshIdx(); 
    }
  }
  
  // GUI props.
  updateAgentProps();
  
  // All the interaction logic.
  handleInteraction();
  
  // Update agents.
  for (auto &a : agents) {
    a->update(alphaAgentProps, betaAgentProps);
  }

  // Create super agents based on collision bodies.
  createSuperAgents();
  
  // Update background
  if (bg.isAllocated()) {
//    bg.update(skipBgUpdate, isOccupied);
      bg.updateBackground(); 
  }

  // Update broken bonds.
  ofRemove(brokenBonds, [&](Memory &m) {
    m.update();
    if (m.shouldRemove) {
      m.destroy();
    }
    return m.shouldRemove;
  });
  
  // Update explodedAgents
  ofRemove(explodedAgent, [&](Memory &m) {
    m.update();
    if (m.shouldRemove) {
      m.destroy();
    }
    return m.shouldRemove;
  });
  
  // Draw in the fbo if I'm screen capturing.
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
  
  // Draw all the interAgent joints. 
  SuperAgent::drawJointMesh();
  
  // Draw Agent is the virtual method for derived class.
  for (auto a: agents) {
    a->draw(showVisibilityRadius, showTexture);
  }

  // Draw broken bonds
  for (auto m : brokenBonds) {
    m.draw();
  }
  
  // Draw exploded agents
  for (auto m : explodedAgent) {
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
  
  // All debug logic.
  if (debug) {
    kinect.draw(); // Kinect debug code.
    // Alignment lines
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofSetLineWidth(10);
      ofDrawLine(0, ofGetHeight()/2, ofGetWidth(), ofGetHeight()/2); // Horizontal
      ofDrawLine(ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight());
    ofPopStyle();
  }
  
  // Visibility radius of the Kinect body.
  // TODO: Comes from the GUI. 
  if (showVisibilityRadius) {
    // Draw the visibility radius around agents as well as users
    auto people = kinect.getBodyCentroids();
    ofPushStyle();
      ofNoFill();
      for (auto p : people) {
        ofSetColor(ofColor::red);
        ofDrawCircle(p, audienceVisibilityRadius);
      }
    ofPopStyle();
  }
  
  // Show mouse cursor if the kinect didn't open. That means there was
  // no Kinect connected. 
  if (!kinect.kinectOpen) {
    ofPushStyle();
      ofSetColor(ofColor::yellow);
      for (auto p : testPeople) {
        ofDrawCircle(p, 5); 
      }
    
      if (showVisibilityRadius) {
        ofNoFill();
        ofSetColor(ofColor::red);
        for (auto p : testPeople) {
          ofDrawCircle(p, audienceVisibilityRadius);
        }
      }
    ofPopStyle();
  }
  
  
  // Health parameters
  if (showGui) {
    gui.draw();
  }
}

// ------------------ Activate Agent Behaviors With Audience Interaction --------------------- //

void ofApp::mousePressed(int x, int y, int button) {
  if (button == 2) { // Right click.
    if (testPeople.size() > 0) {
        // Trim the array
        auto randIdx = (int) ofRandom(testPeople.size());
        auto iterator = testPeople.begin();
        testPeople.erase(iterator + randIdx);
        testPeople.shrink_to_fit();
      }
    } else if (button == 0) { // Left click.
      testPeople.push_back(glm::vec2(x, y));
    }
}

void ofApp::handleInteraction() {
  if (kinect.kinectOpen) {
    // Is the area occupied?
    auto people = kinect.getBodyCentroids();
    isOccupied = people.size() > 0;
    //evaluateEntryExit(people.size()); // Hook for sound.
    
    if (isOccupied) {
      // Agents can bond now.
      shouldBond = true;
      setBehavior(people);
      specialRepelTimer = ofRandom(200, 300);
    } else {
      for (auto &a : agents) {
        a->enableStretchMidi(false); // It can happen here that targets disappear.
      }
      if (specialRepelTimer > 0) {
        enableRepelBeforeBreak();
      } else {
        wasteTime();
        clearInterAgentBonds();
      }
    }
  } else { // Test Routine
    isOccupied = testPeople.size() > 0;
    //evaluateEntryExit(testPeople.size()); // Hook for sound.
    
    if (isOccupied) {
      shouldBond = true;
      setBehavior(testPeople);
      specialRepelTimer = ofRandom(200, 300);
    } else {
      for (auto &a : agents) {
        a->enableStretchMidi(false); // It can happen here that targets disappear.
      }
      if (specialRepelTimer > 0) {
        enableRepelBeforeBreak();
      } else {
        // Do other silly things
        wasteTime();
        clearInterAgentBonds();
      }
    }
  }
}

void ofApp::evaluateEntryExit(int curPeopleSize) {
//    if (prevPeopleSize < curPeopleSize) {
//      // Somebody entered
//      Midi::instance().sendEntryExitMidi(true);
//    } else if (prevPeopleSize > curPeopleSize) {
//      // Somebody left
//      Midi::instance().sendEntryExitMidi(false);
//    }
  
    prevPeopleSize = curPeopleSize; 
}

void ofApp::setBehavior(std::vector<glm::vec2> people) {
  // Get all visible agents.
  for (auto p : people) {
    // Visible Agents.
    auto visibleAgents = getVisibleAgents(p);
    // Apply stretch on visible agents
    for (auto &a : visibleAgents) {
      a->setBehavior(Behavior::Stretch, {}, true); // All visible agents, turn on the note! They turn it off, as soon as they become invisible
      a->enableStretchMidi(true); 
    }
  }
  
  // Toss a coin on the invisible targets for each agent
  // to attract or repel from the people.
  for (auto &a : agents) {
    auto invisibleTargets = getInvisibleTargets(people, a);
    if (ofRandom(1) < 0.85) {
      a->setBehavior(Behavior::Attract, invisibleTargets);
    } else {
      a->setBehavior(Behavior::Repel, invisibleTargets);
    }
    
    // How do I test if an agent is in the nest or nest or not. 
    
    // If no visible targets, turn off the midi.
    auto numVisibleTargets = people.size() - invisibleTargets.size();
    if (numVisibleTargets == 0) {
      a->enableStretchMidi(false);
      a->stretchCounter = 0; 
    }
  }
}

void ofApp::wasteTime() {
  for (auto &a : agents) {
    auto target = glm::vec2(ofRandom(50, ofGetWidth()-50), ofRandom(50, ofGetHeight()-50));
    if (ofRandom(1) < 0.3) { // Low priority for seeking targets. Lower the movement.
      a->setBehavior(Behavior::Attract, { target });
    } else {
      a->setBehavior(Behavior::Shock); 
    }
  }
}

void ofApp::enableRepelBeforeBreak() {
  if (superAgents.size() > 0) {
    // Keep tracking time.
    if (specialRepelTimer > 0) {
      specialRepelTimer--;
    }
    
    // Enable special repel. 
    for (auto &sa : superAgents) {
      auto agentA = sa.agentA;
      auto agentB = sa.agentB;
      agentA->setBehavior(Behavior::SpecialRepel, { agentB->getCentroid() });
      agentB->setBehavior(Behavior::SpecialRepel, { agentA->getCentroid() });
    }
  }
}

void ofApp::clearInterAgentBonds() {
  shouldBond = false;
  if (superAgents.size() == 0) {
    SuperAgent::initJointMesh(); // Clear the mesh and reinitialize
  }
}

Agent* ofApp::getClosestAgent(std::vector<Agent *> targetAgents, glm::vec2 targetPos) {
  auto minD = 9999;
  Agent *minAgent = NULL;
  for (auto &a : targetAgents) {
    auto d = glm::distance(targetPos, a->getCentroid());
    if (d < minD) {
      minD = d;
      minAgent = a;
    }
  }
  return minAgent;
}

std::vector<glm::vec2> ofApp::getInvisibleTargets(std::vector<glm::vec2> people, Agent* a) {
  std::vector<glm::vec2> invisibleTargets;
  for (auto p : people) {
    auto d = glm::distance(a->getCentroid(), p);
    if (d > audienceVisibilityRadius + a->visibilityRadius) {
      invisibleTargets.push_back(p);
    }
  }
  
  return invisibleTargets;
}

std::vector<Agent *> ofApp::getVisibleAgents(glm::vec2 target) {
  std::vector<Agent *> visibleAgents;
  // Find the closest agents to the target
  for (auto &a : agents) {
    auto d = glm::distance(a->getCentroid(), target);
    if (d <= a->visibilityRadius + audienceVisibilityRadius) {
      visibleAgents.push_back(a);
    }
  }

  return visibleAgents;
}

std::vector<Agent*> ofApp::getInvisibleAgents(glm::vec2 target) {
  std::vector<Agent *> invisibleAgents;
  // Find the closest agents to the target
  for (auto &a : agents) {
    auto d = glm::distance(a->getCentroid(), target);
    if (d > a->visibilityRadius + audienceVisibilityRadius) {
      invisibleAgents.push_back(a);
    }
  }

  return invisibleAgents;
}


void ofApp::keyPressed(int key){
  // ------------------ Interactive Gestures --------------------- //
  if (key == 's') {
    skipBgUpdate = !skipBgUpdate;
  }
  
  if (key == 'v') {
    showVisibilityRadius = !showVisibilityRadius;
  }
  
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgents(numAgentsToCreate);
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
    bounds.x = -150; bounds.y = -150;
    bounds.width = ofGetWidth() + (-1) * bounds.x * 2; bounds.height = ofGetHeight() + (-1) * 2 * bounds.y;
    box2d.createBounds(bounds);
    
    // Allocate the fbo for screen grabbing.
    if (screenGrabFbo.isAllocated()) {
      screenGrabFbo.clear();
      screenGrabFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    }
  }
  
  cout << "Create new background." << endl;
  // Create background
  bg.setup();
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("The Nest GUI");
  
    // General settings
    generalParams.setName("General Parameters");
    generalParams.add(alphaAgentProbability.set("Alpha Agent Probability", 0.1, 0, 0.9));
    generalParams.add(audienceVisibilityRadius.set("Audience Visibility Radius", 100, 50, 200)); 
    generalParams.add(numAgentsToCreate.set("Num Agents To Create", 1, 1, 10));
  
    // Alpha Agent GUI parameters
    alphaAgentParams.setName("Alpha Agent Params");
    alphaAgentParams.add(aVisibilityRadiusFactor.set("Visibility Radius Factor", 2, 1, 5));
    alphaAgentParams.add(aMeshRows.set("Mesh Rows", 5, 5, 100));
    alphaAgentParams.add(aMeshColumns.set("Mesh Columns", 5, 5, 100));
    alphaAgentParams.add(aMeshWidth.set("Mesh Width", 50, 10, 300));
    alphaAgentParams.add(aMeshHeight.set("Mesh Height", 50, 10, 300));
    alphaAgentParams.add(aTextureWidth.set("Texture Width", 50, 10, 500));
    alphaAgentParams.add(aTextureHeight.set("Texture Height", 50, 10, 500));
    alphaAgentParams.add(aVertexDensity.set("Vertex Density", 1, 0, 5));
    alphaAgentParams.add(aVertexBounce.set("Vertex Bounce", 0.1, 0, 1));
    alphaAgentParams.add(aVertexFriction.set("Vertex Friction", 1, 0, 1));
    alphaAgentParams.add(aVertexRadius.set("Vertex Radius", 3, 1, 10));
    alphaAgentParams.add(aJointFrequency.set("Joint Frequency", 2, 0, 20));
    alphaAgentParams.add(aJointDamping.set("Joint Damping", 1, 0, 5));
    // Weights.
    alphaAgentParams.add(aStretchWeight.set("Max Stretch Weight", 1.5, 0, 10));
    alphaAgentParams.add(aRepulsionWeight.set("Max Repulsion Weight", 2.5, 0, 20));
    alphaAgentParams.add(aAttractionWeight.set("Max Attraction Weight", 1.0, 0, 20));
    alphaAgentParams.add(aTickleWeight.set("Max Tickle Weight", 2.5, 0, 20));
    alphaAgentParams.add(aVelocity.set("Max Velocity", 15, 1, 20));

    // Beta Agent GUI parameters
    betaAgentParams.setName("Beta Agent Params");
    betaAgentParams.add(bVisibilityRadiusFactor.set("Visibility Radius Factor", 2, 1, 5)); 
    betaAgentParams.add(bMeshRadius.set("Mesh Radius", 5, 5, 100));
    betaAgentParams.add(bTextureWidth.set("Texture Width", 50, 10, 300));
    betaAgentParams.add(bTextureHeight.set("Texture Height", 50, 10, 300));
    betaAgentParams.add(bVertexDensity.set("Vertex Density", 1, 0, 5));
    betaAgentParams.add(bVertexBounce.set("Vertex Bounce", 0.1, 0, 1));
    betaAgentParams.add(bVertexFriction.set("Vertex Friction", 1, 0, 1));
    betaAgentParams.add(bVertexRadius.set("Vertex Radius", 3, 1, 10));
    betaAgentParams.add(bCenterJointFrequency.set("Center Joint Frequency", 2, 0, 20));
    betaAgentParams.add(bCenterJointDamping.set("Center Joint Damping", 1, 0, 5));
    betaAgentParams.add(bSideJointFrequency.set("Side Joint Frequency", 2, 0, 20));
    betaAgentParams.add(bSideJointDamping.set("Side Joint Damping", 1, 0, 5));
    betaAgentParams.add(bSideJointOffset.set("Side Joint Offset", 5, 0, 30));
    // Weights.
    betaAgentParams.add(bStretchWeight.set("Max Stretch Weight", 1.0, 0, 10));
    betaAgentParams.add(bRepulsionWeight.set("Max Repulsion Weight", 2.5, 0, 20));
    betaAgentParams.add(bAttractionWeight.set("Max Attraction Weight", 0.5, 0, 20));
    betaAgentParams.add(bTickleWeight.set("Max Tickle Weight", 2.5, 0, 20));
    betaAgentParams.add(bVelocity.set("Max Velocity", 15, 1, 20));
  
    // InterAgentJoint GUI parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(iJointFrequency.set("Joint Frequency", 2, 0, 20));
    interAgentJointParams.add(iJointDamping.set("Joint Damping", 1, 0, 10));
    interAgentJointParams.add(iMinJointLength.set("Min Joint Length", 250, 100, 1000));
    interAgentJointParams.add(iMaxJointLength.set("Max Joint Length", 300, 100, 1000));

    settings.add(generalParams);
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
  // Weights.
  alphaAgentProps.stretchWeight = aStretchWeight;
  alphaAgentProps.repulsionWeight = aRepulsionWeight;
  alphaAgentProps.attractionWeight = aAttractionWeight;
  alphaAgentProps.tickleWeight = aTickleWeight;
  alphaAgentProps.velocity = aVelocity;
  alphaAgentProps.visibilityRadiusFactor = aVisibilityRadiusFactor;
  
  // Beta Agent GUI param payload.
  betaAgentProps.meshRadius = bMeshRadius;
  betaAgentProps.textureSize = ofPoint(bTextureWidth, bTextureHeight);
  betaAgentProps.vertexPhysics = ofPoint(bVertexBounce, bVertexDensity, bVertexFriction);
  betaAgentProps.vertexRadius = bVertexRadius;
  betaAgentProps.centerJointPhysics = ofPoint(bCenterJointFrequency, bCenterJointDamping);
  betaAgentProps.sideJointPhysics = ofPoint(bSideJointFrequency, bSideJointDamping);
  betaAgentProps.sideJointOffset = bSideJointOffset;
  // Weights.
  betaAgentProps.stretchWeight = bStretchWeight;
  betaAgentProps.repulsionWeight = bRepulsionWeight;
  betaAgentProps.attractionWeight = bAttractionWeight;
  betaAgentProps.tickleWeight = bTickleWeight;
  betaAgentProps.velocity = bVelocity;
  betaAgentProps.visibilityRadiusFactor = bVisibilityRadiusFactor;
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
      createAgents(numAgentsToCreate);
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

void ofApp::createAgents(int numAgents) {
  for (int i = 0; i < numAgents; i++) {
    ofPoint origin = ofPoint(ofRandom(50, 500), ofRandom(50, ofGetHeight()-100));
    Agent *agent;
    alphaAgentProps.meshOrigin = origin;
    agent = new Alpha(box2d, alphaAgentProps);
    agents.push_back(agent);
  }
  cout << "Total Agents: " << agents.size() << endl; 
}


void ofApp::removeJoints() {
  box2d.disableEvents();

  // Clear superAgents only
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();
  SuperAgent::initJointMesh(); // Clear the mesh and reinitialize

  superAgents.clear();
  box2d.enableEvents();
}


void ofApp::removeUnbonded() {
  ofRemove(agents, [&](Agent *a) {
    return false;
  });
}

void ofApp::clearScreen() {
  SloganFactory::instance().clearAllocatedSlogans();
  
  // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
  // middle of a step function. Disabling and renabling the events work as a good solution for now.
  box2d.disableEvents();
  collidingBodies.clear();

  // Clear SuperAgents
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();
  SuperAgent::initJointMesh(); // Clear the joint mesh as well.
  
  // Clean agents
  for (auto &a : agents) {
    a->clean(box2d);
    delete a;
  }
  agents.clear();

  box2d.enableEvents();
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
          if (agentA->currentBehavior == None) {
            if (ofRandom(1) < 0.90) {
              dataA->applyRepulsion = true;
              e.a->GetBody()->SetUserData(dataA);
            } else {
              dataA->applyAttraction = true;
              e.a->GetBody()->SetUserData(dataA);
            }
          }
          
          if (agentB->currentBehavior == None) {
            if (ofRandom(1) < 0.90) {
              dataA->applyRepulsion = true;
              e.a->GetBody()->SetUserData(dataA);
            } else {
              dataB->applyAttraction = true;
              e.b->GetBody()->SetUserData(dataB);
            }
          }

          // If agents can bond, evaluate the colliding bodies for collision.
          // Along with the agents they both belong to.
          if (shouldBond) {
            evaluateBonding(e.a->GetBody(), e.b->GetBody(), agentA, agentB);
          }
        }
      }
    }
  }
}

// ------------------------------ Inter-Agent Bonding Routines ------------------------------------ //

// Critical routine that evaluates when the 2 bodies should actually bond to each other.
void ofApp::evaluateBonding(b2Body *bodyA, b2Body *bodyB, Agent *agentA, Agent *agentB) {
  collidingBodies.clear();
  
  if (agentA->stretchCounter<100 && agentB->stretchCounter<100) {
    // Vertex level checks. Is this vertex bonded to anything except itself?
    bool a = canVertexBond(bodyA, agentA);
    bool b = canVertexBond(bodyB, agentB);
    if (a && b) {
      // Prepare for bond.
      collidingBodies.push_back(bodyA);
      collidingBodies.push_back(bodyB);
    }
  }
}

bool ofApp::canVertexBond(b2Body* body, Agent *curAgent) {
  // Does it have an interAgent joint already? If it doesn, can't allow this body to create another joint.
  auto data = reinterpret_cast<VertexData*>(body->GetUserData());
  return !data->hasInterAgentJoint;
}

void ofApp::createSuperAgents() {
  // Joint creation based on when two bodies collide at certain vertices.
  if (collidingBodies.size()>0) {
      // Find the agent of this body.
        auto bodyA = reinterpret_cast<VertexData*>(collidingBodies[0]);
        auto bodyB = reinterpret_cast<VertexData*>(collidingBodies[1]);
        
        if (bodyA && bodyB) {
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
      }
    
      collidingBodies.clear();
  }
}

std::shared_ptr<ofxBox2dJoint> ofApp::createInterAgentJoint(b2Body *bodyA, b2Body *bodyB) {
    auto j = std::make_shared<ofxBox2dJoint>();
    j->setup(box2d.getWorld(), bodyA, bodyB, iJointFrequency, iJointDamping); // Use the interAgentJoint props.
  
    // Joint length (determine with probability)
    int jointLength = ofRandom(iMinJointLength, iMaxJointLength);
    j->setLength(jointLength);
  
    // Update Body A
    auto data = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    data->hasInterAgentJoint = true;
    data->jointMeshIdx = SuperAgent::curMeshIdx;
    bodyA->SetUserData(data);
  
    // Update Body B
    data = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    data->hasInterAgentJoint = true;
    data->jointMeshIdx = SuperAgent::curMeshIdx + 1;
    bodyB->SetUserData(data);
  
    // Insert these into mesh for the interAgent joints.
    auto posA = getBodyPosition(bodyA); auto posB = getBodyPosition(bodyB);
    SuperAgent::insertJointMesh(glm::vec3(posA.x, posA.y, 0), glm::vec3(posB.x, posB.y, 0));
  
    // Increment by 2 because it just served 2 bodies.
    SuperAgent::curMeshIdx += 2;
  
    return j;
}

// Unused code. Don't need this for now.
//          // Desire state is ATTRACTION!
//          // Repel the other agent.
//          if (agentA->desireState == Attraction) {
//             // Attract A's vertices
//             if (!dataA->hasInterAgentJoint) {
//               dataA->applyAttraction = true;
//               e.a->GetBody()->SetUserData(dataA);
//            }
//
//            // Repel B's vertices
//            if (!dataB->hasInterAgentJoint) {
//              dataB->applyRepulsion = true;
//              e.b->GetBody()->SetUserData(dataB);
//            }
//          
//            // Reset agent state to None on collision.
//            agentA->setDesireState(None); // TODO: Fix this.
//          }
//
//          if (agentB->desireState == Attraction) {
//            // Attract B's vertice
//             if (!dataB->hasInterAgentJoint) {
//              dataB->applyAttraction = true;
//              e.b->GetBody()->SetUserData(dataB);
//            }
//
//            // Repel A's vertices
//            if (!dataA->hasInterAgentJoint) {
//               dataA->applyRepulsion = true;
//               e.a->GetBody()->SetUserData(dataA);
//            }
//
//            // Reset agent state to None on collision.
//            agentB->setDesireState(None); // TODO: Fix this.
//          }

//    for (auto &sa : superAgents) {
//      auto joints = sa.joints;
//      for (auto &j : joints) {
//        auto bodyA = j->joint->GetBodyA();
//        auto dataA = reinterpret_cast<VertexData*>(j->joint->GetBodyA()->GetUserData());
//
//        auto bodyB = j->joint->GetBodyB();
//        auto dataB = reinterpret_cast<VertexData*>(j->joint->GetBodyB()->GetUserData());
//
//        // Enable repulsion on bodyA
//        dataA->targetPos = getBodyPosition(bodyB);
//        dataA->applyRepulsion = true;
//        bodyA->SetUserData(dataA);
//
//        // Enable repulsion on bodyB
//        dataB->targetPos = getBodyPosition(bodyA);
//        dataB->applyRepulsion = true;
//        bodyB->SetUserData(dataB);
//      }
//    }
