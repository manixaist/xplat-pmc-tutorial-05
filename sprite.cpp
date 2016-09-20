#include "include/sprite.h"
#include <algorithm>

using namespace XplatGameTutorial::PacManClone;

Sprite::Sprite(TextureWrapper *pTextureWrapper, Uint16 cxFrame, Uint16 cyFrame, Uint16 cFramesTotal, Uint16 cAnimationsTotal) :
    _x(0.0),
    _y(0.0),
    _dx(0.0),
    _dy(0.0),
    _cFramesTotal(cFramesTotal),
    _pFrames(nullptr),
    _cxFrame(cxFrame),
    _cyFrame(cyFrame),
    _cxFrameOffset(0),
    _cyFrameOffset(0),
    _currentAnimationIndex(0),
    _cAnimationsTotal(cAnimationsTotal),
    _staticFrameIndex(0),
    _fVisible(SDL_TRUE),
    _pTextureWrapper(pTextureWrapper),
    _ppSpriteAnimations(nullptr)
{
}

Sprite::~Sprite()
{
    // Delete all loaded animations
    for (int i = 0; i < _cAnimationsTotal; i++)
    {
        delete _ppSpriteAnimations[i];
        _ppSpriteAnimations[i] = nullptr;
    }
    // Delete the array holding those animations
    delete[] _ppSpriteAnimations;

    // Delete allocated frame rects
    delete[] _pFrames;
}

// Loads a single frame at the given coordinates on the texture to the specifed index
bool Sprite::LoadFrame(Uint16 frameIndex, Uint16 xTexture, Uint16 yTexture)
{
    // We've made several assumption in the implementation, so validate them
    SDL_assert((_pTextureWrapper != nullptr) && (!_pTextureWrapper->IsNull()));
    SDL_assert(_cxFrame > 0);
    SDL_assert(_cyFrame > 0);
    SDL_assert(_cFramesTotal > 0);

    bool fResult = true;

    // Index bounds check
    if (frameIndex >= _cFramesTotal)
    {
        printf("Sprite::LoadFrame() : frame index out of range\n");
        fResult = false;
    }

    // Texture bounds check
    if ((xTexture + _cxFrame > _pTextureWrapper->Width()) ||
        (yTexture + _cyFrame > _pTextureWrapper->Height()))
    {
        printf("Sprite::LoadFrame() : frame bounds out of range {x:%u y:%u w:%d h:%d}\n", 
            xTexture, yTexture, _pTextureWrapper->Width(), _pTextureWrapper->Height());
        fResult = false;
    }

    if (fResult)
    {
        // On first frame load, allocate the frames
        if (_pFrames == nullptr)
        {
            // If this fails we're OOM and in for crashes anyway....
            _pFrames = new SDL_Rect[_cFramesTotal]{ {0,0,0,0} };
        }

        _pFrames[frameIndex].x = xTexture;
        _pFrames[frameIndex].y = yTexture;
        _pFrames[frameIndex].w = _cxFrame; // Every frame in the sprite is the same size
        _pFrames[frameIndex].h = _cyFrame;
    }
    return fResult;
}

// Load a series of frames assumed to be in horizontal order starting at the given index/coord
// This takes advantage of how I know the sprite textures are laid out (which is not uncommon)
bool Sprite::LoadFrames(Uint16 frameIndexStart, Uint16 xTextureStart, Uint16 yTextureStart, Uint16 framesToLoad)
{
    bool fResult = true;
    Uint16 x = xTextureStart;
    Uint16 y = yTextureStart;

    for (Uint16 index = frameIndexStart; (index < (frameIndexStart + framesToLoad)) && fResult; index++)
    {
        fResult |= LoadFrame(index, x, y);
        x += _cxFrame;
    }
    return fResult;
}

//  Store the given animation sequence at the specified index.  This is mostly delegated to the SpriteAnimation helper class
void Sprite::LoadAnimationSequence(Uint16 index, AnimationType animationType, int* pSequence, Uint16 cFramesInSequence, Uint16 animationSpeed)
{
    // First time allocate the space for the animation helpers
    if (_ppSpriteAnimations == nullptr)
    {
        _ppSpriteAnimations = new SpriteAnimation*[_cAnimationsTotal];
        SDL_memset(_ppSpriteAnimations, 0, sizeof(SpriteAnimation*) * _cAnimationsTotal);
    }

    // Creates a new helper for the animation and stores it
    SpriteAnimation* pAnimation = new SpriteAnimation(cFramesInSequence, pSequence, animationType, animationSpeed);
    _ppSpriteAnimations[index] = pAnimation;
}

void Sprite::ResetAnimation()
{
    // Delegate to helper
    _ppSpriteAnimations[_currentAnimationIndex]->Reset();
}

void Sprite::SetAnimation(Uint16 index)
{
    // If this isn't already the current animation
    // Because if it is, you wanted ResetAnimation()
    if (_currentAnimationIndex != index)
    {
        // Store it and reset the sequence
        _currentAnimationIndex = index;
        _ppSpriteAnimations[_currentAnimationIndex]->Reset();
    }
}

// Store a new velocity
void Sprite::SetVelocity(double dx, double dy)
{
    _dx = dx;
    _dy = dy;
}

// Manually set a position, normal play position is Update()d but we also
// need the ability to place it directly
void Sprite::ResetPosition(double x, double y)
{
    _x = x;
    _y = y;
}

// Manually set frame index for non-animated sprites
void Sprite::SetFrame(Uint16 frameIndex)
{
    // We're assuming this sprite has no animations, so assert it
    SDL_assert(_ppSpriteAnimations == nullptr);
    _staticFrameIndex = frameIndex;
}

// Set the offset of the 2D image rect from the X,Y location 
//
// e.g.       X--------------  X = offset location
//            |             |
//            |             |
//            |       O     |  O = origin of sprite (x,y)
//            |             |
//            ---------------
//
void Sprite::SetFrameOffset(int xOffset, int yOffset)
{
    _cxFrameOffset = xOffset;
    _cyFrameOffset = yOffset;
}

// Turn on/off sprite
void Sprite::SetVisible(SDL_bool visible)
{
    _fVisible = visible;
}

// set new positio based on velocity and update the current animation
void Sprite::Update()
{
    _x += _dx;
    _y += _dy;

    // Advance animation counters and if needed the frame
    _ppSpriteAnimations[_currentAnimationIndex]->Update();
}

// Very similar to the tilemap, only in this case, we're index the frame
// to draw based on the current animation state (or static frame) instead
// on a static indexed map of tiles
void Sprite::Render(SDL_Renderer *pSDLRenderer)
{
    if (_fVisible == SDL_TRUE)
    {
        // Find the index to the current frame in the current animation and draw it to the renderer
        // at the correct x,y delta offset
        int frameIndex = (_ppSpriteAnimations == nullptr) ? _staticFrameIndex : _ppSpriteAnimations[_currentAnimationIndex]->CurrentFrame();
        SDL_Rect targetRect{ static_cast<int>(_x) + _cxFrameOffset, static_cast<int>(_y) + _cyFrameOffset, _cxFrame, _cyFrame };
        SDL_RenderCopy(
            pSDLRenderer,
            _pTextureWrapper->Ptr(),
            &_pFrames[frameIndex],
            &targetRect);
    }
}

Direction Sprite::CurrentDirection()
{
    Direction result = Direction::None;

    if (_dx > 0)
    {
        result = Direction::Right;
    }
    else if (_dx < 0)
    {
        result = Direction::Left;
    }
    else if (_dy > 0)
    {
        result = Direction::Down;
    }
    else if (_dy < 0)
    {
        result = Direction::Up;
    }
    return result;
}

bool Sprite::IsOutOfView(SDL_Rect &rect)
{
    bool result = false;
    if (X() > rect.x + rect.w + Width())
    {
        result = true;
    }
    else if (X() < rect.x - Width())
    {
        result = true;
    }
    return result;
}
