#pragma once
#include "constants.h"
#include "tiledmap.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    class Sprite;

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

        SDL_bool IsTileIntersection(Uint16 row, Uint16 col)
        {
            Direction directions[] = { Direction::Up, Direction::Down, Direction::Left, Direction::Right };

            Uint16 exitsFound = 0;
            // We can stop if we find at least 3 (we will always have 1 - the direction we came)
            for (size_t index = 0; index < SDL_arraysize(directions) && exitsFound < 3; index++)
            {
                Uint16 nextRow = 0;
                Uint16 nextCol = 0;
                GetNextCell(row, col, nextRow, nextCol, directions[index]);
                if (!IsTileSolid(nextRow, nextCol))
                {
                    exitsFound++;
                }
            }

            // Always should be at least 1 found
            SDL_assert(exitsFound > 0);
            return (exitsFound >= 3) ? SDL_TRUE : SDL_FALSE;
        }

        void GetNextCell(Uint16 row, Uint16 col, Uint16 &nextRow, Uint16 &nextCol, Direction direction)
        {
            switch (direction)
            {
            case Direction::Up:
                nextRow = row - 1;
                nextCol = col;
                break;
            case Direction::Down:
                nextRow = row + 1;
                nextCol = col;
                break;
            case Direction::Left:
                nextCol = col - 1;
                nextRow = row;
                break;
            case Direction::Right:
                nextCol = col + 1;
                nextRow = row;
                break;
            }
            // No promises on whether this is solid, etc
        }

        void Render(SDL_Renderer *pSDLRenderer)
        {
            TiledMap::Render(pSDLRenderer);
        }

        SDL_bool IsSpritePastCenter(Uint16 row, Uint16 col, Sprite* pSprite)
        {
            SDL_Point spritePoint = { static_cast<int>(pSprite->X()), static_cast<int>(pSprite->Y()) };
            SDL_bool result = SDL_FALSE;

            // This returns the center pixel which is useful here
            SDL_Point centerPoint = GetTileCoordinates(row, col);

            // Now based on the direction, are we ahead of or behind that center pixel?
            if (pSprite->DX() < 0)
            {
                result = (pSprite->X() <= centerPoint.x) ? SDL_TRUE : SDL_FALSE;
            }
            else if (pSprite->DX() > 0)
            {
                result = (pSprite->X() > centerPoint.x) ? SDL_TRUE : SDL_FALSE;
            }
            else if (pSprite->DY() < 0)
            {
                result = (pSprite->Y() <= centerPoint.y) ? SDL_TRUE : SDL_FALSE;
            }
            else if (pSprite->DY() > 0)
            {
                result = (pSprite->Y() > centerPoint.y) ? SDL_TRUE : SDL_FALSE;
            }
            return result;
        }
    };
}
}