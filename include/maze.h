#pragma once
#include "constants.h"
#include "tiledmap.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Derived class that adds information to the TiledMap specific to the PacManClone
    // maze, such as collision detection with walls and pellets.
    class Maze : public TiledMap
    {
    public:
        Maze(const Uint16 rows, const Uint16 cols, Uint16 cxScreen, Uint16 cyScreen) :
            XplatGameTutorial::PacManClone::TiledMap(rows, cols, cxScreen, cyScreen)
        {
        }

        virtual ~Maze()
        {
        }

        SDL_bool IsTilePellet(Uint16 row, Uint16 col)
        {
            if ((GetTileIndexAt(row, col) == 16) || (GetTileIndexAt(row, col) == 13))
            {
                return SDL_TRUE;
            }
            return SDL_FALSE;
        }

        void EatPellet(Uint16 row, Uint16 col)
        {
            SDL_assert((GetTileIndexAt(row, col) == 16) || (GetTileIndexAt(row, col) == 13));
            SetTileIndexAt(row, col, 49);
        }

        SDL_bool IsTileSolid(Uint16 row, Uint16 col)
        {
            return (Constants::CollisionMap[
                row * Constants::MapCols + col] == 1) ? SDL_TRUE : SDL_FALSE;
        }

        void Render(SDL_Renderer *pSDLRenderer)
        {
            TiledMap::Render(pSDLRenderer);
        }
    };
}
}