#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>
#include <string>

using std::string;
using std::vector;
using std::cout;
using std::endl;

// Function prototypes
void initMap(int gridWidth, int gridHeight, vector<vector<int>>& map);
void drawMap(int gridWidth, int gridHeight, vector<vector<int>>& map, SDL_Renderer* renderer);
void drunkardsWalk(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int iterations);
void cellularAutomata(int gridWidth, int gridHeight, vector<vector<int>>& map);
int countNeighbors(int gridWidth, int gridHeight, vector<vector<int>>& map, int x, int y);
void outlineTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile);
void replaceTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile, int replacementTile, int threshold);
int getRandom(int low, int high);
string randomName();
void showInventory(SDL_Renderer* renderer, TTF_Font* font);
void processBattle(SDL_Renderer* renderer, TTF_Font* font);
bool updateBattle(SDL_Event& event); 
void executeBattleTurn();
void levelUp(SDL_Renderer* renderer, TTF_Font* font);

// Globals
int cellSize = 10;
bool bInventoryOpen = false;
bool bBattleActive = false;

SDL_Texture* inventoryTexture = nullptr;
SDL_Surface* inventorySurface = nullptr;
string currentInventoryName = "";
string enemyName = "";
int playerHealth = 100;
int playerMaxHealth = 100;
int playerMana = 100;
int playerMaxMana = 100;
int playerLevel = 1;
int playerExperience = 0;
int playerExperienceToNextLevel = 100;
int playerAttack = 10;
int playerDefense = 5;
int playerSpeed = 5;
int playerGold = 0;

int enemyHealth = 0;//((playerLevel + 1) * 5) + getRandom(5, playerLevel * 5);
int enemyAttack = 0;//(playerLevel * 2) + getRandom(1, playerLevel * 2);
int enemyDefense = 0;//(playerLevel * 2) + getRandom(1, playerLevel * 2);
bool bPlayerTurn = true;
int selection = 0;


int main() {
    srand(time(NULL));
    int screenWidth = 900;
    int screenHeight = 600;
    int gridWidth = screenWidth / cellSize;
    int gridHeight = screenHeight / cellSize;
    vector<vector<int>> map(gridWidth, vector<int>(gridHeight));

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    // hide mouse cursor and replace with a custom cursor
    // SDL_ShowCursor(SDL_DISABLE);
    // SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR));
   //initialize SDL_ttf
    TTF_Init();

TTF_Font* font = TTF_OpenFont("fonts/00TT.TTF", 24);
if (!font) {
    std::cout << "Font loading error: " << TTF_GetError() << std::endl;
}
        
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 game",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // make fullscreen
    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Initialize map
    initMap(gridWidth, gridHeight, map);

    // Apply cellular automata
    int automataIterations = 10;
    for (int i = 0; i < automataIterations; i++) {
        cellularAutomata(gridWidth, gridHeight, map);
    }

    // Apply drunkard's walk
    drunkardsWalk(gridWidth, gridHeight, map, 2, 1000);
    outlineTiles(gridWidth, gridHeight, map, 3, 2);
    replaceTiles(gridWidth, gridHeight, map, 1, 1, 4, 4);
    replaceTiles(gridWidth, gridHeight, map, 2, 3, 2, 5);
    bool running = true;
    SDL_Event event;
    int x = screenWidth / 2; // Start in the middle
    int y = screenHeight / 2; 
    int speed = cellSize;
    string name = randomName();

    

    while (running) {
        if (playerExperience >= playerExperienceToNextLevel) {
            levelUp(renderer, font);
        }
        while (SDL_PollEvent(&event)) {
           
            if (event.type == SDL_QUIT) {
                running = false;}
            else if (event.type == SDL_KEYDOWN) {
                int newX = x, newY = y;
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:  
                        if(bBattleActive) {
                            selection = std::max(0, selection - 1);
                        } else if(!bInventoryOpen) {
                            newX = x - speed;
                        }
                        break;
                    case SDLK_RIGHT: 
                        if(bBattleActive) {
                            selection = std::min(1, selection + 1);
                        } else if(!bInventoryOpen) {
                            newX = x + speed;
                        }
                        break;
                    case SDLK_RETURN:
                        if(bBattleActive) {
                            updateBattle(event);
                        }
                        break;
                    case SDLK_UP:    if(!bInventoryOpen && !bBattleActive) newY = y - speed; break;
                    case SDLK_DOWN:  if(!bInventoryOpen && !bBattleActive) newY = y + speed; break;
                    case SDLK_1: playerHealth = playerHealth - 10; break;
                    case SDLK_7:
                    enemyName = randomName();
                    bBattleActive = true;
                    enemyHealth = ((playerLevel + 1) * 5) + getRandom(5, playerLevel * 5);
                    enemyAttack = (playerLevel * 2) + getRandom(1, playerLevel * 2);
                    enemyDefense = (playerLevel * 2) + getRandom(1, playerLevel * 2);
                    break;
                    case SDLK_8:
                    // Toggle inventory
                    currentInventoryName = randomName();
                        bInventoryOpen = !bInventoryOpen;
                        break;
                }
                // Move only if within bounds and on a floor tile
                if (newX >= 0 && newX < screenWidth &&
                    newY >= 0 && newY < screenHeight &&
                    (map[newX / cellSize][newY / cellSize] == 0 ||
                    map[newX / cellSize][newY / cellSize] == 3 ||
                    map[newX / cellSize][newY / cellSize] == 1)) {
                    x = newX;
                    y = newY;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);


            // Draw map
            drawMap(gridWidth, gridHeight, map, renderer);
        
            // draw text to screen
            SDL_Color color = {255, 255, 255};
            
            // SDL_Surface* textSurface = TTF_RenderText_Solid(font, name.c_str(), color);
            // if (textSurface) {
            //     SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            //     SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
            //     SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            //     SDL_FreeSurface(textSurface);
            //     SDL_DestroyTexture(textTexture);
            // }

            // Draw inventory if open
            if(bInventoryOpen) {            
                showInventory(renderer, font);
                //processBattle(renderer, font);
            }
            if(bBattleActive) {
                processBattle(renderer, font);
                SDL_RenderPresent(renderer);
            }



        // Draw player
        if(!bInventoryOpen && !bBattleActive) {
            SDL_Rect rect = {x, y, cellSize, cellSize};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
      

        // Update screen
        SDL_RenderPresent(renderer);
        
        //SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND));



    }

    TTF_CloseFont(font);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
void initMap(int gridWidth, int gridHeight, vector<vector<int>>& map) {
    for (int i = 0; i < gridWidth; i++) {
        for (int j = 0; j < gridHeight; j++) {
            // Initialize map: 1 = wall, 0 = floor
            if (i == 0 || i == gridWidth - 1 || j == 0 || j == gridHeight - 1) {
                map[i][j] = 1; // Border walls
            } else {
                map[i][j] = (rand() % 100 < 40) ? 1 : 0;
            }
        }
    }
}

void drawMap(int gridWidth, int gridHeight, vector<vector<int>>& map, SDL_Renderer* renderer) {
    for (int i = 0; i < gridWidth; i++) {
        for (int j = 0; j < gridHeight; j++) {
            SDL_Rect rect = {i * cellSize, j * cellSize, cellSize, cellSize};
            if (map[i][j] == 0) {
                SDL_SetRenderDrawColor(renderer, 100, 100, 25, 255); // Floor
            } else if (map[i][j] == 1) {
                SDL_SetRenderDrawColor(renderer, 50, 75, 50, 255); // Wall
            } else if (map[i][j] == 2) {
                SDL_SetRenderDrawColor(renderer, 10, 25, 150, 255); // Drunkard's path
            }
            else if (map[i][j] == 3) {
                SDL_SetRenderDrawColor(renderer, 125, 125, 50, 255); // Outline
            }
            else if (map[i][j] == 4) {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // Outline
            }
            SDL_RenderFillRect(renderer, &rect);
            
        }
    }
}

void drunkardsWalk(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int iterations) {
    int x = rand() % (gridWidth - 2) + 1;
    int y = rand() % (gridHeight - 2) + 1;

    map[x][y] = tile;
    for (int i = 0; i < iterations; i++) {
        int direction = rand() % 4;
        int newX = x, newY = y;
        switch (direction) {
            case 0: newX++; break;
            case 1: newX--; break;
            case 2: newY++; break;
            case 3: newY--; break;
        }
        if (newX > 0 && newX < gridWidth - 1 && 
            newY > 0 && newY < gridHeight - 1) {
            x = newX;
            y = newY;
            map[x][y] = tile;
        }
    }
}

int countNeighbors(int gridWidth, int gridHeight, vector<vector<int>>& map, int x, int y) {
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue; // Skip the current cell
            int nx = x + i, ny = y + j;
            if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                if (map[nx][ny] == 1) neighbors++;
            }
        }
    }
    return neighbors;
}

void cellularAutomata(int gridWidth, int gridHeight, vector<vector<int>>& map) {
    vector<vector<int>> newMap = map;
    for (int i = 0; i < gridWidth; i++) {
        for (int j = 0; j < gridHeight; j++) {
            int neighbors = countNeighbors(gridWidth, gridHeight, map, i, j);
            if (map[i][j] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    newMap[i][j] = 0; // Wall becomes floor
                }
            } else {
                if (neighbors == 3) {
                    newMap[i][j] = 1; // Floor becomes wall
                }
            }
        }
    }
    map = newMap;
}

void outlineTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile) {
    for (int i = 0; i < gridWidth; i++) {
        for (int j = 0; j < gridHeight; j++) {
            if (map[i][j] ==targetTile) 
            {
                //if surrounding tiles do not equal target tile, set surrounding tile to tile
                if (i > 0 && map[i-1][j] != targetTile) map[i-1][j] = tile;
                if (i < gridWidth-1 && map[i+1][j] != targetTile) map[i+1][j] = tile;
                if (j > 0 && map[i][j-1] != targetTile) map[i][j-1] = tile;
                if (j < gridHeight-1 && map[i][j+1] != targetTile) map[i][j+1] = tile;
                if (i > 0 && j > 0 && map[i-1][j-1] != targetTile) map[i-1][j-1] = tile;
                if (i < gridWidth-1 && j < gridHeight-1 && map[i+1][j+1] != targetTile) map[i+1][j+1] = tile;
                if (i > 0 && j < gridHeight-1 && map[i-1][j+1] != targetTile) map[i-1][j+1] = tile;
                if (i < gridWidth-1 && j > 0 && map[i+1][j-1] != targetTile) map[i+1][j-1] = tile;
                

        
            }
        }
    }

}

void replaceTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile, int replacementTile, int threshold) {
    for (int i = 0; i < gridWidth; i++) {
        for (int j = 0; j < gridHeight; j++) {
            // if targettile is completely surrounded by tiles, replace it with replacementtile
            if (map[i][j] == targetTile) {
                int neighbors = 0;
                if (i > 0 && map[i-1][j] == targetTile) neighbors++;
                if (i < gridWidth-1 && map[i+1][j] == targetTile) neighbors++;
                if (j > 0 && map[i][j-1] == targetTile) neighbors++;
                if (j < gridHeight-1 && map[i][j+1] == targetTile) neighbors++;
                if (i > 0 && j > 0 && map[i-1][j-1] == targetTile) neighbors++;
                if (i < gridWidth-1 && j < gridHeight-1 && map[i+1][j+1] == targetTile) neighbors++;
                if (i > 0 && j < gridHeight-1 && map[i-1][j+1] == targetTile) neighbors++;
                if (i < gridWidth-1 && j > 0 && map[i+1][j-1] == targetTile) neighbors++;
                if (neighbors >= threshold) {
                    map[i][j] = replacementTile;
                }
            }
            
            //if (map[i][j] == targetTile) {
            //    map[i][j] = tile;
            //}
        }
    }
}

int getRandom(int low, int high)
{
    // Set up a random device to seed the random number generator
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> distribution(low, high);
    int randomInteger = distribution(engine);

    return randomInteger;
}




void showInventory(SDL_Renderer* renderer, TTF_Font* font)
{
    if(bInventoryOpen) {
        
        
        // Create inventory background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
        SDL_Rect inventoryBG = {10, 10, 880, 580};
        SDL_RenderFillRect(renderer, &inventoryBG);
        
        // Create text using stored name
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* inventorySurface = TTF_RenderText_Solid(font, "Inventory", textColor);
        SDL_Texture* inventoryTexture = SDL_CreateTextureFromSurface(renderer, inventorySurface);
        
        // Render text
        SDL_Rect textRect = {20, 20, inventorySurface->w, inventorySurface->h};
        SDL_RenderCopy(renderer, inventoryTexture, NULL, &textRect);
        
        // Clean up
        SDL_FreeSurface(inventorySurface);
        SDL_DestroyTexture(inventoryTexture);
        // display player gold and stats
        SDL_Color textColor2 = {255, 255, 255};
        SDL_Surface* statsSurface = TTF_RenderText_Solid(font, ("Gold: " + std::to_string(playerGold)).c_str(), textColor2);

        SDL_Texture* statsTexture = SDL_CreateTextureFromSurface(renderer, statsSurface);
        SDL_Rect statsRect = {20, 60, statsSurface->w, statsSurface->h};
        SDL_RenderCopy(renderer, statsTexture, NULL, &statsRect);
        SDL_FreeSurface(statsSurface);
        SDL_DestroyTexture(statsTexture);
        // display player stats
        SDL_Surface* statsSurface2 = TTF_RenderText_Solid(font, ("Health: " + std::to_string(playerHealth)).c_str(), textColor2);
        SDL_Texture* statsTexture2 = SDL_CreateTextureFromSurface(renderer, statsSurface2);
        SDL_Rect statsRect2 = {20, 90, statsSurface2->w, statsSurface2->h};
        SDL_RenderCopy(renderer, statsTexture2, NULL, &statsRect2);
        SDL_FreeSurface(statsSurface2);
        SDL_DestroyTexture(statsTexture2);
        // display player experience
        SDL_Surface* statsSurface3 = TTF_RenderText_Solid(font, ("Experience: " + std::to_string(playerExperience)).c_str(), textColor2);
        SDL_Texture* statsTexture3 = SDL_CreateTextureFromSurface(renderer, statsSurface3);
        SDL_Rect statsRect3 = {20, 120, statsSurface3->w, statsSurface3->h};
        SDL_RenderCopy(renderer, statsTexture3, NULL, &statsRect3);
        SDL_FreeSurface(statsSurface3);
        SDL_DestroyTexture(statsTexture3);
        // display player level
        SDL_Surface* statsSurface4 = TTF_RenderText_Solid(font, ("Level: " + std::to_string(playerLevel)).c_str(), textColor2);
        SDL_Texture* statsTexture4 = SDL_CreateTextureFromSurface(renderer, statsSurface4);
        SDL_Rect statsRect4 = {20, 150, statsSurface4->w, statsSurface4->h};
        SDL_RenderCopy(renderer, statsTexture4, NULL, &statsRect4);
        SDL_FreeSurface(statsSurface4);
        SDL_DestroyTexture(statsTexture4);


    } else {
        // Reset the stored name when inventory is closed
        
    }
}
string randomName() {
    //string names[] = {"Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Henry", "Isabelle", "Jack", "Karen", "Liam", "Mia", "Noah", "Olivia", "Peter", "Quinn", "Rachel", "Sam", "Tina", "Uma", "Victor", "Wendy", "Xavier", "Yara", "Zach"};
    string fname1[] = {"Snar", "Bar", "Gnar", "Baz", "Bum"};
    string fname2[] = {"boo", "bum", "ple", "po", "poo"};
    string lname1[] = {"Snarboo", "Barbum", "Gnarple", "Bazpo", "Bumpo"};
    string lname2[] = {"boos", "bumple", "plepie", "pop", "plop"};
    string fullname = fname1[getRandom(0, 4)] + fname2[getRandom(0, 4)] + " " + lname1[getRandom(0, 4)] + lname2[getRandom(0, 4)];

    
    //int index = getRandom(0, 4);
    return fullname;
}

void processBattle(SDL_Renderer* renderer, TTF_Font* font) {

//    bPlayerTurn = true;
//    selection = 0;

    if (bBattleActive) {
        // Draw battle background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 100);
        SDL_Rect battleBG = {10, 10, 880, 580};
        SDL_RenderFillRect(renderer, &battleBG);

        // Enemy info
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* enemySurface = TTF_RenderText_Solid(font, enemyName.c_str(), textColor);
        SDL_Texture* enemyTexture = SDL_CreateTextureFromSurface(renderer, enemySurface);
        SDL_Rect enemyRect = {10, 10, enemySurface->w, enemySurface->h};
        SDL_RenderCopy(renderer, enemyTexture, NULL, &enemyRect);
        SDL_FreeSurface(enemySurface);
        SDL_DestroyTexture(enemyTexture);

        // Health bars
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect enemyHealthBar = {10, 50, enemyHealth, 20};
        SDL_RenderFillRect(renderer, &enemyHealthBar);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect playerHealthBar = {10, 80, playerHealth, 20};
        SDL_RenderFillRect(renderer, &playerHealthBar);

        // Player options
        const char* options[] = {"Attack", "Run"};
        for (int i = 0; i < 2; i++) {
            SDL_Rect button = {20 + i * 130, 110, 100, 25};
            SDL_SetRenderDrawColor(renderer, (i == selection) ? 0 : 0, (i == selection) ? 191 : 64, (i == selection) ? 255 : 128, 255);
            SDL_RenderFillRect(renderer, &button);

            SDL_Surface* textSurface = TTF_RenderText_Solid(font, options[i], textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {button.x, button.y, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }

        // Turn logic
        executeBattleTurn();
    }
}

// Add this function
bool updateBattle(SDL_Event& event) {
    if (!bBattleActive) return false;
    
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: 
                selection = std::max(selection - 1, 0);
                break;
            case SDLK_RIGHT: 
                selection = std::min(selection + 1, 1);
                break;
            case SDLK_RETURN:
                if (selection == 0) {
                    // Attack
                    int damage = playerAttack - enemyDefense;
                    damage = std::max(damage, 1); // Ensure minimum 1 damage
                    enemyHealth -= damage;
                    
                    if (enemyHealth <= 0) {
                        playerGold += getRandom(1, 10);
                        playerExperience += getRandom(1, 10);
                        bBattleActive = false;
                        return false;
                    }
                    
                    // Enemy counter-attack
                    int enemyDamage = enemyAttack - playerDefense;
                    enemyDamage = std::max(enemyDamage, 1);
                    playerHealth -= enemyDamage;
                    
                    if (playerHealth <= 0) {
                        playerHealth = 0;
                        bBattleActive = false;
                        return false;
                    }
                } else if (selection == 1) {
                    bBattleActive = false;
                    return false;
                }
                break;
        }
    }
    return true;
}

void executeBattleTurn() {
    if (!bPlayerTurn) {
        cout << "Enemy's turn" << endl;
        int damage = enemyAttack - playerDefense;
        damage = std::max(damage, 0);
        playerHealth -= damage;
        if (playerHealth <= 0) {
            playerHealth = 0;
            bBattleActive = false;
        }
        bPlayerTurn = true;
    }
}

void levelUp(SDL_Renderer* renderer, TTF_Font* font) {
    playerLevel++;
    //playerExperience = 0;
    playerExperienceToNextLevel = playerExperience + (playerLevel * 75) + (playerLevel * 50);
    playerMaxHealth += playerLevel * 10;
    playerHealth = playerMaxHealth;
    playerMaxMana += 10;
    playerMana = playerMaxMana;
    playerAttack += 5;
    playerDefense += 2;
    playerSpeed += 1;
    // display level up message
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface* levelUpSurface = TTF_RenderText_Solid(font, "Level up!", textColor);
    SDL_Texture* levelUpTexture = SDL_CreateTextureFromSurface(renderer, levelUpSurface);
    SDL_Rect levelUpRect = {10, 10, levelUpSurface->w, levelUpSurface->h};
    SDL_RenderCopy(renderer, levelUpTexture, NULL, &levelUpRect);
    SDL_FreeSurface(levelUpSurface);
    SDL_DestroyTexture(levelUpTexture);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    SDL_RenderClear(renderer);
    

}
