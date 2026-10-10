#ifndef GAME_DEMO_H
#define GAME_DEMO_H

#include "../engine/Game.h"
#include "../engine/Room.h"
#include "../engine/Item.h"
#include "../engine/Door.h"
#include "../engine/MiniGame.h"
#include "../engine/NPC.h"

// Forward declarations for mini-games
std::string playFlappyBird(Game& game);
std::string playBurgerFlipper(Game& game);
std::string playReactionTest(Game& game);
std::string playMathQuiz(Game& game);
std::string playTypingTest(Game& game);
std::string playPotionBrewing(Game& game);
std::string playSwordSharpening(Game& game);
std::string playLockpicking(Game& game);

void clearScreenMain(Game& game);
void resetCursorMain(Game& game);

#endif // GAME_DEMO_H
