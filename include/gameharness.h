#pragma once
#include <stdio.h>
#include "constants.h"
#include "utils.h"
#include "maze.h"
#include "sprite.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
// Simple enum to denote the 4 possible directions
// The sprites can move
enum class Direction
{
    Up = 0,
    Down,
    Left,
    Right
};

// Encapsulates the game, tracks state, player, pellets, ghosts, score, etc
// Things that are tightly game sepcific should go here (e.g. PlayerSprite 
// vs 2DTiledMap which is more generic)
class GameHarness
{
public:
    GameHarness() :
        _fInitialized(false),
        _state(GameState::Title),
        _pSDLRenderer(nullptr),
        _pSDLWindow(nullptr),
        _pTilesTexture(nullptr),
        _pSpriteTexture(nullptr),
        _pMaze(nullptr),
        _pPlayerSprite(nullptr)
    {
    }

    SDL_bool Initialize();  // Needs to be called successfully before Run()
    void Run();             // Main loop

private:
    enum class GameState
    {
        Title,                  // Eventual Title screen
        LoadingLevel,           // Once we add levels, we'll need a way to "load/select" the correct map, etc
        WaitingToStartLevel,    // Starting animation (gives the player a chance to get bearings)
        Running,                // Playing - most time should be in here! :)
        PlayerWarpingOut,       // Exiting maze through warp tunnel
        PlayerWarpingIn,        // Entering maze from warp tunnel
        PlayerDying,            // Got caught by a ghost
        LevelComplete,          // Ate all the pellets on the current level (flashing level animation)
        GameOver,               // All lives are gone - cycles back to title after some time or input
        Exiting                 // App is closing
    };

    // Oneshot timer for state transistions
    class StateTimer
    {
    public:
        StateTimer() : _startTicks(0), _targetTicks(0), _fStarted(false)
        {
        }

        void Start(Uint32 waitTicks)
        {
            SDL_assert(!_fStarted);
            SDL_assert(_startTicks == 0);
            _startTicks = SDL_GetTicks();
            _targetTicks = waitTicks;
            _fStarted = true;
        }

        void Reset() { _fStarted = false; _startTicks = 0; }
        bool IsStarted() { return _fStarted; }
        bool IsDone() { return IsStarted() && (SDL_GetTicks() - _startTicks > _targetTicks); }
    private:
        Uint32 _startTicks;
        Uint32 _targetTicks;
        bool _fStarted;

    };

    // Methods
    void Cleanup();
    void InitializeSprites();
    bool ProcessInput();
    void DoPlayerInputCheck(Direction direction, Uint16 row, Uint16 col, Uint16 animationIndex, double dx, double dy);
    Uint16 HandlePelletCollision();
    void DoPlayerBoundsCheck();
    bool IsPlayerWarpingOut();
    void Render();
    
    // GameState Handlers
    GameState OnLoading();
    GameState OnWaitingToStartLevel();
    GameState OnRunning();
    GameState OnLevelComplete();
    GameState OnPlayerWarpingIn();
    GameState OnPlayerWarpingOut();

    // Members
    bool _fInitialized;                 // Tracks if we've started SDL
    GameState _state;                   // current GameState
    SDL_Renderer *_pSDLRenderer;        // SDL renderer object
    SDL_Window *_pSDLWindow;            // SDL window object
    TextureWrapper *_pTilesTexture;     // Texture that holds the maze tiles
    TextureWrapper *_pSpriteTexture;    // Texture that holds the sprite frames
    Maze *_pMaze;                       // Maze - playing area
    Sprite *_pPlayerSprite;             // The player sprite PacManClone
};
}
}