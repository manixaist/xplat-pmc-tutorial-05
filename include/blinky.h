#pragma once
#include "ghost.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    // "Blinky" type ghost.  This class is a thin wrapper in most places, just holding the
    // specific tile initialization code for example
    // This level also defines the specific movement behavior in the various ghost states,
    // e.g. what are its target tiles
    class Blinky : public Ghost
    {
    public:
        Blinky(TextureWrapper* pTextureWrapper);

        // "Interface" for my ghosts to implement
        bool Initialize();
        bool Reset(Maze *pMaze);
        Direction MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze);
    };
}
}