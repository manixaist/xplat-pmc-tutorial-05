#pragma once
#include "utils.h"
#include "spriteanimation.h"
#include <map>

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Sprite: Represents a moveable, animation capable, 2D "character" on the screen.  Sprites can be static, or they can
    // be animated, and they have a position and velocity.  Pac-Man and the Ghosts are very obvious examples of sprites, but
    // they can be used for other purposes, such as the "text" output and the bonus fruit in the future.
    // It would also be possible to make the larger pellets (or even the smaller ones) into animated sprites.
    class Sprite
    {
    public:
        // pTextureWrapper - pointer to loaded texture that holds our sprite frames
        // cxFrame - width of a frame in pixels
        // cyFrame - height of a frame in pixels
        // cFramesTotal - total frames to load
        // cAnimationsTotal - total number of animation sequences needed
        Sprite(TextureWrapper *pTextureWrapper, Uint16 cxFrame, Uint16 cyFrame, Uint16 cFramesTotal, Uint16 cAnimationsTotal);
        virtual ~Sprite();

        // All frames are the same size once created above (cxFrame * cyFrame)
        // index - frame index to assign the image to 
        // xTexture - x coordinate on loaded texture
        // yTexture - y coordinate on loaded texture
        bool LoadFrame(Uint16 index, Uint16 xTexture, Uint16 yTexture);
        // Load a series of frame assumed to be in horizontal order starting at the given index/coord
        bool LoadFrames(Uint16 indexStart, Uint16 xTextureStart, Uint16 yTextureStart, Uint16 cFramesToLoad);

        //  Saves a series of frames to cycle through in order at a given speed (frame delay per update)
        // index - animation index to assign the sequence to
        // animationType - Currently either loop or once
        // pSequence - pointer to list of frames
        // cFramesInSequence - total frames in the sequence passed in
        // animationSpeed - the delay between frame updates
        void LoadAnimationSequence(Uint16 index, AnimationType animationType, int* pSequence, Uint16 cFramesInSequence, Uint16 animationSpeed);
        // Start the current animation over
        void ResetAnimation();
        // Set a new (already loaded) animation sequence as the current
        void SetAnimation(Uint16 index);
        // Set a new velocity
        void SetVelocity(double dx, double dy);
        // Set a new position (normally handled via Update but on death, etc)
        void ResetPosition(double x, double y); 
        // This is only needed for sprites that have no animation, the frame will not update
        void SetFrame(Uint16 frameIndex);
        // Offset from the pixel (X,Y) location of the sprite for the frame (defaults to 0)
        void SetFrameOffset(int xOffset, int yOffset);
        // If the sprite isn't visible, it won't render
        void SetVisible(SDL_bool visible);
        // Applies current state to the object (velocity, animation, etc)
        void Update();
        // Draw it to the renderer
        void Render(SDL_Renderer *pSDLRenderer);
        // Some quick accessors
        double X() { return _x; }
        double Y() { return _y; }
        double DX() { return _dx; }
        double DY() { return _dy; }
        Uint16 Width() { return _cxFrame; }
        Uint16 Height() { return _cyFrame; }

        Uint16 CurrentAnimation() { return _currentAnimationIndex; }
        Direction CurrentDirection();
        bool IsOutOfView(SDL_Rect &rect);

    protected:
        double _x;                              // Position
        double _y;
        double _dx;                             // Velocity
        double _dy;
        Uint16 _cFramesTotal;                   // Total number of frames to allocate
        SDL_Rect *_pFrames;                     // Frame rects in the texture
        Uint16 _cxFrame;                        // Width of a frame
        Uint16 _cyFrame;                        // Height of a frame
        int _cxFrameOffset;                     // Offset of left side of frame from position (can be negative)
        int _cyFrameOffset;                     // Offset of Top side of frame from position
        Uint16 _currentAnimationIndex;          // Index to the current animation sequence
        Uint16 _cAnimationsTotal;               // Total number of animation sequences
        Uint16 _staticFrameIndex;               // Index in non-animated sprite to frame to draw
        SDL_bool _fVisible;                     // Visibility flag
        TextureWrapper *_pTextureWrapper;       // Not owned by the sprite class
        SpriteAnimation** _ppSpriteAnimations;  // Is owned and holds the list of animation sequences
    };
}
}