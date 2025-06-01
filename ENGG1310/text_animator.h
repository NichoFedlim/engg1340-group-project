#ifndef TEXT_ANIMATOR_H
#define TEXT_ANIMATOR_H

#include <ncursesw/ncurses.h>
#include <string>

class TextAnimator {
private:
    int maxWidth;
    std::string speakerName;
    int textDelay; // Milliseconds between words
    bool canSkip;

public:
    TextAnimator(int width, const std::string& name, int delay = 100, bool skip = true);
    void animateText(const std::string& text);
};

// Function to show game introduction
void showGameIntroduction();

// Show round information
void showRoundInfo(int round, int numSafeTiles, float flashTime, int hp);

// Show game over screen
void gameOver(bool won, int finalRound);

// Show result after each round
void showResult(bool safe, int playerHP);

#endif // TEXT_ANIMATOR_H