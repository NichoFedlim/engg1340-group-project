# ENGG1340 Game Group Project
ENGG1340: Computer Programming II
-- Semester 2, Year 2024-2025

## Group 17 Team Members Name & UID:
1. Arthur Wirjolukito           - 3036445804
2. Darren Jonathan Christian    - 3036384696
3. Kelly Valerie                - 3036335815
4. Louise Danielle Sugiarto     - 3036459738
5. Nicholas Fedlim              - 3036385482
6. Rionaldo Arifin              - 3036320432

### Work Division / Contributions:
1. Arthur is mainly responsible for: _**ENGG1340, Makefile(s), Detailing, Text_animator**_
2. Darren is mainly responsible for: _**ENGG1330**_
3. Kelly is mainly responsible for: _**ENGG1300, Video Editing**_
4. Louise is mainly responsible for: _**ENGG1300, Video Editing**_
5. Nicholas is mainly responsible for: _**ENGG1320, README.md**_
6. Rionaldo is mainly responsible for: _**ENGG1310, Main_menu**_

## Game Title: The Year 1 Engineering Experience
ENGINEER'S GAMBIT: Code & Conquer!

__**Tagline**__: _"Code, Conquer, Triumph: Engineer's Gambit Unleashed!"_

## Game Overview:
Dive into "The Year 1 Engineering Experience", a pulse-pounding, terminal-based odyssey that fuses the thrill of arcade classics with the brain-bending challenges of engineering! Crafted in C++ for Linux servers, this 5-stage epic transforms core engineering concepts—physics, electricity, programming logic, and strategic combat—into an electrifying gauntlet of interactive trials. Inspired by First-Year Engineering Core Courses at The University of Hong Kong (HKU), each stage reimagines academic principles as high-stakes gameplay, from dodging sizzling electric tiles to outwitting a chess-inspired bullet-hell onslaught.

Embark on a relentless journey through five unique arenas, where precision, strategy, and lightning-fast reflexes are your keys to victory. Whether you’re weaving through laser mazes, blasting spaceships in a cosmic showdown, memorizing vanishing safe zones, shattering blocks with pinpoint accuracy, or facing a final, mind-bending gambit, every stage delivers a fresh twist on retro gaming with a modern engineering edge. This isn’t just a game—it’s a battle to conquer technical chaos, making it the ultimate playground for aspiring engineers to flex their computational prowess.

## Application Description:
The Year 1 Engineering Experience unfolds across five heart-racing stages, each a standalone C++ masterpiece powered by the ncurses library for vibrant, text-based visuals. Played in a Linux terminal, the game weaves a cohesive narrative of overcoming engineering trials, with each stage escalating in complexity and ingenuity:
1. **ENGG1300**: Control a paddle to bounce a ball and destroy blocks in three rounds with varying block density and lives. Accurate paddle placement and trajectory prediction are essential.
2. **ENGG1310**: Move a heart avatar across a grid to land on briefly flashed safe tiles in five rounds. Memory and precise movement are critical to avoid heavy damage.
3. **ENGG1320**: Race against time in a dynamic battle box, collecting various coins while evading moving obstacles across five rounds. Path optimization and resource management are crucial to secure a high score and advance.
4. **ENGG1330**: Navigate a heart-shaped avatar through a BattleBox, dodging escalating laser patterns, a pursuing snake, and knight-like ambushes across seven rounds. Survival hinges on pattern recognition and quick reflexes.
5. **ENGG1340**: Pilot a heart-shaped fighter, shooting down enemy spaceships, projectiles, and bombs in three rounds. Strategic laser fire and health management are key to achieving a high score.

With smooth 60 FPS gameplay, responsive controls, and a retro aesthetic, the game delivers an addictive mix of nostalgia and innovation, all while embedding real-world engineering principles like collision detection, randomized algorithms, and dynamic physics.

## Features:
1. Diverse Gameplay Mechanics: Five unique stages test different skills:
    * a. ENGG1300: Physics-driven paddle-and-ball dynamics.
    * b. ENGG1310: Memory-based navigation with time pressure.
    * c. ENGG1320: Time-based coin collection with path optimization and energy management.
    * d. ENGG1330: Bullet-hell dodging with complex patterns.
    * e. ENGG1340: Shoot-’em-up combat with scoring and health management.
2. Progressive Difficulty: Each stage escalates in complexity across rounds, from beginner-friendly to expert-level challenges.
3. Real-Time Interaction: Smooth 60 FPS gameplay with non-blocking input for responsive controls.
4. Engineering Focus: Implements key concepts like floating-point physics, collision detection, randomized generation, and state management.
5. Visual Feedback: Colourful text-based graphics using ncurses, with distinct colour pairs for objects (e.g., red paddles, cyan lasers).
6. Robust State Handling: Manages lives, scores, health, and round progression, with clear win/loss conditions.
7. Linux Compatibility: Optimized for Linux servers, with straightforward compilation and execution via makefiles.

## Non-Standard Libraries Used
The five stages collectively rely on the following non-standard libraries:
1. __**`ncursesw` (ncurses)**__: Used across all stages (ENGG1300, ENGG1310, ENGG1320, ENGG1330, and ENGG1340) for terminal-based graphics, input handling, and colour management. The wide-character version (__ncursesw__) supports enhanced character rendering.
    * a. Purpose: Enables text-based UI, real-time input (non-blocking), and colourised output (e.g., COLOR_PAIR for red hearts, green safe tiles).
    * b. Dependency: Requires libncursesw5-dev (or equivalent) installed on the Linux server.

No other non-standard libraries are explicitly required, as the games leverage standard C++ libraries (\<vector\>, \<cmath\>, \<random\>, \<algorithm\>, \<ctime\>, \<unistd.h\>, etc.) for core functionality.

## Compilation and Execution Instructions
The Year 1 Engineering Experience is designed to run on a Linux computer (like a school server). Follow these easy steps to set up and play the game. You don’t need to be a coding expert—just follow along carefully!

**Step 1: Set Up Your Computer**

Before you can play, you need to install a few things on your Linux computer. Don’t worry, these are simple commands you can copy and paste into your terminal (a black window where you type commands).

1. Open a Terminal:
   * On your Linux computer, find the terminal app (it might be called "Terminal" or "Console") and open it.
2. Install What the Game Needs: **(OPTIONAL)**
   * Type these commands one by one, pressing Enter after each line. You might need to type your password when asked:
      ```
      sudo apt-get update
      sudo apt-get install build-essential
      sudo apt-get install libncursesw5-dev
      ```
   * These commands install tools the game needs to work, like a program to build the game (g++) and a library for drawing the game in the terminal (ncurses).
3. Check If It Worked:
   * Type this command and press Enter:
   ```
   pkg-config --libs --cflags ncursesw
   ```
   * If it shows something like `-lncursesw`, you’re good to go! If it doesn’t work, ask a friend or teacher for help installing `libncursesw5-dev`.

**Step 2: Get the Game Files**

You need the game files on your computer. If you’re using a school server, they might already be there. If not:
1. Download the Game:
   * If the game is on GitHub (like in your project), you can download it with this command:
   ```
   git clone https://github.com/arthur-wirjo/ENGG1340-Game-Group-Project.git
   ```
   * This creates a folder called ENGG1340-Game-Group-Project with all the game files.
   * If you already have the files in a folder (like ENGG1340-Game-Group-Project), **skip this step**.

**Step 3: Go to the Game Folder**
   * In the terminal, move to the game folder/directory by typing this command and pressing Enter:
   ```
   cd ENGG1340-Game-Group-Project
   ```
   * To check you’re in the right place, type `ls` and press Enter. You should see files like main_menu.cpp, README.md, Makefile, and others.

**Step 4: Build and Run the Game**
   * The game comes with a special file called a Makefile that makes it easy to build and start the game.
   * Build and Start the Game in One Step:
      * Type this command and press Enter:
      ```
      make run
      ```
      * This command does two things: it builds the game (turns the code into a program) and then starts the game right away.
      * If everything works, you’ll see the game’s main menu in the terminal, and you can start playing!

**Step 5: Playing the Game** 
   * When the game starts, you’ll see a menu to pick one of the five stages (ENGG1300, ENGG1310, ENGG1320, ENGG1330, or ENGG1340).
   * Use the arrow keys to move around, press `Space` for actions (like shooting or pausing), and press `q` to quit a stage.
   * Each stage has its own instructions on the screen, so read them to learn how to play.

### If Something Goes Wrong

If `make run` doesn’t work:
   * You might see an error like make: *** No rule to make target 'run'. This means the Makefile isn’t set up correctly. Try these steps:
   1. Build the game first:
   ```
   make
   ```
   2. Then start the game:
   ```
   ./year1_experience
   ```
   * If you see an error about ncurses, double-check Step 1 to make sure `libncursesw5-dev` is installed.

#### If The Problem Persists:
1. Please run this code in your terminal/console:
   ```
   rm -rf ENGG1340-Game-Group-Project
   git clone https://github.com/arthur-wirjo/ENGG1340-Game-Group-Project.git
   ```
2. Then, enter your GitHub username and password.
3. Run this code again.
   ```
   make run
   ```

### If the Game Looks Weird:
   * Make sure your terminal window is big enough (at least 80 characters wide and 24 lines tall). You can resize it by dragging the edges.
   * If colours don’t show up, your terminal might not support them. Try using a different terminal app (like xterm).

### Cleaning Up
If you want to remove the game files that were built (to save space or start fresh):
Type this command and press Enter:
```
make clean
```
This removes the game program but keeps your code files safe.

## All Games & Mechanics Details:
### Stage 1: ENGG1300 – Bounce Field (Fundamental Mechanics)
Concept: Physics-based paddle control, strategic destruction
#### Gameplay:
1. Control a paddle to bounce a ball within a BattleBox arena, aiming to destroy all blocks across three rounds.
2. Engage the elements: 
    * a. Paddle: Moves horizontally to deflect the ball, with variable speed based on hit position and paddle motion. 
    * b. Ball: Bounces off walls, paddle, and blocks with realistic physics, adjusting direction based on impact. 
    * c. Blocks: Arranged in rows, destroyed on ball impact; clearing all blocks wins the round.
3. Survive and conquer: 
    * a. Lives: Lose a life if the ball falls off the bottom; zero lives end the game. 
    * b. Rounds: Progress through three escalating challenges: _Round 1_ (half-density blocks, 3 lives), _Round 2_ (full-density blocks, 3 lives), _Round 3_ (full-density blocks, 1 life).
4. Controls: Left/right arrows move the paddle, space toggles movement, Enter relaunches after losing a ball.

**Think:** Arkanoid meets Pinball -- a precision-driven test of geometry prediction and reactive control where every bounce reshapes the battlefield.

#### Key Engineering Principles:
1. Floating-point physics: Smooth ball and paddle movement with normalized direction vectors for accurate bounces.
2. Collision detection: Bounding box checks for ball-block and ball-paddle interactions, with dynamic direction adjustments based on hit position.
3. Randomized ball trajectories: Initial and post-paddle directions incorporate controlled randomness for varied gameplay.
4. Efficient rendering: Selective redrawing and optimized block management ensure a fluid 60 FPS experience.
5. State management: Tracks lives, block counts, and game states (win/loss/ball lost) for seamless round transitions.

The game escalates from a forgiving introduction to a high-stakes challenge, demanding precise paddle control and quick reflexes to clear dense block formations with limited lives.

**Pro tip:** Position the paddle to hit the ball near its edges for sharper angles, and keep moving to add spin, making it easier to target remaining blocks.

### Stage 2: ENGG1310 – Sneaky Tiles (Electricity & Memory)
Concept: Flash-memory navigation, precision movement under time pressure
#### Gameplay:
1. Navigate a heart-shaped avatar across a 10x8 grid of tiles within a centered GameBoard, aiming to land on designated safe tiles in five rounds.
2. Master the challenge: 
    * a. Safe Tiles: Randomly selected tiles (5 to 1 per round) flash briefly (0.5s to 0.1s) before disappearing, requiring memorization. 
    * b. Movement Phase: After the flash, move for 5 seconds to reach a safe tile. 
    * c. Health System: Start with 10 HP; landing off a safe tile costs 5 HP; zero HP ends the game.
3. Face escalating difficulty: 
    * a. Round 1: 5 safe tiles, 0.5s flash. 
    * b. Round 2: 5 tiles, 0.3s flash. 
    * c. Round 3: 3 tiles, 0.3s flash. 
    * d. Round 4: 1 tile, 0.3s flash. 
    * e. Round 5: 1 tile, 0.1s flash.
4. Controls: Arrow keys set direction, space toggles movement, with smooth, aspect-ratio-adjusted navigation.

**Think:** Memory meets Bomberman's danger tiles—a nerve-wracking test of spatial recall and pixel-perfect movement where hesitation means damage

#### Key Engineering Principles:
1. Floating-point movement: Aspect-ratio-compensated motion ensures smooth, precise player control across the grid.
2. Tile-based collision: Converts screen coordinates to board tiles, ensuring accurate safe tile detection while avoiding grid lines.
3. Randomized tile selection: Shuffled tile positions create varied, fair safe tile layouts each round.
4. Buffered rendering: Uses a pad window to eliminate flicker, maintaining a crisp 60 FPS display.
5. Timed state transitions: Manages countdowns, flash phases, and movement windows with precise timing for consistent challenge.

The game transforms from a manageable memory test into a heart-pounding dash, with dwindling safe tiles and fleeting glimpses pushing your recall and reflexes to the limit.

**Pro Tip**: During the flash, focus on memorizing one or two safe tiles closest to the center, and move decisively to the nearest one as soon as the movement phase begins.

### Stage 3: ENGG1320 – Coin Frenzy (Modern Engineering)
Concept: Precision navigation, strategic timing, and risk management
#### Gameplay:
1. Maneuver through a futuristic, BattleBox-style arena to collect coins under intensifying time constraints, with each level requiring more coins in less time.
2. Encounter three distinct coin types: 
    * a. Regular (*): Standard coins that safely increment your score. 
    * b. Special ($): Grant +5 seconds and an extra coin, extending your survival window. 
    * c. Traps (X): Dangerous coins that deduct 3 coins and 5 seconds unless you're invincible.
3. Overcome escalating challenges: 
    * a. Decreasing time limits: Each level reduces available time by 10 seconds, starting from 60 seconds in Level 1. 
    * b. Dynamic trap coins: Trap coins move unpredictably, demanding constant vigilance. 
    * c. Limited invincibility: Brief invincibility periods protect against traps but require strategic timing.

**Think:** A blend of Asteroids' precision movement and Geometry Wars' frenetic pace—a vibrant, high-stakes test of reflexes, spatial awareness, and calculated risk-taking.

#### Key Engineering Principles:
1. Floating-point collision detection: Ensures precise interactions between the player and coins for reliable gameplay.
2. Adaptive difficulty scaling: Coin requirements increase and time limits decrease per level, balancing challenge and engagement.
3. Randomized spawn algorithms: Coins spawn with controlled randomness, ensuring fair placement away from the player and other coins.
4. Time management mechanics: Special coins introduce time dilation, rewarding bold moves with extended playtime.
5. Real-time system optimization: Efficient rendering and input handling maintain smooth gameplay at ~60 FPS.

The game evolves from a straightforward coin hunt into a pulse-pounding survival challenge across 5 carefully crafted levels, where every decision could lead to victory or defeat.

**Pro tip**: Prioritize special coins early to build a time buffer, and use invincibility periods to safely navigate dense trap coin clusters.

### Stage 4: ENGG1330 – Chess Boss Bullet Hell (Programming I)
Concept: Precision dodging, pattern recognition, and survival under pressure
#### Gameplay:
1. Control a heart-shaped avatar navigating a BattleBox arena, evading an escalating gauntlet of laser attacks across seven rounds.
2. Face diverse threats: 
    * a. Lasers: Traverse horizontal, vertical, diagonal, or chess-inspired knight-move patterns, with projectile trails that linger briefly. 
    * b. Snake (Round 5+): A relentless pursuer that tracks your movements, requiring strategic kiting. 
    * c. Knight (Round 6+): Spawns randomly, firing lasers in knight-move patterns for sudden, unpredictable threats.
3. Survive increasingly complex attack waves: 
    * a. Dynamic patterns: From simple horizontal lines to tic-tac-toe grids, X-cross diagonals, and quadrant-based assaults. 
    * b. Quadrant warnings: In Round 4, visual cues highlight a safe quadrant, demanding quick repositioning. 
    * c. Health management: Start with 10 HP; each hit reduces HP, and depletion ends the game.

**Think:** A fusion of bullet-hell intensity like Touhou and strategic positioning akin to Chess—a neon-lit crucible testing reflexes, spatial awareness, and tactical foresight.

#### Key Engineering Principles:
1. Real-time collision detection: Precise floating-point checks ensure accurate interactions between the heart, lasers, snake, and knight.
2. Procedural attack generation: Randomized patterns (e.g., quadrant attacks, knight spawns) create varied, replayable challenges.
3. State machine logic: Manages complex attack cycles with warning, active, and cooldown phases, especially in Round 4’s quadrant attacks.
4. Dynamic object tracking: The snake’s pursuit algorithm balances challenge and fairness with delayed, alternating movements.
5. Optimized rendering: Efficient trail management and 60 FPS updates maintain smooth visuals despite multiple active elements.

The game transforms from a manageable dodge test into a frenetic survival ordeal, culminating in a chaotic final round combining lasers, a pursuing snake, and knight ambushes.

**Pro tip:** Memorize early patterns to anticipate gaps, and in Round 4, move to the safe quadrant as soon as warnings appear to avoid the dense laser barrages.

### Stage 5: ENGG1340 – C++ Space Laser Showdown (Programming II)
Concept: High-speed combat, strategic positioning, and resource management
#### Gameplay:
1. Pilot a heart-shaped fighter in a BattleBox arena, fending off waves of enemy spaceships across three escalating rounds.
2. Engage multiple threats: 
    * a. Spaceships: Move from right to left, firing projectiles; reaching the left edge damages you. 
    * b. Projectiles: Fired by enemies toward your position with slight randomness, requiring constant movement. 
    * c. Bombs (Rounds 2+): Drop from the top, exploding after a short timer if not destroyed.
3. Fight back and survive: 
    * a. Fire lasers to destroy enemies and their projectiles, earning points (100 for spaceships, 50 for bombs, 10 for projectiles). 
    * b. Manage health (10 HP start, 1 damage per hit) with brief invincibility after damage. 
    * c. Aim for a score of 2000 to win a round, while avoiding health depletion.
4. Face increasing difficulty: 
    * a. Round 1: Spaceships only, moderate spawn rate. 
    * b. Round 2: Faster spawns, introduces bombs. 
    * c. Round 3: Rapid spawns, frequent bombs, and more enemies.

**Think:** A blend of Galaga’s shoot-’em-up intensity and Asteroids’ precise navigation—a fast-paced, neon-charged test of reflexes, aiming accuracy, and tactical dodging.

#### Key Engineering Principles:
1. Floating-point movement: Smooth, aspect-ratio-adjusted motion for the player and enemies ensures precise control.
2. Collision detection: Integer-based checks for reliable interactions between lasers, enemies, projectiles, and the player.
3. Dynamic spawning algorithms: Randomized enemy and bomb placement with constrained trajectories ensure balanced challenge.
4. State management: Tracks invincibility, enemy cooldowns, and bomb timers for consistent gameplay.
5. Resource optimization: Efficient object cleanup and 60 FPS rendering handle multiple active entities without lag.

The game evolves from a manageable skirmish into a relentless onslaught, demanding quick decisions and precise laser shots to survive and triumph.

**Pro tip:** Stay near the left edge to maximize laser travel time against enemies, and prioritize bombs in Rounds 2 and 3 to prevent unavoidable damage.

## Additional Notes:
1. **Portability**: The game assumes a Linux environment with ncursesw. For other systems (e.g., WSL, macOS), install ncurses and adjust linker flags if needed (e.g., -lncurses on some systems).
2. **Error Handling**: If compilation fails, check for missing ncursesw (libncursesw5-dev) or incorrect file paths. Ensure g++ supports C++11 or later.
3. **Customization**: Adjust stage parameters (e.g., round counts, speeds) by modifying constants in source files (e.g., INITIAL_PLAYER_HEALTH in ENGG1340 - Space Battle).

This setup provides a robust framework for running Engineer's Gambit: Code & Conquer on a Linux server, delivering an engaging and educational gaming experience for ENGG1340 students.
