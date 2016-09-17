#pragma once
#include "SDL_image.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Takes a texture divided evenly into tiles as well as a map size and a list of indices to the tiles
    // to fill out the map.  When rendered, the map will center itself in the total window and iterate over
    // the map, drawing the indexed tile
    class TiledMap
    {
    public:
        TiledMap(const Uint16 rows, const Uint16 cols, Uint16 cxScreen, Uint16 cyScreen) :
            _cxScreen(cxScreen),
            _cyScreen(cyScreen),
            _cxWidth(0),
            _cyHeight(0),
            _cxOffset(0),
            _cyOffset(0),
            _pTileRects(nullptr),
            _pMapIndicies(nullptr),
            _cCols(cols),
            _cRows(rows),
            _tileSize(0),
            _pTileTexture(nullptr),
            _cTilesOnTexture(0)
        {
            SDL_memset(&_textureRect, 0, sizeof(SDL_Rect));
        }

        virtual ~TiledMap()
        {
            // Free our allocated memory
            delete[] _pMapIndicies;
            delete[] _pTileRects;
        }

        // Initialize our map with the texture and map data
        bool Initialize(SDL_Rect textureRect, SDL_Rect tileRect, SDL_Texture *pTexture, Uint16 *pMapIndices, Uint16 countOfIndicies);
        
        // Draw to the renderer at the current offset, etc
        virtual void Render(SDL_Renderer *pSDLRenderer);
        
        // Given an [row][col] location, return the (X,Y) coordinates on the screen
        SDL_Point GetTileCoordinates(Uint16 row, Uint16 col);
        // Given a (X,Y) location, return the [row][col] if it exists
        bool GetTileRowCol(SDL_Point &point, Uint16 &row, Uint16 &col);
        // Return the outer bounds of the map
        SDL_Rect GetMapBounds();
        
    protected:
        Uint16 GetTileIndexAt(Uint16 row, Uint16 col) { return _pMapIndicies[(row * _cCols) + col]; }
        void SetTileIndexAt(Uint16 row, Uint16 col, Uint16 index) { _pMapIndicies[(row * _cCols) + col] = index; }
        
        Uint16 _cxScreen;           // Total screen (window) width in pixels
        Uint16 _cyScreen;           // Total screen height
        Uint16 _cxWidth;            // Total width of map
        Uint16 _cyHeight;           // Total height of map
        Uint16 _cxOffset;           // Offset from upper left corner of window (screen)
        Uint16 _cyOffset;           // ...
        SDL_Rect* _pTileRects;      // Will hold the list of tile source rects from the texture loaded
        Uint16 *_pMapIndicies;      // Will hold the indices to the map
        Uint16 _cCols;              // Cols in the map
        Uint16 _cRows;              // Rows in the map
        Uint16 _tileSize;           // Cached size of the tile (w == h in our implementation e.g. square tiles only)
        SDL_Rect _textureRect;      // Size of the texture
        SDL_Texture *_pTileTexture; // Texture that holds the tiles (must be evenly divisible by tile size)
        Uint16 _cTilesOnTexture;    // Total number of tiles on the texture
    };
}
}