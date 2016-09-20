#pragma once
#include "SDL.h"
#include <stdio.h>

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Oneshot timer for state transistions
    class StateTimer
    {
    public:
        StateTimer() : _startTicks(0), _targetTicks(0), _fStarted(false)
        {
        }

        void Start(Uint32 waitTicks)
        {
            SDL_assert(!_fStarted);
            SDL_assert(_startTicks == 0);
            _startTicks = SDL_GetTicks();
            _targetTicks = waitTicks;
            _fStarted = true;
        }

        void Reset() { _fStarted = false; _startTicks = 0; }
        bool IsStarted() { return _fStarted; }
        bool IsDone() { return IsStarted() && (SDL_GetTicks() - _startTicks > _targetTicks); }
    private:
        Uint32 _startTicks;
        Uint32 _targetTicks;
        bool _fStarted;
    };

    // Simple enum to denote the 4 possible directions
    // The sprites can move, plus none
    enum class Direction
    {
        Up = 0,
        Down,
        Left,
        Right,
        None
    };

    Direction Opposite(Direction dir);

    void TranslateCell(Uint16 &row, Uint16 &col, Direction direction);

    // Template helper to delete a class and set its pointer to nullptr
    template <class T> void SafeDelete(T*& p)
    {
        delete p;   // null check is not required for delete
        p = nullptr;
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
