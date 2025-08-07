#pragma once

#include "Common.h"
#include "GameWindow.h"

// Classes used in game.  For a new class, declare it here and provide a pointer to an object of this class below.  Then, in Game.cpp, 
// include the header.  In the Game constructor, set the pointer to NULL and in Game::Initialise, create a new object.  Don't forget to 
// delete the object in the destructor.   
class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CSphere;
class COpenAssetImportMesh;
class CAudio;
class CCatmullRom;
class CPyramid;
class CCuboid;

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
	void Update();
	void Render();

	// Pointers to game objects.  They will get allocated in Game::Initialise()
	CSkybox *m_pSkybox;
	CCamera *m_pCamera;
	vector <CShaderProgram *> *m_pShaderPrograms;
	CPlane *m_pPlanarTerrain;
	CFreeTypeFont *m_pFtFont;
	CSphere *m_pSphere;
	CPyramid *m_pPyramid;
	CCuboid* m_pCuboid;
	CHighResolutionTimer *m_pHighResolutionTimer;
	CAudio *m_pAudio;

	// Some other member variables
	double m_dt;
	int m_framesPerSecond;
	bool m_appActive;

	// Track members
	float m_currentDistance;
	float m_carCentrelineOffset;
	float m_carSpeed;
	const float INITIAL_CAR_SPEED = 0.04f;
	const float ACCELERATION = 0.00000f;
	const float MAX_CENTRELINE_OFFSET = 18.0f;
	CCatmullRom* m_pCatmullRom;

	// Camera view state
	bool m_topDownView;
	bool m_freeCamera;


public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	static const int FPS = 60;
	void DisplayFrameRate();
	void GameLoop();
	GameWindow m_gameWindow;
	HINSTANCE m_hInstance;
	int m_frameCount;
	double m_elapsedTime;

	float m_gameTime;
	bool m_isGameRunning;
	void RenderHUD();

	// Pickup system
	void InitializePickups();
	void UpdatePickups();

	static const int NUM_PICKUPS = 20;
	static const int PICKUP_HOVER_HEIGHT = 1;
	static const int TRACK_WIDTH = 20;
	const float PICKUP_INACTIVE_TIME = 1.0f;

	struct Pickup {
		glm::vec3 position;
		float trackDistance;    // Distance along track
		float lateralOffset;    // Distance from centreline
		bool active;
		float inactiveTimer;
	};

	vector<Pickup> m_pickups;
	float m_lastCarDistance;

	// Start light members
	bool m_startSequenceActive;
	float m_startSequenceTimer;
	std::vector<bool> m_startLightStates;
	glm::vec4 m_startLightPositions[3];
	bool m_goLightActive;

	bool m_fogEnabled;

	// Lap time HUD component members
	float m_currentLapTime;
	float m_fastestLapTime;
	int m_currentLap;
	bool m_firstLapCompleted;

};
