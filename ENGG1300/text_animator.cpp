#include "text_animator.h"
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>

TextAnimator::TextAnimator(int width, const std::string& name, int delay, bool skip) 
    : maxWidth(width), speakerName(name), textDelay(delay), canSkip(skip) {}

void TextAnimator::loadDialogues(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open dialogue file: " << filename << std::endl;
        return;
    }
    
    dialogues.clear();
    while (std::getline(file, line)) {
        // Skip empty lines and comments (lines starting with #)
        if (line.empty() || line[0] == '#') continue;
        dialogues.push_back(line);
    }
    
    file.close();
}

void TextAnimator::animateText(const std::string& text) {
    // Get screen dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Position dialogue at bottom of screen
    int startY = maxY - 7; // 7 lines from bottom
    int startX = (maxX - maxWidth) / 2;
    
    // Split text into words
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }

    // Draw speaker name box
    attron(COLOR_PAIR(3)); // Cyan for name
    mvprintw(startY - 1, startX, "%s:", speakerName.c_str());
    attroff(COLOR_PAIR(3));
    
    // Draw dialogue box
    attron(COLOR_PAIR(4)); // Blue for box
    for (int i = 0; i < 5; i++) {
        mvhline(startY + i, startX - 2, ACS_HLINE, maxWidth + 4);
    }
    mvvline(startY, startX - 2, ACS_VLINE, 5);
    mvvline(startY, startX + maxWidth + 1, ACS_VLINE, 5);
    mvaddch(startY, startX - 2, ACS_ULCORNER);
    mvaddch(startY, startX + maxWidth + 1, ACS_URCORNER);
    mvaddch(startY + 4, startX - 2, ACS_LLCORNER);
    mvaddch(startY + 4, startX + maxWidth + 1, ACS_LRCORNER);
    attroff(COLOR_PAIR(4));

    // Add skip instruction
    if (canSkip) {
        mvprintw(startY - 2, startX + maxWidth - 25, "Press any arrow key to skip");
    }

    // Animate text
    int currentX = startX;
    int currentY = startY;
    
    nodelay(stdscr, TRUE); // Non-blocking input
    
    bool completed = false;
    
    for (size_t i = 0; i < words.size() && !completed; i++) {
        // Check if next word fits on current line - convert length to int
        int wordLength = static_cast<int>(words[i].length());
        if (currentX + wordLength >= startX + maxWidth) {
            currentX = startX;
            currentY++;
            
            // Check if we need to scroll up
            if (currentY >= startY + 4) {
                // Clear dialogue area and move up one line
                for (int y = startY; y < startY + 4; y++) {
                    mvhline(y, startX, ' ', maxWidth);
                }
                currentY = startY + 3;
            }
        }
        
        // Print the word
        mvprintw(currentY, currentX, "%s", words[i].c_str());
        refresh();
        
        // Add space after word
        currentX += wordLength + 1;
        
        // Check for key press to skip
        if (canSkip) {
            int ch = getch();
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                // Print all remaining text at once
                for (size_t j = i + 1; j < words.size(); j++) {
                    // Check if next word fits on current line - convert length to int
                    int nextWordLength = static_cast<int>(words[j].length());
                    if (currentX + nextWordLength >= startX + maxWidth) {
                        currentX = startX;
                        currentY++;
                        
                        // Check if we need to scroll up
                        if (currentY >= startY + 4) {
                            // Clear dialogue area and move up one line
                            for (int y = startY; y < startY + 4; y++) {
                                mvhline(y, startX, ' ', maxWidth);
                            }
                            currentY = startY + 3;
                        }
                    }
                    
                    // Print the word
                    mvprintw(currentY, currentX, "%s", words[j].c_str());
                    
                    // Add space after word
                    currentX += nextWordLength + 1;
                }
                refresh();
                completed = true;
                break;
            }
        }
        
        // Delay between words
        usleep(textDelay * 1000);
    }
    
    nodelay(stdscr, FALSE); // Return to blocking input
    
    // Wait for key press to continue
    attron(COLOR_PAIR(3)); // Cyan for prompt
    mvprintw(startY + 5, startX + maxWidth / 2 - 10, "Press Enter to continue...");
    attroff(COLOR_PAIR(3));
    refresh();
    
    // Clear input buffer
    flushinp();
    
    // Wait for Enter
    int ch;
    do {
        ch = getch();
    } while (ch != '\n' && ch != KEY_ENTER && ch != 10 && ch != 13);
    
    // Clear the dialogue area
    for (int y = startY - 2; y <= startY + 5; y++) {
        mvhline(y, startX - 2, ' ', maxWidth + 4);
    }
    refresh();
}

void TextAnimator::showDialogue(int index) {
    if (index >= 0 && index < static_cast<int>(dialogues.size())) {
        animateText(dialogues[index]);
    }
}

std::string TextAnimator::getDialogue(int index) {
    if (index >= 0 && index < static_cast<int>(dialogues.size())) {
        return dialogues[index];
    }
    return "";
}

size_t TextAnimator::getDialogueCount() {
    return dialogues.size();
}

void showGameIntroduction() {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 100, true);
    animator.loadDialogues("1300_dialogue.txt");
    
    // Introduction sequence - first 8 dialogues are intro
    for (int i = 0; i < 8; i++) {
        clear();
        animator.showDialogue(i);
    }
    clear();
    maxY++;
    maxY--;
}

void showRoundInfo(int round) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    animator.loadDialogues("1300_dialogue.txt");
    
    // Dialogues 8, 9, 10 are for rounds 1, 2, 3
    animator.showDialogue(7 + round);
    maxY++;
    maxY--;
}

void showRoundComplete(int round, int score) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    animator.loadDialogues("1300_dialogue.txt");
    
    // Get round completion dialogue and replace score placeholder
    std::string text = animator.getDialogue(10 + round);
    size_t pos = text.find("SCORE");
    if (pos != std::string::npos) {
        text.replace(pos, 5, std::to_string(score));
    }
    
    animator.animateText(text);
    maxY++;
    maxY--;
}

void gameOver(bool won, int finalRound, int score) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    animator.loadDialogues("1300_dialogue.txt");
    
    if (won) {
        // Get victory dialogue and replace score
        std::string text = animator.getDialogue(14);
        size_t pos = text.find("SCORE");
        if (pos != std::string::npos) {
            text.replace(pos, 5, std::to_string(score));
        }
        animator.animateText(text);
    } else {
        // Get game over dialogue and replace round/score
        std::string text = animator.getDialogue(15);
        size_t pos = text.find("ROUND");
        if (pos != std::string::npos) {
            text.replace(pos, 5, std::to_string(finalRound));
        }
        pos = text.find("SCORE");
        if (pos != std::string::npos) {
            text.replace(pos, 5, std::to_string(score));
        }
        animator.animateText(text);
        maxY++;
        maxY--;
    }
}