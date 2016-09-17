#include "include/gameharness.h"

using namespace XplatGameTutorial::PacManClone;

// Start up SDL and load our textures - the stuff we'll need for the entire process lifetime
SDL_bool GameHarness::Initialize()
{
    SDL_assert(_fInitialized == false);
    SDL_bool result = SDL_FALSE;
    if (InitializeSDL(&_pSDLWindow, &_pSDLRenderer) == SDL_TRUE)
    {
        // Load our textures
        SDL_Color colorKey = Constants::SDLColorMagenta;
        _pTilesTexture = new TextureWrapper(Constants::TilesImage, SDL_strlen(Constants::TilesImage), _pSDLRenderer, nullptr);
        _pSpriteTexture = new TextureWrapper(Constants::SpritesImage, SDL_strlen(Constants::SpritesImage), _pSDLRenderer, &colorKey);

        if (_pTilesTexture->IsNull() || _pSpriteTexture->IsNull())
        {
            printf("Failed to load one or more textures\n");
        }
        else
        {
            _fInitialized = true;
            result = SDL_TRUE;
        }
    }
    return result;
}

// Main loop, process window messages and dispatch to the current GameState handler
void GameHarness::Run()
{
    SDL_assert(_fInitialized);
    static bool fQuit = false;
    SDL_Event eventSDL;

    Uint32 startTicks;
    while (!fQuit)
    {
        startTicks = SDL_GetTicks();
        while (SDL_PollEvent(&eventSDL) != 0)
        {
            if (eventSDL.type == SDL_QUIT)
            {
                fQuit = true;
            }
        }

        if (!fQuit)
        {
            switch (_state)
            {
            case GameState::Title:
                // Skipping this for now
                _state = GameState::LoadingLevel;
                break;
            case GameState::LoadingLevel:
                // Loads the current maze and the sprites if needed
                _state = OnLoading();
                break;
            case GameState::WaitingToStartLevel:
                // Small delay before level starts
                _state = OnWaitingToStartLevel();
                break;
            case GameState::PlayerWarpingOut:
                _state = OnPlayerWarpingOut();
                break;
            case GameState::PlayerWarpingIn:
                _state = OnPlayerWarpingIn();
                break;
            case GameState::Running:
                // Normal gameplay
                _state = OnRunning();
                break;
            case GameState::PlayerDying:
                // Death animation, skip for now since no ghosts
                break;
            case GameState::LevelComplete:
                // Flashing level animation
                _state = OnLevelComplete();
                break;
            case GameState::GameOver:
                // Final drawing of level, score, etc
                break;
            case GameState::Exiting:
                fQuit = true;
                break;
            }

            // Draw the current frame
            Render();

            // TIMING
            // Fix this at ~c_framesPerSecond
            Uint32 endTicks = SDL_GetTicks();
            Uint32 elapsedTicks = endTicks - startTicks;
            if (elapsedTicks < Constants::TicksPerFrame)
            {
                SDL_Delay(Constants::TicksPerFrame - elapsedTicks);
            }
        }
    }

    // cleanup
    Cleanup();
}

void GameHarness::Cleanup()
{
    SDL_assert(_fInitialized);
    SafeDelete<TextureWrapper>(_pTilesTexture);
    SafeDelete<TextureWrapper>(_pSpriteTexture);
    SafeDelete<Maze>(_pMaze);
    SafeDelete<Sprite>(_pPlayerSprite);

    SDL_DestroyRenderer(_pSDLRenderer);
    _pSDLRenderer = nullptr;

    SDL_DestroyWindow(_pSDLWindow);
    _pSDLWindow = nullptr;

    IMG_Quit();
    SDL_Quit();
    _fInitialized = false;
}

void GameHarness::InitializeSprites()
{
    SDL_assert(_fInitialized);
    // Only need to do this once
    if (_pPlayerSprite == nullptr)
    {
        // Declare and initialize sprite object(s)
        _pPlayerSprite = new Sprite(_pSpriteTexture, Constants::PlayerSpriteWidth, Constants::PlayerSpriteHeight,
            Constants::PlayerTotalFrameCount, Constants::PlayerTotalAnimationCount);

        _pPlayerSprite->LoadFrames(0, 0, 0, 10);
        _pPlayerSprite->LoadFrames(10, 0, Constants::PlayerSpriteHeight, 10);
        _pPlayerSprite->LoadAnimationSequence(Constants::AnimationIndexLeft, AnimationType::Loop, Constants::PlayerAnimation_LEFT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
        _pPlayerSprite->LoadAnimationSequence(Constants::AnimationIndexRight, AnimationType::Loop, Constants::PlayerAnimation_RIGHT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
        _pPlayerSprite->LoadAnimationSequence(Constants::AnimationIndexUp, AnimationType::Loop, Constants::PlayerAnimation_UP, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
        _pPlayerSprite->LoadAnimationSequence(Constants::AnimationIndexDown, AnimationType::Loop, Constants::PlayerAnimation_DOWN, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
        _pPlayerSprite->LoadAnimationSequence(Constants::AnimationIndexDeath, AnimationType::Once, Constants::PlayerAnimation_DEATH, Constants::PlayerAnimationDeathFrameCount, Constants::PlayerAnimationSpeed);
        _pPlayerSprite->SetFrameOffset(1 - (Constants::PlayerSpriteWidth / 2), 1 - (Constants::PlayerSpriteHeight / 2));
    }

    // Do this every time
    _pPlayerSprite->SetAnimation(Constants::AnimationIndexLeft);
    SDL_Point playerStartCoord = _pMaze->GetTileCoordinates(Constants::PlayerStartRow, Constants::PlayerStartCol);
    _pPlayerSprite->ResetPosition(playerStartCoord.x, playerStartCoord.y);
    _pPlayerSprite->SetVelocity(-1.5, 0);
}

// Handle any keyboard input.  The basic logic here is
// 1)  If a directional key is pressed
// 2)  Check the cell adjacent based on direction
// 3)  If the new direction is open, place the sprite along the centerline and
//     set its new velocity
// 4)  If ESC is hit, signal quit
//
// pInputSprite is the temporary graphical helper which will go away - it shows directions pressed
bool GameHarness::ProcessInput()
{
    bool fResult = false;

    // All it takes to get the key states.  The array is valid within SDL while running
    const Uint8 *pCurrentKeyState = SDL_GetKeyboardState(nullptr);

    // Get the player's info before any input is taken
    SDL_Point playerPreInputPoint = { static_cast<int>(_pPlayerSprite->X()), static_cast<int>(_pPlayerSprite->Y()) };
    Uint16 playerPreInputRow = 0;
    Uint16 playerPreInputCol = 0;
    _pMaze->GetTileRowCol(playerPreInputPoint, playerPreInputRow, playerPreInputCol);

     // LOGIC
    // Check if a direction key is down (or WASD) and then process it with our helper
    // which will handle collision, etc
    if (pCurrentKeyState[SDL_SCANCODE_UP] || pCurrentKeyState[SDL_SCANCODE_W])
    {
        DoPlayerInputCheck(Direction::Up, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexUp, 0, -1.5);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_DOWN] || pCurrentKeyState[SDL_SCANCODE_S])
    {
        DoPlayerInputCheck(Direction::Down, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexDown, 0, 1.5);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_LEFT] || pCurrentKeyState[SDL_SCANCODE_A])
    {
        DoPlayerInputCheck(Direction::Left, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexLeft, -1.5, 0);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_RIGHT] || pCurrentKeyState[SDL_SCANCODE_D])
    {
        DoPlayerInputCheck(Direction::Right, playerPreInputRow, playerPreInputCol, Constants::AnimationIndexRight, 1.5, 0);
    }
    else if (pCurrentKeyState[SDL_SCANCODE_ESCAPE])
    {
        printf("ESC hit - exiting main loop...\n");
        fResult = true;
    }
    return fResult;
}

// Given a player's current state (location, direction, animation) check if the player can move in a given direction, and if
// so position the player on the new track at the new velocity
void GameHarness::DoPlayerInputCheck(Direction direction, Uint16 row, Uint16 col, Uint16 animationIndex, double dx, double dy)
{
    // Helper lambda to check the map in a given direction.  I put it here instead of another helper
    // because it's only useful here now.  Plus I wanted to check the c++11 feature on both compilers :)
    auto CanMove = [](Direction direction, Uint16 row, Uint16 col) -> SDL_bool
    {
        SDL_bool fResult = SDL_FALSE;
        // Adjust the [row][col] to look at based on direction
        if (direction == Direction::Up)
        {
            row--;
        }
        else if (direction == Direction::Down)
        {
            row++;
        }
        else if (direction == Direction::Left)
        {
            col--;
        }
        else if (direction == Direction::Right)
        {
            col++;
        }

        // Check the map, 0s are legal free space
        if (Constants::CollisionMap[row * Constants::MapCols + col] == 0)
        {
            fResult = SDL_TRUE;
        }
        return fResult;
    };

    // If we can move and we're not already moving in the direction
    if ((CanMove(direction, row, col) == SDL_TRUE) &&
        (_pPlayerSprite->CurrentAnimation() != animationIndex))
    {
        // Set a new animation and position the player with a new velocity
        _pPlayerSprite->SetAnimation(animationIndex);
        SDL_Point tilePoint = _pMaze->GetTileCoordinates(row, col);
        _pPlayerSprite->ResetPosition(tilePoint.x, tilePoint.y);
        _pPlayerSprite->SetVelocity(dx, dy);
    }
}

Uint16 GameHarness::HandlePelletCollision()
{
    Uint16 ret = 0;
    SDL_Point playerPoint = { static_cast<int>(_pPlayerSprite->X()), static_cast<int>(_pPlayerSprite->Y()) };
    Uint16 row = 0;
    Uint16 col = 0;
    _pMaze->GetTileRowCol(playerPoint, row, col);

    if (_pMaze->IsTilePellet(row, col))
    {
        _pMaze->EatPellet(row, col);
        ret++;
    }
    return ret;
}

// Even if no input is pressed, the player may run into a wall, so we need to handle collisions
// after the player is moved
void GameHarness::DoPlayerBoundsCheck()
{
    SDL_Point playerPoint = { static_cast<int>(_pPlayerSprite->X()), static_cast<int>(_pPlayerSprite->Y()) };

    // Need to check bounds in direction moving (account for width of half the sprite)
    // This is because the sprite is double the size of the tiles and placed along the centerline
    // in the direction of movement.  So 1/2 of its size in a given direction is the "edge" of the
    // sprite on the screen (minus a pixel or 2 of transparency)
    if (_pPlayerSprite->DX() != 0) // If we're not moving in this axis, then don't bother
    {
        if (_pPlayerSprite->DX() < 0)
        {
            playerPoint.x -= (Constants::PlayerSpriteWidth / 2) - Constants::TileWidth / 2;
        }
        else
        {
            playerPoint.x += (Constants::PlayerSpriteWidth / 2) - Constants::TileWidth / 2;
        }
    }
    else  // We cann't be moving in both directions at once
    {
        if (_pPlayerSprite->DY() < 0)  // Same logic for y axis if moving
        {
            playerPoint.y -= (Constants::PlayerSpriteHeight / 2) - Constants::TileHeight / 2;
        }
        else
        {
            playerPoint.y += (Constants::PlayerSpriteHeight / 2) - Constants::TileHeight / 2;
        }
    }

    // Now get the row, col we're in
    Uint16 row = 0;
    Uint16 col = 0;
    _pMaze->GetTileRowCol(playerPoint, row, col);

    if (_pMaze->IsTileSolid(row, col))
    {
        // If we wandered into a bad cell, stop
        _pPlayerSprite->SetVelocity(0, 0);
    }
}

// Called from normal play so just check the current cell
bool GameHarness::IsPlayerWarpingOut()
{
    SDL_Point playerPoint = { static_cast<int>(_pPlayerSprite->X()), static_cast<int>(_pPlayerSprite->Y()) };
    Uint16 row, col;
    _pMaze->GetTileRowCol(playerPoint, row, col);
    return ((row == 17) && ((col == 0) || (col == 27)));
}


void GameHarness::Render()
{
    SDL_RenderClear(_pSDLRenderer);
    if (_pMaze != nullptr)
    {
        _pMaze->Render(_pSDLRenderer);
    }

    if (_pPlayerSprite != nullptr)
    {
        _pPlayerSprite->Render(_pSDLRenderer);
    }

    SDL_RenderPresent(_pSDLRenderer);
}

GameHarness::GameState GameHarness::OnLoading()
{
    // This should be know, but it should also match what we just queried
    SDL_assert(_pTilesTexture->Width() == Constants::TileTextureWidth);
    SDL_assert(_pTilesTexture->Height() == Constants::TileTextureHeight);
    SDL_Rect textureRect{ 0, 0, Constants::TileTextureWidth, Constants::TileTextureHeight };

    SDL_SetTextureColorMod(_pTilesTexture->Ptr(), 255, 255, 255);

    // Initialize our tiled map object
    SafeDelete(_pMaze);
    _pMaze = new Maze(Constants::MapRows, Constants::MapCols, Constants::ScreenWidth, Constants::ScreenHeight);

    _pMaze->Initialize(textureRect, { 0, 0,  Constants::TileWidth,  Constants::TileHeight }, _pTilesTexture->Ptr(),
        Constants::MapIndicies, Constants::MapRows *  Constants::MapCols);

    // Clip around the maze so nothing draws there (this will help with the wrap around for example)
    SDL_Rect mapBounds = _pMaze->GetMapBounds();
    if (SDL_RenderSetClipRect(_pSDLRenderer, &mapBounds) != 0)
    {
        printf("SDL_RenderSetClipRect() failed, error = %s\n", SDL_GetError());
    }
    else
    {
        // Initialize our sprites
        InitializeSprites();
    }
    return GameState::WaitingToStartLevel;
}

// This is the traditional delay before the level starts, normally you hear the little
// tune that signals play is about to begin, then you transition.  We have no sound yet
// so just delay the game a bit
GameHarness::GameState GameHarness::OnWaitingToStartLevel()
{
    static StateTimer timer;
    
    if (!timer.IsStarted())
    {
        timer.Start(Constants::LevelLoadDelay);
    }

    if (timer.IsDone())
    {
        timer.Reset();
        return GameState::Running;
    }
    return GameState::WaitingToStartLevel;
}

// Normal game play, check for collisions, update based on input, eventually the ghosts
// and their updates will need to be in here as well.  
GameHarness::GameState GameHarness::OnRunning()
{
    static Uint16 pelletsEaten = 0;
    GameState stateResult = GameState::Running;

    // INPUT
    bool fQuit = ProcessInput();
    if (!fQuit)
    {
        // UPDATE
        _pPlayerSprite->Update();

        // COLLISIONS
        pelletsEaten += HandlePelletCollision();
        if (pelletsEaten == Constants::TotalPellets)
        {
            pelletsEaten = 0;
            return GameState::LevelComplete;
        }

        // BOUNDS CHECK
        // Warping?
        if (IsPlayerWarpingOut())
        {
            return GameState::PlayerWarpingOut;
        }
        
        // We still need to check if the player has wandered into a wall
        DoPlayerBoundsCheck();
    }
    else
    {
        // Input told us to exit above
        stateResult = GameState::Exiting;
    }
    return stateResult;
}

// All 244 pellets have been eaten, so we briefly flash the screen before moving to the
// next level.  We only have the one level, so it just restarts
GameHarness::GameState GameHarness::OnLevelComplete()
{
    static StateTimer timer;
    static Uint16 counter = 0;
    static bool flip;

    if (!timer.IsStarted())
    {
        counter = 0;
        flip = false;
        timer.Start(Constants::LevelCompleteDelay);
    }
    
    if (counter++ > 60)
    {
        counter = 0;
        flip = !flip;
    }

    // This will add a blue multiplier to the texture, making the shade chage.
    // We flip this back and forth roughly every second until the overall timer is done.
    SDL_SetTextureColorMod(_pTilesTexture->Ptr(), 255, 255, flip ? 100 : 255);
    
    if (timer.IsDone())
    {
        timer.Reset();
        return GameState::LoadingLevel;
    }
    return GameState::LevelComplete;
}

// Assume control of the player sprite while warping in.  Control is returned to 
// the player once we're 1 col "in"
GameHarness::GameState GameHarness::OnPlayerWarpingIn()
{
    // Maintain current velocity until we're back in frame
    _pPlayerSprite->Update();

    SDL_Point playerPoint = { static_cast<int>(_pPlayerSprite->X()), static_cast<int>(_pPlayerSprite->Y()) };
    Uint16 row, col;
    _pMaze->GetTileRowCol(playerPoint, row, col);
    if ((row == Constants::WarpRow) && ((col == 1) || (col == Constants::MapCols - 2)))
    {
        _state = GameState::Running;
    }
    
    return _state;
}

// Assume control of the player while warping out.  Once the sprite is off the visible
// screen, we reposition it on the other side of the map and transition to "WarpingIn"
GameHarness::GameState GameHarness::OnPlayerWarpingOut()
{
    // Maintain current velocity until we're out of frame
    _pPlayerSprite->Update();
    
    SDL_Rect mapRect = _pMaze->GetMapBounds();
    if (_pPlayerSprite->X() > mapRect.x + mapRect.w + Constants::PlayerSpriteWidth)
    {
        _pPlayerSprite->ResetPosition(mapRect.x - Constants::PlayerSpriteWidth, _pPlayerSprite->Y());
        _state = GameState::PlayerWarpingIn;
    }
    else if (_pPlayerSprite->X() < mapRect.x - Constants::PlayerSpriteWidth)
    {
        _pPlayerSprite->ResetPosition(mapRect.x + mapRect.w + Constants::PlayerSpriteWidth, _pPlayerSprite->Y());
        _state = GameState::PlayerWarpingIn;
    }
    return _state;
}