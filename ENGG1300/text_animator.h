#ifndef TEXT_ANIMATOR_H
#define TEXT_ANIMATOR_H

#include <ncursesw/ncurses.h>
#include <string>
#include <vector>

class TextAnimator {
private:
    int maxWidth;              // Maximum width of text box
    std::string speakerName;   // Name of the speaker
    int textDelay;             // Delay between words in milliseconds
    bool canSkip;              // Whether text can be skipped
    std::vector<std::string> dialogues; // Loaded dialogues
    
public:
    TextAnimator(int width, const std::string& name, int delay = 100, bool skip = true);
    
    // Helper function to animate a single text - moved to public
    void animateText(const std::string& text);
    
    // Load dialogues from a file
    void loadDialogues(const std::string& filename);
    
    // Show dialogue at the specified index
    void showDialogue(int index);
    
    // Get dialogue text at the specified index
    std::string getDialogue(int index);
    
    // Get the total number of loaded dialogues
    size_t getDialogueCount();
};

// Additional utility functions
void showGameIntroduction();
void showRoundInfo(int round);
void showRoundComplete(int round, int score);
void gameOver(bool won, int finalRound, int score);

#endif