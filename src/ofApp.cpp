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
  
  // Instantiate Midi.
  Midi::instance().setup();
  
  // Setup Kinect.
  kinect.setup();
  
  glEnable(GL_POINT_SMOOTH);
  
  // Init the joint mesh to start with.
  SuperAgent::initJointMesh();
}

void ofApp::update(){
//  processOsc();
  box2d.update();
  kinect.update();
  
  // Update super agents
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, memories, shouldBond); // Possibly update the mesh here as well (for the interAgentJoints)
    return sa.shouldRemove;
  });
  
  // GUI props.
  updateAgentProps();
  
  // All the interaction logic.
  handleInteraction();
  
  // Update agents
  for (auto &a : agents) {
    a->update(alphaAgentProps, betaAgentProps);
  }
  
  // Create super agents based on collision bodies.
  createSuperAgents();
  
  // Update background
  if (bg.isAllocated()) {
    bg.update();
  }

  // Update memories.
  ofRemove(memories, [&](Memory &m) {
    m.update();
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
  
  // Print a circle so we know where all the agents are being attracted to.
  if (isOccupied) {
    ofPushStyle();
      ofSetColor(ofColor::yellow);
      ofDrawCircle(ofGetMouseX(), ofGetMouseY(), 5);
    ofPopStyle();
  }
  
  // Draw Kinect content.
  kinect.draw(debug, showGui);
}

// ------------------ Activate Agent Behaviors With Audience Interaction --------------------- //
void ofApp::mouseEntered(int x, int y) {
  isOccupied = true;
}

void ofApp::mouseExited(int x, int y) {
  isOccupied = false;
}

void ofApp::handleInteraction() {
  if (kinect.kinectOpen) {
    if (isOccupied) {
      auto people = kinect.getBodyCentroids();
      // Activate all the possible behaviors on the agents.
      attract(people);
      tickle(people);
    }
  } else {
    if (isOccupied) {
      std::vector<glm::vec2> people;
      people.push_back(glm::vec2(ofGetMouseX(), ofGetMouseY()));
      // Activate all the possible behaviors on the agents.
      attract(people);
      tickle(people);
    }
  }
}

void ofApp::attract(std::vector<glm::vec2> targets) {
  // Closest agent (centroid wise)
  for (auto t: targets) {
    auto closestAgent = getClosestAgent(t);
    if (closestAgent != NULL) {
      closestAgent->setDesireState(Attraction, t);
    }
  }
}

void ofApp::tickle(std::vector<glm::vec2> targets) {
  for (auto t: targets) {
    for (auto &a : agents) {
      auto d = glm::distance(t, a->getCentroid());
      if (d <= a->visibilityRadius) {
        a->tickle();
      }
    }
  }
}

void ofApp::stretch(std::vector<glm::vec2> targets) {
  // Not implemented currently.
}

void ofApp::repel(std::vector<glm::vec2> targets) {
  // Repel each figment away from each other
  for (auto &a: agents) {
    a->setDesireState(Repulsion, targets[0]); // TODO fix this.
  }
}

Agent* ofApp::getClosestAgent(glm::vec2 targetPos) {
  auto minD = 9999;
  Agent *minAgent = NULL;
  for (auto &a : agents) {
    auto d = glm::distance(targetPos, a->getCentroid());
    if (d < minD) {
      minD = d;
      minAgent = a;
    }
  }
  return minAgent;
}



void ofApp::keyPressed(int key){
  // ------------------ Interactive Gestures --------------------- //
  
  // Enable/Disable Bonding
  if (key == 'b') {
    enableBonding();
  }
  
  // Tickle the agents.
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a->tickle();
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
  // Create background
  bg.setup();
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("The Nest GUI");
  
    // General settings
    generalParams.setName("General Parameters");
    generalParams.add(alphaAgentProbability.set("Alpha Agent Probability", 0.1, 0, 0.9));
  
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
    interAgentJointParams.add(iMinJointLength.set("Min Joint Length", 250, 50, 600));
    interAgentJointParams.add(iMaxJointLength.set("Max Joint Length", 300, 50, 600));

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
  if (ofRandom(1) <= alphaAgentProbability) {
    alphaAgentProps.meshOrigin = origin;
    agent = new Alpha(box2d, alphaAgentProps);
  } else {
    betaAgentProps.meshOrigin = origin;
    agent = new Beta(box2d, betaAgentProps);
  }
  
  agents.push_back(agent);
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

void ofApp::enableBonding() {
  shouldBond = !shouldBond;
  
  if (!shouldBond) {
    SuperAgent::initJointMesh(); // Clear the mesh and reinitialize
  }
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
            agentA->setDesireState(None, glm::vec2(0, 0)); // TODO: Fix this. 
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
            agentB->setDesireState(None, glm::vec2(0, 0)); // TODO: Fix this.
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
  // Does it have an interAgent joint already? If it doesn, can't allow this body to create another joint.
  auto data = reinterpret_cast<VertexData*>(body->GetUserData());
  return !data->hasInterAgentJoint;
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
    j->setup(box2d.getWorld(), bodyA, bodyB, iJointFrequency, iJointDamping); // Use the interAgentJoint props.
  
    // Joint length (determine with probability)
    int jointLength = ofRandom(iMinJointLength, iMaxJointLength);
    j->setLength(jointLength);
  
    // Enable interAgentJoint
  
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
  
    // Increment by 2 because it just served 2 bodies. 
    SuperAgent::curMeshIdx += 2;
  
    // Insert these into mesh for the interAgent joints.
    auto posA = getBodyPosition(bodyA); auto posB = getBodyPosition(bodyB);
    SuperAgent::insertJointMesh(glm::vec3(posA.x, posA.y, 0), glm::vec3(posB.x, posB.y, 0));
  
    return j;
}
