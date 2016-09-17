#pragma once
#include "SDL.h"
#include <stdio.h>

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Template helper to delete a class and set its pointer to nullptr
    template <class T> void SafeDelete(T* p)
    {
        if (p != nullptr)
        {
            delete reinterpret_cast<T*>(p);
            p = nullptr;
        }
    }

    // Load a texture from disk with optional transparency
    SDL_Texture* LoadTexture(const char *szFileName, SDL_Renderer *pSDLRenderer, SDL_Color *pSdlTransparencyColorKey);
    
    // Sets up our SDL environment and Window
    bool InitializeSDL(SDL_Window **ppSDLWindow, SDL_Renderer **ppSDLRenderer);

    // Small wrapper for the SDL_Texture object.  It will cache some basic info (like size)
    // and free it upon destruction
    class TextureWrapper
    {
    public:
        TextureWrapper() :
            _pTexture(nullptr),
            _cxTexture(0),
            _cyTexture(0),
            _pszFilename(nullptr)
        {
        }

        TextureWrapper(const char *szFileName, size_t cchFileName, SDL_Renderer *pSDLRenderer, SDL_Color *pSdlTransparencyColorKey);
        
        ~TextureWrapper();

        // Accessors
        bool IsNull() { return _pTexture == nullptr; }
        int Width() { return _cxTexture;  }
        int Height() { return _cyTexture; }
        SDL_Texture* Ptr() { return _pTexture; }
  
    private:
        SDL_Texture *_pTexture;
        int _cxTexture;
        int _cyTexture;
        char *_pszFilename;
    };
}
}
