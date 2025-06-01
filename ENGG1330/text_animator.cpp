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
        if (line.empty() || line[0] == '#') continue;
        dialogues.push_back(line);
    }
    
    file.close();
}

void TextAnimator::animateText(const std::string& text) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int startY = maxY - 7;
    int startX = (maxX - maxWidth) / 2;
    
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }

    attron(COLOR_PAIR(3));
    mvprintw(startY - 1, startX, "%s:", speakerName.c_str());
    attroff(COLOR_PAIR(3));
    
    attron(COLOR_PAIR(4));
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

    if (canSkip) {
        mvprintw(startY - 2, startX + maxWidth - 22, "Press any arrow key to skip");
    }

    int currentX = startX;
    int currentY = startY;
    
    nodelay(stdscr, TRUE);
    
    bool completed = false;
    
    for (size_t i = 0; i < words.size() && !completed; i++) {
        if (currentX + static_cast<int>(words[i].length()) >= startX + maxWidth) {
            currentX = startX;
            currentY++;
            
            if (currentY >= startY + 4) {
                for (int y = startY; y < startY + 4; y++) {
                    mvhline(y, startX, ' ', maxWidth);
                }
                currentY = startY + 3;
            }
        }
        
        mvprintw(currentY, currentX, "%s", words[i].c_str());
        refresh();
        
        currentX += words[i].length() + 1;
        
        if (canSkip) {
            int ch = getch();
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                for (size_t j = i + 1; j < words.size(); j++) {
                    if (currentX + static_cast<int>(words[j].length()) >= startX + maxWidth) {
                        currentX = startX;
                        currentY++;
                        
                        if (currentY >= startY + 4) {
                            for (int y = startY; y < startY + 4; y++) {
                                mvhline(y, startX, ' ', maxWidth);
                            }
                            currentY = startY + 3;
                        }
                    }
                    
                    mvprintw(currentY, currentX, "%s", words[j].c_str());
                    
                    currentX += words[j].length() + 1;
                }
                refresh();
                completed = true;
                break;
            }
        }
        
        usleep(textDelay * 1000);
    }
    
    nodelay(stdscr, FALSE);
    
    attron(COLOR_PAIR(3));
    mvprintw(startY + 5, startX + maxWidth / 2 - 10, "Press Enter to continue...");
    attroff(COLOR_PAIR(3));
    refresh();
    
    flushinp();
    
    int ch;
    do {
        ch = getch();
    } while (ch != '\n' && ch != KEY_ENTER && ch != 10 && ch != 13);
    
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
    
    TextAnimator animator(dialogWidth, "Game Master", 100, true);
    animator.loadDialogues("1330_dialogue.txt");
    
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
    
    TextAnimator animator(dialogWidth, "Game Master", 80, true);
    animator.loadDialogues("1330_dialogue.txt");
    
    animator.showDialogue(7 + round);
    maxY++;
    maxY--;
}

void showRoundComplete(int round, bool success) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Game Master", 80, true);
    animator.loadDialogues("1330_dialogue.txt");
    
    if (success) {
        animator.showDialogue(14 + round);
    }
    maxY++;
    maxY--;
}

void gameOver(bool won, int finalRound) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Game Master", 80, true);
    animator.loadDialogues("1330_dialogue.txt");
    
    if (won) {
        animator.showDialogue(22);
    } else {
        std::string text = animator.getDialogue(23);
        size_t pos = text.find("ROUND");
        if (pos != std::string::npos) {
            text.replace(pos, 5, std::to_string(finalRound));
        }
        animator.animateText(text);
    }
    maxY++;
    maxY--;
}