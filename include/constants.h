#pragma once
#include "SDL.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    class Constants
    {
    public:
        static const Uint16 ScreenWidth = 800;
        static const Uint16 ScreenHeight = 600;
        static const Uint32 FramesPerSecond = 60;
        static const Uint32 TicksPerFrame;
        static const SDL_Color SDLColorGrey;
        static const SDL_Color SDLColorMagenta;
        static const SDL_Color RenderDrawColor;
        static const char * const WindowTitle;
        static const Uint16 MapRows = 36;
        static const Uint16 MapCols = 28;
        static const Uint16 TileTextureWidth = 192;
        static const Uint16 TileTextureHeight = 192;
        static const Uint16 TileWidth = 16;
        static const Uint16 TileHeight = 16;
        static const Uint16 PlayerSpriteWidth = 32;
        static const Uint16 PlayerSpriteHeight = 32;
        static const Uint16 PlayerStartRow = 26;
        static const Uint16 PlayerStartCol = 13;
        static const Uint16 TotalPellets = 244;
        static const Uint32 LevelLoadDelay = 3000;
        static const Uint32 LevelCompleteDelay = 6000;
        static const Uint16 WarpRow = 17;

        // Indices to tiles that make up the map - for your own sanity use a level editor (several free ones exist) or better
        // yet develop your own tool early in the design process
        //  We just have this one level we'll reuse, so just and paste as long as you don't change the order of the tiles.png
        static Uint16 MapIndicies[MapRows * MapCols];
        static Uint16 CollisionMap[MapRows * MapCols];

        static const Uint16 PlayerAnimationSpeed = 5;

        // Animations
        static const Uint16 AnimationIndexUp = 0;
        static const Uint16 AnimationIndexDown = 1;
        static const Uint16 AnimationIndexLeft = 2;
        static const Uint16 AnimationIndexRight = 3;
        static const Uint16 AnimationIndexDeath = 4;
        
        static const Uint16 PlayerTotalFrameCount = 20;
        static const Uint16 PlayerTotalAnimationCount = 5;
        static const Uint16 PlayerAnimationFrameCount = 4;
        static const Uint16 PlayerAnimationDeathFrameCount = 11;
        static int PlayerAnimation_UP[PlayerAnimationFrameCount];
        static int PlayerAnimation_DOWN[PlayerAnimationFrameCount];
        static int PlayerAnimation_LEFT[PlayerAnimationFrameCount];
        static int PlayerAnimation_RIGHT[PlayerAnimationFrameCount];
        static int PlayerAnimation_DEATH[PlayerAnimationDeathFrameCount];

        // Strings
        static const char * const TilesImage;
        static const char * const SpritesImage;

    private:
        static const Uint32 c_msPerSecond = 1000;
        static const Uint32 c_msPerFrame = (c_msPerSecond / FramesPerSecond);
    };
}
}
