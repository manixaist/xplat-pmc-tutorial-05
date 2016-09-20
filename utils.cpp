#include "include/constants.h"
#include "include/utils.h"
#include "SDL_image.h"
#include <stdio.h>

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Returns the opposite direction passed in.
    Direction Opposite(Direction dir)
    {
        SDL_assert(dir != Direction::None);
        Direction opposites[] = { Direction::Down, Direction::Up, Direction::Right, Direction::Left };
        return opposites[static_cast<int>(dir)];
    }

    // Shifts a cell in a direction, does not care about bounds or "solidness"
    void TranslateCell(Uint16 &row, Uint16 &col, Direction direction)
    {
        switch (direction)
        {
        case Direction::Up:
            row--;
            break;
        case Direction::Down:
            row++;
            break;
        case Direction::Left:
            col--;
            break;
        case Direction::Right:
            col++;
            break;
        }
    }

    // Fairly basic SDL code of which there are many examples.  This loads an image from disk to a surface, then
    // if a colorKey is provided, sets the transparency, then we create a texture from the surface that is compatible
    // and finally we're done
    SDL_Texture* LoadTexture(const char *szFileName, SDL_Renderer *pSDLRenderer, SDL_Color *pSdlTransparencyColorKey)
    {
        SDL_Texture* pTextureOut = nullptr;
        SDL_Surface* pSDLSurface = IMG_Load(szFileName);
        if (pSDLSurface == nullptr)
        {
            printf("IMG_Load() failed, error = %s\n", IMG_GetError());
        }
        else
        {
            if (pSdlTransparencyColorKey != nullptr)
            {
                // Set the color key
                SDL_SetColorKey(pSDLSurface, SDL_TRUE, SDL_MapRGB(pSDLSurface->format, 
                    pSdlTransparencyColorKey->r, pSdlTransparencyColorKey->g, pSdlTransparencyColorKey->b));
            }
            pTextureOut = SDL_CreateTextureFromSurface(pSDLRenderer, pSDLSurface);
            SDL_FreeSurface(pSDLSurface);
        }
        return pTextureOut;
    }

    // Setup SDL and our window
    bool InitializeSDL(SDL_Window **ppSDLWindow, SDL_Renderer **ppSDLRenderer)
    {
        bool fResult = true;
        *ppSDLWindow = nullptr;
        *ppSDLRenderer = nullptr;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) // SDL_INIT_EVERYTHING works too, but we only need video...init what you need
        {
            printf("SDL_Init() failed, error = %s\n", SDL_GetError());
            fResult = false;
        }
        else
        {
            // Creates the Window for the GUI
            *ppSDLWindow = SDL_CreateWindow(Constants::WindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                Constants::ScreenWidth, Constants::ScreenHeight, SDL_WINDOW_SHOWN);
            if (*ppSDLWindow == nullptr)
            {
                printf("SDL_CreateWindow() failed, error = %s\n", SDL_GetError());
                fResult = false;
            }
            else
            {
                // We now need a renderer to make use of textures, so create one based on the window and we'll use this to update what
                // the user sees rather than drawing to the SDL_Surface like last time
                *ppSDLRenderer = SDL_CreateRenderer(*ppSDLWindow, -1, SDL_RENDERER_ACCELERATED);
                if (*ppSDLRenderer == nullptr)
                {
                    printf("SDL_CreateRender() failed, error = %s\n", SDL_GetError());
                    fResult = false;
                }
                else
                {
                    // Basically sets the color that will fill the screen when cleared
                    if (SDL_SetRenderDrawColor(*ppSDLRenderer, Constants::RenderDrawColor.r, Constants::RenderDrawColor.g,
                        Constants::RenderDrawColor.b, Constants::RenderDrawColor.a) < 0)
                    {
                        printf("SDL_SetRenderDrawColor() failed, error = %s\n", SDL_GetError());
                        fResult = false;
                    }
                    else
                    {
                        // This bit will allow us to load PNG files, which I am storing all my images assets as
                        const int cFlagsNeeded = IMG_INIT_PNG | IMG_INIT_JPG;
                        int iFlagsInitted = IMG_Init(cFlagsNeeded);
                        if ((iFlagsInitted & (cFlagsNeeded)) != (cFlagsNeeded))
                        {
                            printf("IMG_Init() failed, error = %s\n", IMG_GetError());
                            fResult = false;
                        }
                    }
                }
            }
        }
        return fResult;
    }

    // Instantiate our helper - load the texture, query basic info and cache it
    TextureWrapper::TextureWrapper(const char *szFileName, size_t cchFileName, SDL_Renderer *pSDLRenderer, SDL_Color *pSdlTransparencyColorKey) : TextureWrapper()
    {
        size_t bytesToAllocate = cchFileName + 1;
        _pszFilename = new char[bytesToAllocate];
        SDL_memset(_pszFilename, 0, bytesToAllocate);
        SDL_memcpy(_pszFilename, szFileName, bytesToAllocate);

        printf("Attempting to load texture %s...\n", szFileName);
        _pTexture = LoadTexture(szFileName, pSDLRenderer, pSdlTransparencyColorKey);
        if (_pTexture != nullptr)
        {
            if (SDL_QueryTexture(_pTexture, nullptr, nullptr, &_cxTexture, &_cyTexture) != 0)
            {
                printf("SDL_QueryTexture() failed, error = %s\n", SDL_GetError());
            }
            else
            {
                printf("loaded %s { w:%d, h:%d }\n", szFileName, _cxTexture, _cyTexture);
            }
        }
    }

    TextureWrapper::~TextureWrapper()
    {
        if (_pTexture != nullptr)
        {
            printf("Destroying Texture %s\n", _pszFilename);
            SDL_DestroyTexture(_pTexture);
            _pTexture = nullptr;
        }
        delete[] _pszFilename;
    }
}
}