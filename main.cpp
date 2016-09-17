// main.cpp : Defines the entry point for the console application.
//
#include "include/gameharness.h"

using namespace XplatGameTutorial::PacManClone;

int main(int /*argc*/, char* /*argv*/[])
{
    GameHarness gameHarness;

    if (gameHarness.Initialize() == SDL_TRUE)
    { 
        gameHarness.Run();
    }
    return 0;
}
