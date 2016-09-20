#pragma once
#include "sprite.h"
#include "maze.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    class Player : public Sprite
    {
    public:
        Player(TextureWrapper *pTextureWrapper);
        virtual ~Player();

        bool Initialize();
        bool Reset(Maze *pMaze);
        void Update(Maze* pMaze, Direction inputDirection);

    private:
        // Internal state
        enum class Mode
        {
            Normal = 0,
            WarpingOut,
            WarpingIn
        };

        void ProcessPlayerInput(Maze* pMaze, Direction direction);
        void DoBoundsCheck(Maze* pMaze);

        bool IsWarpingOut(Maze* pMaze)
        {
            SDL_Point spritePoint = { static_cast<int>(X()), static_cast<int>(Y()) };
            Uint16 row, col;
            pMaze->GetTileRowCol(spritePoint, row, col);
            return ((row == Constants::WarpRow) && 
                ((col == Constants::WarpColPlayerLeft) || (col == Constants::WarpColPlayerRight)));
        }

        Mode _mode;
    };
}
}