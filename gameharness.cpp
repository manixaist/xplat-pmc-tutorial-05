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
    SafeDelete<Player>(_pPlayer);
    SafeDelete<Blinky>(_pBlinky);

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
    
    if (_pPlayer == nullptr)
    {
        _pPlayer = new Player(_pSpriteTexture);
        _pPlayer->Initialize();
    }
    _pPlayer->Reset(_pMaze);

    if (_pBlinky == nullptr)
    {
        _pBlinky = new Blinky(_pSpriteTexture);
        _pBlinky->Initialize();
    }
    _pBlinky->Reset(_pMaze);
}

// Record key presses we care about
bool GameHarness::ProcessInput(Direction *pInputDirection)
{
    *pInputDirection = Direction::None;
    bool fResult = false;

    // All it takes to get the key states.  The array is valid within SDL while running
    const Uint8 *pCurrentKeyState = SDL_GetKeyboardState(nullptr);

    if (pCurrentKeyState[SDL_SCANCODE_UP] || pCurrentKeyState[SDL_SCANCODE_W])
    {
        *pInputDirection = Direction::Up;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_DOWN] || pCurrentKeyState[SDL_SCANCODE_S])
    {
        *pInputDirection = Direction::Down;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_LEFT] || pCurrentKeyState[SDL_SCANCODE_A])
    {
        *pInputDirection = Direction::Left;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_RIGHT] || pCurrentKeyState[SDL_SCANCODE_D])
    {
        *pInputDirection = Direction::Right;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_ESCAPE])
    {
        printf("ESC hit - exiting main loop...\n");
        fResult = true;
    }
    return fResult;
}

Uint16 GameHarness::HandlePelletCollision()
{
    Uint16 ret = 0;
    SDL_Point playerPoint = { static_cast<int>(_pPlayer->X()), static_cast<int>(_pPlayer->Y()) };
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

void GameHarness::Render()
{
    SDL_RenderClear(_pSDLRenderer);
    if (_pMaze != nullptr)
    {
        _pMaze->Render(_pSDLRenderer);
    }

    if (_pPlayer != nullptr)
    {
        _pPlayer->Render(_pSDLRenderer);
    }

    if (_pBlinky != nullptr)
    {
        _pBlinky->Render(_pSDLRenderer);
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
    Direction inputDirection = Direction::None;
    bool fQuit = ProcessInput(&inputDirection);
    if (!fQuit)
    {
        // UPDATE
        _pPlayer->Update(_pMaze, inputDirection); 
        _pBlinky->Update(_pPlayer, _pMaze);
        
        // COLLISIONS
        pelletsEaten += HandlePelletCollision();
        if (pelletsEaten == Constants::TotalPellets)
        {
            pelletsEaten = 0;
            return GameState::LevelComplete;
        }
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
