#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>
#include <string>

using std::string;
using std::vector;

// Function prototypes
void initMap(int gridWidth, int gridHeight, vector<vector<int>>& map);
void drawMap(int gridWidth, int gridHeight, vector<vector<int>>& map, SDL_Renderer* renderer);
void drunkardsWalk(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int iterations);
void cellularAutomata(int gridWidth, int gridHeight, vector<vector<int>>& map);
int countNeighbors(int gridWidth, int gridHeight, vector<vector<int>>& map, int x, int y);
void outlineTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile);
void replaceTiles(int gridWidth, int gridHeight, vector<vector<int>>& map, int tile, int targetTile, int replacementTile, int threshold);
int getRandom(int low, int high);
void showInventory(SDL_Renderer* renderer, TTF_Font* font);
string randomName();

// Globals
int cellSize = 10;
bool bInventoryOpen = false;
SDL_Texture* inventoryTexture = nullptr;
SDL_Surface* inventorySurface = nullptr;

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

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                int newX = x, newY = y;
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:  if(!bInventoryOpen) newX = x - speed; break;
                    case SDLK_RIGHT: if(!bInventoryOpen) newX = x + speed; break;
                    case SDLK_UP:    if(!bInventoryOpen) newY = y - speed; break;
                    case SDLK_DOWN:  if(!bInventoryOpen) newY = y + speed; break;
                    //case SDLK_0: cellSize = cellSize + 1; break;
                    //case SDLK_9: cellSize = cellSize - 1; break;
                    case SDLK_8: 
                        bInventoryOpen = !bInventoryOpen;
                        //showInventory(renderer, font); 
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
            
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Welcome my guy", color);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }

            // Draw inventory if open
            if(bInventoryOpen) {            
                showInventory(renderer, font);
            }



        // Draw player
        SDL_Rect rect = {x, y, cellSize, cellSize};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

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
//    bInventoryOpen = !bInventoryOpen;
    
    if(bInventoryOpen) {
        // Create inventory background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
        SDL_Rect inventoryBG = {10, 10, 200, 300};
        SDL_RenderFillRect(renderer, &inventoryBG);
        
        // Create text
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* inventorySurface = TTF_RenderText_Solid(font, "Inventory", textColor);
        SDL_Texture* inventoryTexture = SDL_CreateTextureFromSurface(renderer, inventorySurface);
        
        // Render text
        SDL_Rect textRect = {20, 20, inventorySurface->w, inventorySurface->h};
        SDL_RenderCopy(renderer, inventoryTexture, NULL, &textRect);
        
        // Clean up
        SDL_FreeSurface(inventorySurface);
        SDL_DestroyTexture(inventoryTexture);
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