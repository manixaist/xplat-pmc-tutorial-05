#include "include/tiledmap.h"

using namespace XplatGameTutorial::PacManClone;

// The main goals here are to 
// 1) Divide up the texture into src rects
// 2) Copy the index data
// 3) Cache some calculated values we'll reuse rendering
bool TiledMap::Initialize(
    SDL_Rect textureRect,           // Size of the texture
    SDL_Rect tileRect,              // size of the tile - the texture should be a multiple of this size...
    SDL_Texture *pTexture,          // texture holding the tiles
    Uint16 *pMapIndices,            // array of indicies to the tiles, should match in size to map
    Uint16 countOfIndicies)         // again should match, but here to be explicit in the code
{
    // Validate some assumptions
    SDL_assert((textureRect.w % tileRect.w) == 0);
    SDL_assert((textureRect.h % tileRect.h) == 0);
    SDL_assert(countOfIndicies == (_cRows * _cCols));
    SDL_assert(pMapIndices != nullptr);

    // Copy the map indicies data
    _pMapIndicies = new Uint16[countOfIndicies] { };
    SDL_memcpy(_pMapIndicies, pMapIndices, countOfIndicies * sizeof(Uint16));

    // Copy the texture data
    _pTileTexture = pTexture;
    SDL_memcpy(&_textureRect, &textureRect, sizeof(SDL_Rect));

    // Calculate the total available tiles on the texture and allocate space for the source rects
    // this is all just dividing the coordinate space into even squares
    _tileSize = static_cast<Uint16>(tileRect.w);
    Uint16 textureTilesPerWidth  = static_cast<Uint16>((_textureRect.w / _tileSize));    // The texture itself does not need to be square
    Uint16 textureTilesPerHeight = static_cast<Uint16>((_textureRect.h / _tileSize));
    _cTilesOnTexture = static_cast<Uint16>(((_textureRect.w / _tileSize) * textureTilesPerHeight));
    _pTileRects = new SDL_Rect[_cTilesOnTexture] {};
    
    // Center the map, so calculate the offsets
    _cxWidth = (_cCols * _tileSize);
    _cyHeight = (_cRows * _tileSize);
    _cxOffset = (_cxScreen - _cxWidth) / 2;
    _cyOffset = (_cyScreen - _cyHeight) / 2;

    if (_pTileRects != nullptr)
    {
        // Loop through the tiles and set the source rects
        for (int r = 0; r < textureTilesPerHeight; r++)
        {
            for (int c = 0; c < textureTilesPerWidth; c++)
            {
                _pTileRects[((r * textureTilesPerHeight) + c)].h = _tileSize;
                _pTileRects[((r * textureTilesPerHeight) + c)].w = _tileSize;
                _pTileRects[((r * textureTilesPerHeight) + c)].x = _tileSize * c;
                _pTileRects[((r * textureTilesPerHeight) + c)].y = _tileSize * r;
            }
        }
    }
    return true;
}

// Loop through the map of indicies and render each tile in order.  Center the map on the screen
void TiledMap::Render(SDL_Renderer *pSDLRenderer)
{
    SDL_assert(_cRows * _pTileRects[0].w <= _cxScreen); // Every tile is the same size in this implementation
    SDL_assert(_cCols * _pTileRects[0].h <= _cyScreen);

    SDL_Rect targetRect = {0, 0, _tileSize, _tileSize }; // The size won't change, so we'll update the x, y
    for (int r = 0; r < _cRows; r++)
    {
        for (int c = 0; c < _cCols; c++) 
        {
            targetRect.x = (c * _tileSize) + _cxOffset;
            targetRect.y = (r * _tileSize) + _cyOffset;
            int currentTileIndex = _pMapIndicies[r * _cCols + c];

            SDL_RenderCopy(
                pSDLRenderer,                   // Our renderer - everything goes here that draws
                _pTileTexture,                  // texture that holds the source tiles
                &_pTileRects[currentTileIndex], // rect in our map indicies list that tells us which tile to draw
                &targetRect);                   // dest rect on the screen for the tile indexed above
        }
    }
}

// returns the "center" pixel of the tile in 2D space - this helps with the sprite logic
SDL_Point TiledMap::GetTileCoordinates(Uint16 row, Uint16 col)
{
    int x = (col * _tileSize) + _cxOffset + (_tileSize / 2);
    int y = (row * _tileSize) + _cyOffset + (_tileSize / 2);
    return { x, y };
}

bool TiledMap::GetTileRowCol(SDL_Point &point, Uint16 &row, Uint16 &col)
{
    // First check if this point is even on the map
    SDL_Rect pointRect = { point.x, point.y, 1, 1 };
    SDL_Rect mapRect =  GetMapBounds();
    bool fResult = (SDL_HasIntersection(&mapRect, &pointRect) == SDL_TRUE);

    if (fResult)
    {
        // If so convert it
        row = static_cast<Uint16>((point.y - _cyOffset) / _tileSize);
        col = static_cast<Uint16>((point.x - _cxOffset) / _tileSize);
    }
    return fResult;
}

// Return the bounding rect of the entire map
SDL_Rect TiledMap::GetMapBounds()
{
    return{ (_cxScreen - (_cCols * _tileSize)) / 2, (_cyScreen - _cyHeight) / 2, (_cCols * _tileSize), (_cRows * _tileSize) };
}
