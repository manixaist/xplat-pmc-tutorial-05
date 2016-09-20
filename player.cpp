#include "include/player.h"

using namespace XplatGameTutorial::PacManClone;

Player::Player(TextureWrapper *pTextureWrapper) :
    Sprite(pTextureWrapper, Constants::PlayerSpriteWidth, Constants::PlayerSpriteHeight,
        Constants::PlayerTotalFrameCount, Constants::PlayerTotalAnimationCount),
    _mode(Mode::Normal)   
{
}

Player::~Player()
{
}

bool Player::Initialize()
{
    LoadFrames(0, 0, 0, 10);
    LoadFrames(10, 0, Constants::PlayerSpriteHeight, 10);
    LoadAnimationSequence(Constants::AnimationIndexLeft, AnimationType::Loop, Constants::PlayerAnimation_LEFT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexRight, AnimationType::Loop, Constants::PlayerAnimation_RIGHT, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexUp, AnimationType::Loop, Constants::PlayerAnimation_UP, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDown, AnimationType::Loop, Constants::PlayerAnimation_DOWN, Constants::PlayerAnimationFrameCount, Constants::PlayerAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDeath, AnimationType::Once, Constants::PlayerAnimation_DEATH, Constants::PlayerAnimationDeathFrameCount, Constants::PlayerAnimationSpeed);
    SetFrameOffset(1 - (Constants::PlayerSpriteWidth / 2), 1 - (Constants::PlayerSpriteHeight / 2));
    return true;
}

bool Player::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexLeft);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::PlayerStartRow, Constants::PlayerStartCol);
    playerStartCoord.x += Constants::TileWidth / 2;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(Constants::PlayerMaxSpeed * -.75, 0);  // Eventually speeds will be based on level, dots eaten, etc
    _mode = Mode::Normal;
    return true;
}

void Player::Update(Maze* pMaze, Direction inputDirection)
{
    Sprite::Update();

    switch (_mode)
    {
    case Mode::Normal:
    {

        if (IsWarpingOut(pMaze))
        {
            // If we've reached a warp tile, stop taking input and let the warp
            // subroutine handle movement
            _mode = Mode::WarpingOut;
        }
        else
        {
            // Otherwise check our input and move if it's valid
            ProcessPlayerInput(pMaze, inputDirection);
            DoBoundsCheck(pMaze);
        }
        break;
    }
    case Mode::WarpingOut:
    {

        // Just keep moving until out of view...
        SDL_Rect mapRect = pMaze->GetMapBounds();
        if (IsOutOfView(mapRect))
        {
            // Place at other end of screen out of view...
            if (DX() > 0)
            {
                ResetPosition(mapRect.x - Width(), Y());
                _mode = Mode::WarpingIn;
            }
            else if (DX() < 0)
            {
                ResetPosition(mapRect.x + mapRect.w + Width(), Y());
                _mode = Mode::WarpingIn;
            }
        }
        break;
    }
    case Mode::WarpingIn:
    {
            // Just keep moving until back in view...
        SDL_Point playerPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
        Uint16 row, col;
        pMaze->GetTileRowCol(playerPoint, row, col);
        if ((row == Constants::WarpRow) && ((col == Constants::WarpColPlayerLeft + 1) || (col == Constants::WarpColPlayerRight - 1)))
        {
            // Start accepting player input again..
            _mode = Mode::Normal;
        }
        break;
    }
    }
}

void Player::ProcessPlayerInput(Maze* pMaze, Direction direction)
{
    if (direction == Direction::None)
    {
        return;
    }

    SDL_Point playerPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
    Uint16 playerRow = 0;
    Uint16 playerCol = 0;
    pMaze->GetTileRowCol(playerPoint, playerRow, playerCol);

    // Given a player's current state (location, direction, animation) check if the player can move in a given direction, and if
    // so position the player on the new track at the new velocity
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
    if ((CanMove(direction, playerRow, playerCol) == SDL_TRUE) &&
        (CurrentAnimation() != static_cast<Uint16>(direction)))
    {
        // Set a new animation and position the player with a new velocity
        SDL_Point tilePoint = pMaze->GetTileCoordinates(playerRow, playerCol);
        ResetPosition(tilePoint.x, tilePoint.y);

        // Set Direction
        switch (direction)
        {
        case Direction::Up:
            SetVelocity(0, Constants::PlayerMaxSpeed * -.75);
            SetAnimation(Constants::AnimationIndexUp);
            break;
        case Direction::Down:
            SetVelocity(0, Constants::PlayerMaxSpeed * .75);
            SetAnimation(Constants::AnimationIndexDown);
            break;
        case Direction::Left:
            SetVelocity(Constants::PlayerMaxSpeed * -.75, 0);
            SetAnimation(Constants::AnimationIndexLeft);
            break;
        case Direction::Right:
            SetVelocity(Constants::PlayerMaxSpeed * .75, 0);
            SetAnimation(Constants::AnimationIndexRight);
            break;
        case Direction::None:
            break;
        }
    }
}

// Don't allow the player to wander through a solid wall
void Player::DoBoundsCheck(Maze* pMaze)
{
    SDL_Point playerPoint = { static_cast<int>(X()), static_cast<int>(Y()) };

    // Need to check bounds in direction moving (account for width of half the sprite)
    // This is because the sprite is double the size of the tiles and placed along the centerline
    // in the direction of movement.  So 1/2 of its size in a given direction is the "edge" of the
    // sprite on the screen (minus a pixel or 2 of transparency)
    if (DX() != 0) // If we're not moving in this axis, then don't bother
    {
        if (DX() < 0)
        {
            playerPoint.x -= (Width() / 2) - Constants::TileWidth / 2;
        }
        else
        {
            playerPoint.x += (Width() / 2) - Constants::TileWidth / 2;
        }
    }
    else  // We cann't be moving in both directions at once
    {
        if (DY() < 0)  // Same logic for y axis if moving
        {
            playerPoint.y -= (Height() / 2) - Constants::TileHeight / 2;
        }
        else
        {
            playerPoint.y += (Height() / 2) - Constants::TileHeight / 2;
        }
    }

    // Now get the row, col we're in
    Uint16 row = 0;
    Uint16 col = 0;
    pMaze->GetTileRowCol(playerPoint, row, col);

    if (pMaze->IsTileSolid(row, col))
    {
        // If we wandered into a bad cell, stop
        SetVelocity(0, 0);
    }
}