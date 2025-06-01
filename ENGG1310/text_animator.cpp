#include "text_animator.h"
#include <unistd.h>
#include <sstream>
#include <vector>

TextAnimator::TextAnimator(int width, const std::string& name, int delay, bool skip) 
    : maxWidth(width), speakerName(name), textDelay(delay), canSkip(skip) {}

void TextAnimator::animateText(const std::string& text) {
    // Get screen dimensions
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Position dialogue at bottom of screen (Genshin Impact style)
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
    attron(COLOR_PAIR(3)); // Yellow for name
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
        // Check if next word fits on current line
        // Fix sign comparison by casting to int
        if (currentX + static_cast<int>(words[i].length()) >= startX + maxWidth) {
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
        currentX += static_cast<int>(words[i].length()) + 1;
        
        // Check for key press to skip
        if (canSkip) {
            int ch = getch();
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                // Print all remaining text at once
                for (size_t j = i + 1; j < words.size(); j++) {
                    // Check if next word fits on current line
                    // Fix sign comparison by casting to int
                    if (currentX + static_cast<int>(words[j].length()) >= startX + maxWidth) {
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
                    currentX += static_cast<int>(words[j].length()) + 1;
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
    attron(COLOR_PAIR(3)); // Yellow for prompt
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
}

void showGameIntroduction() {
    // Clear screen
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 100, true);
    
    // Introduction sequence
    animator.animateText("Welcome to ENGG1310 - Electricity and electronics! I'm your Professor for today's practical assessment.");
    
    clear();
    animator.animateText("Today, you'll be testing your memory and reflexes in our Safe Tiles challenge. This exercise simulates working with electrical circuits under pressure.");
    
    clear();
    animator.animateText("The rules are simple: you'll be shown a grid with some safe tiles that will flash briefly. Your task is to memorize these safe tiles and step ONLY on them.");
    
    clear();
    animator.animateText("WARNING: Stepping on unsafe tiles will trigger our electrical defense system, resulting in a painful shock! Or as I like to call it - a shocking experience!");
    
    clear();
    animator.animateText("You'll start with 10 HP. Each time you step on an unsafe tile, you'll be hit by an electric laser and lose 5 HP. If your HP reaches zero, you fail the course.");
    
    clear();
    animator.animateText("As you progress through rounds, the challenge increases: fewer safe tiles, shorter flash time, and the dreaded red border countdown before each check!");
    
    clear();
    animator.animateText("Use the arrow keys to navigate. Remember - in electrical engineering, one wrong step can be shocking! Don't get zapped by those electric lasers!");
    
    clear();
    animator.animateText("Are you ready to test your electrical safety knowledge? Let's begin the assessment! Remember, this is for your own good - real electrical work requires focus and precision!");
    
    clear();
    maxY++;
    maxY--;
}

void showRoundInfo(int round, int numSafeTiles, float flashTime, int hp) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    
    std::ostringstream oss;
    oss << "Round " << round << " is about to begin! In this round, you'll need to find " 
        << numSafeTiles << " safe tiles. You'll have " << flashTime 
        << " seconds to memorize them. Current HP: " << hp 
        << ". Remember, don't get hit by the electric lasers!";
    
    animator.animateText(oss.str());
    maxY++;
    maxY--;
}

void gameOver(bool won, int finalRound) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    
    if (won) {
        animator.animateText("CONGRATULATIONS! You've successfully completed all " + std::to_string(finalRound) + 
                             " rounds of the electrical safety test. Your memory and reflexes are excellent - you'll make a fine electrical engineer!");
    } else {
        animator.animateText("COURSE FAILED! The electric lasers have depleted your health. You only reached round " + 
                            std::to_string(finalRound) + ". Better luck next semester - electrical safety is no joke!");
    }
    maxY++;
    maxY--;
}

void showResult(bool safe, int playerHP) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int dialogWidth = maxX * 3/4;
    
    TextAnimator animator(dialogWidth, "Professor", 80, true);
    
    if (safe) {
        animator.animateText("SAFE TILE! Excellent work. You've successfully navigated the electrical hazard. Current HP: " + std::to_string(playerHP));
    } else {
        animator.animateText("YOU GOT HIT BY A LASER! That's an unsafe tile! The electrical defense system has shocked you. You lose 5 HP! Current HP: " + std::to_string(playerHP));
    }
    maxY++;
    maxY--;
}
