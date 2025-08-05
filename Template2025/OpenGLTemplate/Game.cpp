/*
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting,
 different camera controls, different shaders, etc.

 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk)

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

 version 6.1a 13/02/2022 - Sorted out Release mode and a few small compiler warnings
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

*/


#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "CatmullRom.h"
#include "Pyramid.h"
#include "Cuboid.h"

// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pBarrelMesh = NULL;
	m_pHorseMesh = NULL;
	m_pCarMesh = NULL;
	m_pBarrierMesh = NULL;
	m_pSphere = NULL;
	m_pPyramid = NULL;
	m_pCuboid = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;

	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;

	m_gameTime = 0.0f;
	m_playerSpeed = 0.0f;
	m_isGameRunning = false;
	m_topDownView = false;
	m_freeCamera = false;

	m_currentDistance = 0.0f;
	m_cameraSpeed = 0.1f;
	m_cameraRotation = 0.0f;
	m_pCatmullRom = NULL;

	m_startSequenceActive = false;
	m_startSequenceTimer = 0.0f;
	m_startLightStates = std::vector<bool>(3, false);
	m_goLightActive = false;

	m_startLightPositions[0] = glm::vec4(-2.0f, 10.0f, 0.0f, 1.0f);
	m_startLightPositions[1] = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	m_startLightPositions[2] = glm::vec4(2.0f, 10.0f, 0.0f, 1.0f);

	m_fogEnabled = false;
}

// Destructor
Game::~Game()
{
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pBarrelMesh;
	delete m_pHorseMesh;
	delete m_pCarMesh;
	delete m_pBarrierMesh;
	delete m_pSphere;
	delete m_pAudio;
	delete m_pCatmullRom;
	delete m_pPyramid;
	delete m_pCuboid;

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise()
{
	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram*>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pBarrelMesh = new COpenAssetImportMesh;
	m_pHorseMesh = new COpenAssetImportMesh;
	m_pCarMesh = new COpenAssetImportMesh;
	m_pBarrierMesh = new COpenAssetImportMesh;
	m_pSphere = new CSphere;
	m_pPyramid = new CPyramid;
	m_pCuboid = new CCuboid;
	m_pAudio = new CAudio;

	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height);
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)width / (float)height, 0.5f, 5000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	for (int i = 0; i < (int)sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int)sShaderFileNames[i].size() - 4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\" + sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram* pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram* pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	m_pSkybox->Create(2500.0f);

	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);

	// Load some meshes in OBJ format
	//m_pBarrelMesh->Load("resources\\models\\Barrel\\Barrel02.obj");  // Downloaded from http://www.psionicgames.com/?page_id=24 on 24 Jan 2013
	//m_pHorseMesh->Load("resources\\models\\Horse\\Horse2.obj");  // Downloaded from http://opengameart.org/content/horse-lowpoly on 24 Jan 2013
	//m_pCarMesh->Load("resources\\models\\Car\\car.obj");  
	//m_pBarrierMesh->Load("resources\\models\\Barrier\\cone.obj"); 

	// Create a pyramid
	m_pPyramid->Create(2.0f, 3.0f);

	// Create a cuboid
	m_pCuboid->Create(2.0f, 3.0f, 1.0f);

	// Create a sphere
	m_pSphere->Create("resources\\textures\\", "dirtpile01.jpg", 25, 25);  // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013
	glEnable(GL_CULL_FACE);

	// Create the catmull rom spline
	m_pCatmullRom = new CCatmullRom();
	m_pCatmullRom->CreateCentreline();
	m_pCatmullRom->CreateOffsetCurves();
	m_pCatmullRom->CreateTrack();

	// Initialise audio and play background music
	m_pAudio->Initialise();
	m_pAudio->LoadEventSound("resources\\Audio\\Boing.wav");					// Royalty free sound from freesound.org
	m_pAudio->LoadMusicStream("resources\\Audio\\DST-Garote.mp3");	// Royalty free music from http://www.nosoapradio.us/
	//m_pAudio->PlayMusicStream();
}

// Render method runs repeatedly in a loop
void Game::Render()
{

	// Clear the buffers and enable depth testing (z-buffering)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);
	// Note: cubemap and non-cubemap textures should not be mixed in the same texture unit.  Setting unit 10 to be a cubemap texture.
	int cubeMapTextureUnit = 10;
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);

	pMainProgram->SetUniform("fogEnabled", m_fogEnabled);
	pMainProgram->SetUniform("fogDensity", 0.015f);  // Fog thickness value
	pMainProgram->SetUniform("fogColor", glm::vec3(0.5f, 0.5f, 0.5f));

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);


	// Set light and materials in main shader program
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", viewMatrix * lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property


	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
	pMainProgram->SetUniform("renderSkybox", true);
	// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
	glm::vec3 vEye = m_pCamera->GetPosition();
	modelViewMatrixStack.Translate(vEye);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pSkybox->Render(cubeMapTextureUnit);
	pMainProgram->SetUniform("renderSkybox", false);
	modelViewMatrixStack.Pop();

	// Render the planar terrain
	modelViewMatrixStack.Push();
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();


	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	

	// Render the start lights
	if (m_startSequenceActive || m_goLightActive) {
		pMainProgram->SetUniform("bUseTexture", false);

		// Render each light sphere
		for (int i = 0; i < 3; i++) {
			modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(glm::vec3(-2.0f + i * 2.0f, 10.0f, 0.0f));
			modelViewMatrixStack.Scale(glm::vec3(0.3f));

			if (m_goLightActive) {
				// Green for GO
				pMainProgram->SetUniform("material1.Ma", glm::vec3(0.0f, 1.0f, 0.0f));
				pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f, 1.0f, 0.0f));
				pMainProgram->SetUniform("material1.Me", glm::vec3(0.0f, 5.0f, 0.0f));
			}
			else if (m_startLightStates[i]) {
				// Red for countdown
				pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f, 0.0f, 0.0f));
				pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f, 0.0f, 0.0f));
				pMainProgram->SetUniform("material1.Me", glm::vec3(5.0f, 0.0f, 0.0f));
			}
			else {
				// Dark gray for off state
				pMainProgram->SetUniform("material1.Ma", glm::vec3(0.2f));
				pMainProgram->SetUniform("material1.Md", glm::vec3(0.2f));
				pMainProgram->SetUniform("material1.Me", glm::vec3(0.0f));
			}
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_pSphere->Render();
			modelViewMatrixStack.Pop();
		}
		pMainProgram->SetUniform("bUseTexture", true);
	}


	// Render the horse 
	/*modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
		modelViewMatrixStack.Scale(2.5f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pHorseMesh->Render();
	modelViewMatrixStack.Pop();


	// Render the barrel
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(100.0f, 0.0f, 0.0f));
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
	modelViewMatrixStack.Pop();
	*/

	// Render the car
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
	modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
	modelViewMatrixStack.Scale(1.0f);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pCarMesh->Render();
	modelViewMatrixStack.Pop();

	// Render the barrier
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(20.0f, 0.0f, 0.0f));
	modelViewMatrixStack.Scale(1.0f);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pBarrierMesh->Render();
	modelViewMatrixStack.Pop();

	/*
	// Render the sphere
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(0.0f, 2.0f, 150.0f));
		modelViewMatrixStack.Scale(2.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pSphere->Render();
	modelViewMatrixStack.Pop();
	*/

	pMainProgram->SetUniform("bUseTexture", false);  // Turn off texturing
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.2f, 0.5f, 0.2f));  // Ambient color (green-ish)
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.2f, 0.5f, 0.2f));  // Diffuse color (green-ish)
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));  // Specular color (white)
	pMainProgram->SetUniform("material1.shininess", 50.0f);     // Shininess

	// Render the Pyramid
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(0.0f, 2.0f, -150.0f));
	modelViewMatrixStack.Scale(1.0f);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pPyramid->Render();
	modelViewMatrixStack.Pop();

	pMainProgram->SetUniform("bUseTexture", true);

	// Render the cuboid
	pMainProgram->SetUniform("bUseTexture", false);
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(glm::vec3(5.0f, 2.0f, -150.0f));  // Position it where you want
	modelViewMatrixStack.Scale(1.0f);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pCuboid->Render();
	modelViewMatrixStack.Pop();
	pMainProgram->SetUniform("bUseTexture", true);


	// Render the track
	m_pCatmullRom->RenderCentreline();
	m_pCatmullRom->RenderOffsetCurves();
	m_pCatmullRom->RenderTrack();

	RenderHUD();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());

}

// Update method runs repeatedly with the Render method
void Game::Update()
{
	if (m_freeCamera) {
		// Allow camera to be controlled freely
		m_pCamera->Update(m_dt);
	}
	else if (m_topDownView) {
		// Provides a top down view
		glm::vec3 cameraPos(65.0f, 220.0f, 70.0f);
		glm::vec3 lookAtPoint(65.0f, 0.0f, 70.0f);
		glm::vec3 upVector(0.0f, 0.0f, -1.0f);
		m_pCamera->Set(cameraPos, lookAtPoint, upVector);
	}
	else {

		// Your existing camera update code for normal view
		glm::vec3 currentPos, nextPos;
		m_pCatmullRom->Sample(m_currentDistance, currentPos);
		m_pCatmullRom->Sample(m_currentDistance + 1.0f, nextPos);

		// Calculate TNB frame
		glm::vec3 T = glm::normalize(nextPos - currentPos);
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 rotatedUp = glm::rotate(worldUp, m_cameraRotation, T);
		glm::vec3 N = glm::normalize(glm::cross(T, rotatedUp));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		glm::vec3 cameraPos = currentPos + B * 2.0f;
		glm::vec3 lookAtPoint = currentPos + T * 10.0f;

		m_pCamera->Set(cameraPos, lookAtPoint, rotatedUp);

		// Only update distance when not in top-down view
		m_currentDistance += m_dt * m_cameraSpeed;
	}

	// Start Lights
	if (m_startSequenceActive) {
		m_startSequenceTimer += m_dt / 1000.0f; // Convert to seconds

		// Turn on lights sequentially
		if (m_startSequenceTimer >= 1.0f && !m_startLightStates[0]) {
			m_startLightStates[0] = true;
		}
		if (m_startSequenceTimer >= 2.0f && !m_startLightStates[1]) {
			m_startLightStates[1] = true;
		}
		if (m_startSequenceTimer >= 3.0f && !m_startLightStates[2]) {
			m_startLightStates[2] = true;
		}

		// Flash all lights for "GO"
		if (m_startSequenceTimer >= 4.0f && !m_goLightActive) {
			m_goLightActive = true;
		}

		// End sequence and turn off lights
		if (m_startSequenceTimer >= 4.5f) {
			m_startSequenceActive = false;
			m_startLightStates = std::vector<bool>(3, false);
			m_goLightActive = false;
		}
	}

	// Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
	m_pCamera->Update(m_dt);

	if (m_isGameRunning) {
		m_pPyramid->Update(m_dt);
	}

	m_pAudio->Update();

	if (m_isGameRunning) {
		m_gameTime += (float)m_dt / 1000.0f;
	}
	m_playerSpeed = 100;
}



void Game::DisplayFrameRate()
{


	CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if (m_elapsedTime > 1000)
	{
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
	}

	if (m_framesPerSecond > 0) {
		// Use the font shader program and render the text
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 20, 20, "FPS: %d", m_framesPerSecond);
	}
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/


	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();


}

void Game::RenderHUD()
{
	// Get window dimensions
	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;
	int width = dimensions.right - dimensions.left;

	// Use the font shader program
	CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];
	fontProgram->UseProgram();
	glDisable(GL_DEPTH_TEST);

	// Set up the orthographic projection matrix
	fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
	fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// Format time as minutes:seconds.milliseconds
	int minutes = (int)m_gameTime / 60;
	int seconds = (int)m_gameTime % 60;
	int milliseconds = (int)((m_gameTime - (int)m_gameTime) * 100);

	// Render time in top left corner
	m_pFtFont->Render(20, height - 40, 20, "Time: %02d:%02d.%02d", minutes, seconds, milliseconds);

	// Render speed in top right corner
	m_pFtFont->Render(width - 200, height - 40, 20, "Speed: %.1f km/h", m_playerSpeed);
}


WPARAM Game::Execute()
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if (!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();


	MSG msg;

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (m_appActive) {
			GameLoop();
		}
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch (LOWORD(w_param))
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			m_appActive = true;
			m_pHighResolutionTimer->Start();
			break;
		case WA_INACTIVE:
			m_appActive = false;
			break;
		}
		break;
	}

	case WM_SIZE:
		RECT dimensions;
		GetClientRect(window, &dimensions);
		m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch (w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case '1':
			m_pAudio->PlayEventSound();
			break;
		case VK_F1:
			m_pAudio->PlayEventSound();
			break;
		case VK_SPACE:
			m_isGameRunning = !m_isGameRunning;
			break;
		case 'R':
			m_gameTime = 0.0f;
			break;
		case VK_LEFT:
			m_cameraRotation -= m_dt * 0.1f;
			break;
		case VK_RIGHT:
			m_cameraRotation += m_dt * 0.1f;
			break;
		case VK_UP:
			m_cameraSpeed += 0.01f;
			break;
		case VK_DOWN:
			m_cameraSpeed = max(0.0f, m_cameraSpeed - 0.01f);
			break;
		case 'V':
			m_topDownView = !m_topDownView;
			break;
		case 'F':
			m_freeCamera = !m_freeCamera;
			break;
		case 'S':
			if (!m_startSequenceActive) {
				m_startSequenceActive = true;
				m_startSequenceTimer = 0.0f;
				m_startLightStates = std::vector<bool>(3, false);
				m_goLightActive = false;
			}
			break;
		case 'C':
			m_fogEnabled = !m_fogEnabled;
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance()
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance)
{
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int)
{
	Game& game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return int(game.Execute());
}
