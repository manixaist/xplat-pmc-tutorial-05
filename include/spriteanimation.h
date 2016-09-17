#pragma once
#include "SDL.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    // Used to control animation behavior when then
    // end of the sequence is reached
    enum class AnimationType
    {
        Loop = 0,  // start over
        Once = 1,  // stop
    };

    // An animation consists of a sequence of frames and a frame delay (assuming we're updating every frame) between
    // updates to the current frame.  This helper class handles tracking all of that for the sprite
    class SpriteAnimation
    {
    public:
        SpriteAnimation(Uint16 cFrames, int* pAnimationSequence, AnimationType animationType, Uint16 animationSpeed) :
            _cFrames(cFrames),
            _frameIndex(0),
            _currentAnimationCounter(0),
            _maxAnimationCounter(animationSpeed),
            _type(animationType)
        {
            // Allocate space for the sequence and copy each frame over
            _pAnimation = new int[_cFrames];
            SDL_memcpy(_pAnimation, pAnimationSequence, sizeof(int)*cFrames);
        }

        ~SpriteAnimation()
        {
            // free our allocated sequence
            delete[] _pAnimation;
        }

        void Update()
        {
            // Assumes we don't foolishly set the delay to max Uint16 value
            _currentAnimationCounter++;
            if (_currentAnimationCounter >= _maxAnimationCounter)
            {
                AdvanceFrame();
                _currentAnimationCounter = 0;
            }
        }

        void Reset()
        {
            _currentAnimationCounter = 0;
            _frameIndex = 0;
        }

        int CurrentFrame() { return _pAnimation[_frameIndex]; }
        
        void AdvanceFrame()
        {
            // Just advance while we're 1 or more away from the end
            // we're 0 indexed so this is -2 from the total
            if (_frameIndex <= (_cFrames - 2))
            {
                _frameIndex++;
            }
            else if (_frameIndex >= (_cFrames - 1) && _type == AnimationType::Loop)
            {
                // Now if looping, reset the animation, otherwise do nothing
                Reset();
            }
        }

    private:
        Uint16 _cFrames;                    // Total frames in the sequence
        Uint16 _frameIndex;                 // Index into sequence currently displayed
        Uint16 _currentAnimationCounter;    // Counter between updates
        Uint16 _maxAnimationCounter;        // Max counter before updates and _currentAnimationCounter rolls over
        AnimationType _type;                // Loop or once
        int* _pAnimation;                   // The sequence of frames
    };
}
}