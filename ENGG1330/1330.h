#ifndef GAME_1330_H
#define GAME_1330_H

#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>

enum class LaserDirection {
    HORIZONTAL,
    VERTICAL,
    DIAGONAL_DOWN,
    DIAGONAL_UP,
    KNIGHT
};

class Heart;
class BattleBox;
class Snake;
class Laser;
class Knight;

bool runRound(int round);
bool simplified_runRound(int round);
bool runRound(int round, int initialHP = 10);
int getLastHP();

#endif