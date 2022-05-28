#include "Engine/SuperUltraMega.h"

#include <iostream>

// GET RID OF UNNECESSARY VERTEX INFO LIKE NORMALS
// SOME WARNING MSG SYSTEM
// USING KEYWORD FOR THINGS LIKE VEC2F sFOR WHOLE NAMSPACE?
// TETXURES DRAWING WITH PUSH CONSTANTS
// ADD NAMESPACES FOR GRAPHICS/OBJECTS FOLDER
// GET RID OF RECT.OBJ IN VULKAN FOLDER AND AUTO LOADING IT FOR VERTEX BUFFER
// ADD CATCHES TO STD::EXCEPTIONS IN LOADVERTEXDATA/TEXTUREDATA ETC
// SPRITES JUST HOLDING VERTEX DATA?
// SOME CONSTANT TILE SIZE
// SCENE STORE MAP? AND HAVE SCENE.DRAW() ITSELF? BETTER FOR WHEN PHYSICS?
// FIND BETTER WAY FOR LOADING ANIMATION TEXTURES
// IF A CLASS CONTAINS A POINTER TO HEAP DATA IS SOMETHING STILL CONST IF IT CHANGES THAT HEAP DATA?
// MAYBE INCLUDE EVERTHING FROM ONE START IN REPO ("Mega/Engine/etc")
// Input callbacks
// Window class
// Mega::CreateScene() Mega::CreateRenderer(GLFWwindow* in_pWindow) maybe
// Have engine load stuff

//////////////////// FIX UR CODE
// //////////////// ALSO EXCEPTIONS

// ====================== SuperUltraMega2D Engine Example ====================== //
int main()
{
	// =========== Initialize the engine ===========
	Mega::Engine* engine = Mega::CreateEngine();
	Mega::Renderer* renderer = engine->GetRenderer();
	Mega::Scene* scene = engine->GetScene();

	// =========== Load shit ===========
	// Maps
	Mega::Map map = scene->LoadMap("Assets/Maps/TestMap");

	// Sprites
	Mega::Texture hoopla = renderer->LoadTexture("Assets/Textures/hoopla.jpg");
	Mega::Sprite sprite = Mega::Sprite(hoopla);

	// Animations
	Mega::AnimationPlayer anim = scene->LoadAnimationPlayer("Assets/Animations/TestAnimation");
	anim.PlayAnimation("Walk");

	// =========== Game loop example ===========
	while (1)
	{
		// Getting input

		// Update
		anim.Update(16);

		// Drawing
		scene->Clear();

		scene->Draw(sprite);
		scene->Draw(map);
		scene->Draw(anim);

		scene->Display();
	}

	// =========== Cleanup ===========
	Mega::DestroyEngine(engine);
}