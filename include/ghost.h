#pragma once
#include "sprite.h"
#include "maze.h"
#include "player.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Our Ghost class will encapsulate the basic behavior common to every ghost
    // (e.g. movement when not at an intersection) but will defer branching logic
    // (e.g. the case above - intersections) and texture specific loading and values
    // like base speed to a derived class.
    // I.e. this class does not exist by itself, somewhere there is a Blinky : Ghost,
    // Clyde : Ghost, etc
    class Ghost : public Sprite
    {
    public:
        Ghost(TextureWrapper *pTextureWrapper, Uint16 cxFrame, Uint16 cyFrame, Uint16 cFramesTotal, Uint16 cAnimationsTotal);
        
        virtual ~Ghost()
        {
            SafeDelete<Decision>(_pCurrentDecision);
            SafeDelete<Decision>(_pNextDecision);
        }

        // "Interface" for Ghosts to implement
        virtual bool Initialize() = 0;
        virtual bool Reset(Maze *pMaze) = 0;
        virtual Direction MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze) = 0;

        // General movement that is common to all ghosts
        void Update(Player* pPlayer, Maze* pMaze);

    protected:
        struct Decision
        {
            Decision(Uint16 r, Uint16 c, Direction newDirection) :
                row(r),
                col(c),
                direction(newDirection)
            {
            }

            Direction GetDirection() { return direction; }
            Uint16 Row() { return row; }
            Uint16 Col() { return col; }

        private:
            Uint16 row;
            Uint16 col;
            Direction direction;
        };

        // Internal state
        enum class Mode
        {
            Chase = 0,
            Scatter,
            WarpingOut,
            WarpingIn,
            ExitingPen,
        };

        Direction GetNextDirection(Uint16 r, Uint16 c, Maze *pMaze);
        Decision* GetNextDecision(Player *pPlayer, Maze* pMaze);
        bool IsGhostWarpingOut(Maze* pMaze);
        bool IsGhostPenned()
        {
            return (_currentCol > 10 && _currentCol < 17 && _currentRow > 15 && _currentRow < 18);
        }

        void Stop() { SetVelocity(0.0, 0.0); }
        bool IsStopped() { return (DX() == 0.0 && DY() == 0.0); }
        void OnExitingPen(Player* pPlayer, Maze* pMaze);
        void OnWarpingOut(Player* pPlayer, Maze* pMaze);
        void OnWarpingIn(Player* pPlayer, Maze* pMaze);
        void OnChasing(Player* pPlayer, Maze* pMaze);

        void UpdateAnimation(Direction direction);
        
        StateTimer _penTimer;           // Timer used to exit initial pen area
        Uint16 _currentRow;             // Current cell location
        Uint16 _currentCol;
        Mode _mode;                     // Chase, scatter, etc
        Decision *_pNextDecision;       // Decision for the coming cell
        Decision *_pCurrentDecision;    // Decision for our current cell
    };
}
}