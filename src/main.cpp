#include "Engine/SuperUltraMega.h"

#include <Windows.h>
#include <chrono>
#include <iostream>

// GET RID OF UNNECESSARY VERTEX INFO LIKE NORMALS
// SOME WARNING MSG SYSTEM
// GET RID OF RECT.OBJ IN VULKAN FOLDER AND AUTO LOADING IT FOR VERTEX BUFFER
// ADD CATCHES TO STD::EXCEPTIONS IN LOADVERTEXDATA/TEXTUREDATA ETC
// SPRITES JUST HOLDING VERTEX DATA?
// SOME CONSTANT TILE SIZE
// FIND BETTER WAY FOR LOADING ANIMATION TEXTURES
// IF A CLASS CONTAINS A POINTER TO HEAP DATA IS SOMETHING STILL CONST IF IT CHANGES THAT HEAP DATA?
// Input callbacks
// Window class
// Shared ptrs for RAIS	
// have scene hold texture/map data?
// Fix headers - FolderName.h for each, SuperUltraMega.h file
// Engine->createscene instead of get scene?
// Vulkan quad drawing enabled?
// tTimestep
// More effiecient way to bind multiple vertex buffers?
// Clearing vertex buffers every frame (takes long time just allocate once)

//////////////////// FIX UR CODE
// //////////////// ALSO EXCEPTIONS

// ====================== SuperUltraMega2D Engine Example ====================== //
int main()
{
	// =========== Initialize the engine ===========
	Mega::Engine engine = Mega::CreateEngine();
	Mega::Renderer* renderer = engine.GetRenderer();
	Mega::Scene* scene = engine.GetScene();

	// =========== Load shit ===========
	// Maps
	Mega::Map map = scene->LoadMap("Assets/Maps/TestMap");

	// Sprites
	Mega::Texture hoopla = renderer->LoadTexture("Assets/Textures/hoopla.jpg");
	Mega::Sprite sprite = Mega::Sprite(hoopla);

	// Animations
	Mega::AnimationPlayer anim = scene->LoadAnimationPlayer("Assets/Animations/TestAnimation");
	anim.PlayAnimation("Walk");

	/*Mega::Input::PollEvents();
	Mega::Input::IsKeyDown(KEY_A);
	Mega::Input::KeyCallBack(KEY_A)
	{

	}*/

	// =========== Game loop example ===========
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	COORD cursorPos = info.dwCursorPosition;

	float afps = 0.0f;
	uint32_t afc = 0;
	uint32_t frameCount = 0;
	while ( !glfwWindowShouldClose(engine.GetApplicationWindow()) ) 
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		// ================================ //
		// Getting input:
		Mega::Input::PollEvents();
		//Mega::Input::IsKeyPressed(GLFW_KEY_W);

		// Update:
		anim.Update(16);

		// Drawing:
		scene->Clear();

		scene->Draw(sprite);
		scene->Draw(map);
		scene->Draw(anim);

		scene->Display();
		// ================================== //

		auto fDeltaTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count();
		if ((frameCount + 1) % 30 == 0) { // Slow down output (easier to read)
			std::cout << "FPS: " << 1 / (fDeltaTime / 1000) << std::endl; // Milli per frame to fps
			afps += 1 / (fDeltaTime / 1000);
			afc += 1;
		}
		frameCount++;

		SetConsoleCursorPosition(hConsole, cursorPos);
	}
	std::cout << "Average FPS: " << afps / afc << std::endl;

	// =========== Cleanup ===========
	Mega::DestroyEngine(engine);
}