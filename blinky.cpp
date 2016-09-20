
#include "include/blinky.h"

using namespace XplatGameTutorial::PacManClone;

Blinky::Blinky(TextureWrapper* pTextureWrapper) :
    Ghost(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight,
        Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount)
{
}

bool Blinky::Initialize()
{
    // Each ghost will have its own set of frames, so each will override this method and implement
    // the specific loading data
    LoadFrames(0, 0, 64, Constants::GhostTotalFrameCount);
    LoadAnimationSequence(Constants::AnimationIndexLeft, AnimationType::Loop, Constants::GhostAnimation_LEFT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexRight, AnimationType::Loop, Constants::GhostAnimation_RIGHT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexUp, AnimationType::Loop, Constants::GhostAnimation_UP, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDown, AnimationType::Loop, Constants::GhostAnimation_DOWN, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    SetFrameOffset(1 - (Constants::GhostSpriteWidth / 2), 1 - (Constants::GhostSpriteHeight / 2));
    return true;
}

bool Blinky::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexUp);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::GhostPenRow, Constants::GhostPenCol);
    
    // There is no "penned" mode, just placement will take care of that.  Blinky is the only
    // ghost that is supposed to start outside of the pen, but since he's the only one for now
    // put him inside to test out that code path.
    _currentRow = Constants::GhostPenRow;
    _currentCol = Constants::GhostPenCol;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(0, Constants::GhostBaseSpeed * -1.75);

    SafeDelete<Decision>(_pNextDecision);
    SafeDelete<Decision>(_pCurrentDecision);
    _pCurrentDecision = new Decision(Constants::GhostPenRow, Constants::GhostPenCol, CurrentDirection());
    _penTimer.Reset();
    return true;
}

Direction Blinky::MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze)
{
    // Blinky's target tile is the player's current tile
    // We won't bother with "Elroy" states at the moment
    Direction result = CurrentDirection();

    // The "next" cell is already passed in here, given this location, find the branch
    // That brings us closest to the target cell (the player)
    SDL_Point playerPoint = { static_cast<int>(pPlayer->X()), static_cast<int>(pPlayer->Y()) };
    Uint16 targetRow = 0;
    Uint16 targetCol = 0;
    pMaze->GetTileRowCol(playerPoint, targetRow, targetCol);

    // What is the shortest available exit in cell[nRow, nCol]?
    // we know this cell should be an intersection
    SDL_assert(pMaze->IsTileIntersection(nRow, nCol) == SDL_TRUE);

    // This means there should be at least 2 options to pick from minus the
    // reverse of our current direction which is invalid
    // ...
    struct MAZECELL
    {
        Uint16 row;
        Uint16 col;
        double distance;
        bool valid;
    };
    
    // Direction order should match the Direction Enum for easy array access
    MAZECELL options[] = { 
        { static_cast<Uint16>(nRow - 1), nCol, 0, false }, // UP
        { static_cast<Uint16>(nRow + 1), nCol, 0, false }, // DOWN
        { nRow, static_cast<Uint16>(nCol - 1), 0, false }, // LEFT
        { nRow, static_cast<Uint16>(nCol + 1), 0, false }  // RIGHT
    };

    for (size_t index = 0; index < SDL_arraysize(options); index++)
    {
        options[index].valid = (pMaze->IsTileSolid(options[index].row, options[index].col) == SDL_FALSE);
        if (Opposite(static_cast<Direction>(index)) == CurrentDirection())
        {
            options[index].valid = false; // even though it's non solid
        }

        if (options[index].valid)
        {
            //Distance Cell and Player(P, C)
            SDL_Point point = pMaze->GetTileCoordinates(options[index].row, options[index].col);
            options[index].distance = SDL_sqrt(SDL_abs(pPlayer->X() - point.x) * SDL_abs(pPlayer->X() - point.x) +
                SDL_abs(pPlayer->Y() - point.y) * SDL_abs(pPlayer->Y() - point.y));
        }
    }

    size_t index = 0;
    size_t shortest = 0;
    for (;index < SDL_arraysize(options); index++)
    {
        if (options[index].valid)
        {
            shortest = index;
            break;
        }
    }
        
    for (index = shortest+1 ;index < SDL_arraysize(options); index++)
    {
        if (options[index].distance < options[shortest].distance && options[index].valid)
        {
            shortest = index;
        }
    }

    SDL_assert(options[shortest].valid);

    // Remember I said the order should match the enum?
    switch (shortest)
    {
    case 0:
        result = Direction::Up;
        break;
    case 1:
        result = Direction::Down;
        break;
    case 2:
        result = Direction::Left;
        break;
    case 3:
        result = Direction::Right;
        break;
    }

    return result;
}