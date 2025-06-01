#ifndef TEXT_ANIMATOR_H
#define TEXT_ANIMATOR_H

#include <ncursesw/ncurses.h>
#include <string>
#include <vector>

class TextAnimator {
private:
    int maxWidth;
    std::string speakerName;
    int textDelay;
    bool canSkip;
    std::vector<std::string> dialogues;

public:
    TextAnimator(int width, const std::string& name, int delay, bool skip);
    void loadDialogues(const std::string& filename);
    void animateText(const std::string& text);
    void showDialogue(int index);
    std::string getDialogue(int index);
    size_t getDialogueCount();
};

// Helper functions
void showGameIntroduction();
void showRoundInfo(int round);
void showRoundComplete(int round, int score);
void gameOver(bool won, int finalRound, int score);

#endif // TEXT_ANIMATOR_H