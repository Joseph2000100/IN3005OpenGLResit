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
	COpenAssetImportMesh *m_pBarrelMesh;
	COpenAssetImportMesh *m_pHorseMesh;
	COpenAssetImportMesh *m_pCarMesh;
	COpenAssetImportMesh *m_pBarrierMesh;
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
	float m_cameraSpeed;
	float m_cameraRotation;
	CCatmullRom* m_pCatmullRom;

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
	float m_playerSpeed;
	bool m_isGameRunning;
	void RenderHUD();


};
