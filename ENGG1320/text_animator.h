#ifndef TEXT_ANIMATOR_H
#define TEXT_ANIMATOR_H

#include <string>
#include <vector>
#include <ncurses.h>

class TextAnimator {
public:
    TextAnimator(int width, const std::string& name, int delay, bool skip);
    void loadDialogues(const std::string& filename);
    void animateText(const std::string& text);
    void showDialogue(int index);
    std::string getDialogue(int index);
    size_t getDialogueCount();
    
private:
    int maxWidth;
    std::string speakerName;
    int textDelay;
    bool canSkip;
    std::vector<std::string> dialogues;
};

// Helper functions for showing dialogue at specific game points
void showGameIntroduction();
void showRoundInfo(int round);
void showRoundComplete(int round, int score);
void gameOver(bool won, int finalRound, int score);

#endif // TEXT_ANIMATOR_H