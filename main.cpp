#include <iostream>
#include <cstdlib>              // for rand() and srand()
#include <time.h>               // for time()
#include <SFML/Graphics.hpp>    // included SFML library
#include <fstream>              // for file handling
#include <cassert>              // for assert()

using namespace sf;             // required to use SFML library
using namespace std;

// Structure to hold player statistics
struct Player
{
    string name;                // Player name
    int gamesPlayed = 0;        // Number of games played
    int gamesWon = 0;           // Number of games won
};

// Function to save player statistics into a file
void saveStats(Player p)
{
    ofstream fout("stats.txt"); // Opens file to WRITE
    assert(fout.is_open());     // Ensure file is open properly

    // Writing player data into file
    fout << p.name << endl;
    fout << p.gamesPlayed << endl;
    fout << p.gamesWon << endl;

    fout.close();               // Close the file
}

// Function to load player statistics from a file
Player loadStats()
{
    Player p;
    ifstream fin("stats.txt");  // Opens file to READ

    if (fin.is_open())
    {
        // Reading stored player data
        getline(fin, p.name);
        fin >> p.gamesPlayed;
        fin >> p.gamesWon;
        fin.close();
    }
    else
    {
        // Default values if no stats file exists
        p.name = "Guest";
        p.gamesPlayed = 0;
        p.gamesWon = 0;
    }
    return p;
}

// Function to open empty cells and their surrounding cells
// This function uses recursion to mimic real Minesweeper behavior
void openEmpty(int x, int y, int grid[12][12], int sgrid[12][12])
{
    // Boundary check to stay inside the grid
    if (x < 1 || x > 10 || y < 1 || y > 10)
        return;

    // If cell is already opened or flagged, stop recursion
    if (sgrid[x][y] != 10)
        return;

    // Reveal the current cell
    sgrid[x][y] = grid[x][y];

    // If the revealed cell is empty (0), open surrounding cells
    if (grid[x][y] == 0)
    {
        openEmpty(x + 1, y, grid, sgrid);
        openEmpty(x - 1, y, grid, sgrid);
        openEmpty(x, y + 1, grid, sgrid);
        openEmpty(x, y - 1, grid, sgrid);
        openEmpty(x + 1, y + 1, grid, sgrid);
        openEmpty(x - 1, y - 1, grid, sgrid);
        openEmpty(x + 1, y - 1, grid, sgrid);
        openEmpty(x - 1, y + 1, grid, sgrid);
    }
}

// Function that contains ONLY the SFML game logic
// Terminal menu will call this function when user selects "Play Game"
void runGame()
{
    const int gridSize = 12;     // 12 x 12 grid (extra border for safety)
    const int tileSize = 32;     // Size of each tile in pixels
    const int windowSize = 10 * tileSize; // Game window size

    RenderWindow game(VideoMode(windowSize, windowSize), "SFML Minesweeper");

    Texture tileTexture;         // Texture object to load image
    if (!tileTexture.loadFromFile("tiles.jpg"))
    {
        cout << "Error loading tiles.jpg" << endl;
        system("pause");
        return;                 // Exit game if image is missing
    }

    Sprite tile(tileTexture);    // Sprite object used to draw tiles

    int grid[gridSize][gridSize] = {0};   // Actual game data (mines & numbers)
    int sgrid[gridSize][gridSize] = {0};  // What the player sees

    // Randomly place mines and hide all cells
    for (int i = 1; i <= 10; i++)
        for (int j = 1; j <= 10; j++)
        {
            sgrid[i][j] = 10;             // 10 represents hidden tile
            grid[i][j] = (rand() % 5 == 0) ? 9 : 0; // 20% chance of mine
        }

    // Calculate numbers for non-mine cells
    for (int i = 1; i <= 10; i++)
        for (int j = 1; j <= 10; j++)
        {
            if (grid[i][j] == 9)
                continue;                 // Skip mines

            int count = 0;                // Count adjacent mines

            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                    if (grid[i + dx][j + dy] == 9)
                        count++;

            grid[i][j] = count;            // Store number of nearby mines
        }

    bool gameOver = false;                 // Flag to stop game after mine click

    // Main SFML game loop
    while (game.isOpen())
    {
        Event event;
        while (game.pollEvent(event))
        {
            // Close the window
            if (event.type == Event::Closed)
                game.close();

            // Handle mouse input only if game is not over
            if (event.type == Event::MouseButtonPressed && !gameOver)
            {
                int x = event.mouseButton.x / tileSize;
                int y = event.mouseButton.y / tileSize;

                // Ensure click is inside playable grid
                if (x >= 0 && x < 10 && y >= 0 && y < 10)
                {
                    int gx = x + 1;
                    int gy = y + 1;

                    // Left mouse click reveals tile
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        if (grid[gx][gy] == 9)
                        {
                            gameOver = true; // End the game

                            // Reveal entire grid when mine is clicked
                            for (int i = 1; i <= 10; i++)
                                for (int j = 1; j <= 10; j++)
                                    sgrid[i][j] = grid[i][j];

                            cout << "BOOM! Game Over!" << endl;
                        }
                        else
                        {
                            // Open empty cells and neighbors
                            openEmpty(gx, gy, grid, sgrid);
                        }
                    }

                    // Right mouse click places or removes a flag
                    if (event.mouseButton.button == Mouse::Right)
                    {
                        if (sgrid[gx][gy] == 10)
                            sgrid[gx][gy] = 11; // Place flag
                        else if (sgrid[gx][gy] == 11)
                            sgrid[gx][gy] = 10; // Remove flag
                    }
                }
            }
        }

        game.clear(Color::White);           // Clear previous frame

        // Draw the grid on the window
        for (int i = 1; i <= 10; i++)
            for (int j = 1; j <= 10; j++)
            {
                tile.setTextureRect(
                    IntRect(sgrid[i][j] * tileSize, 0, tileSize, tileSize));
                tile.setPosition((i - 1) * tileSize, (j - 1) * tileSize);
                game.draw(tile);
            }

        game.display();                     // Display everything
    }
}

int main()
{
    srand(time(0));                         // Seed random number generator
    Player currentPlayer = loadStats();     // Load saved player data

    // Terminal menu loop
    while (true)
    {
        system("cls");
        cout << "===========================\n";
        cout << "   MINESWEEPER ULTIMATE\n";
        cout << "===========================\n";
        cout << "Welcome back, " << currentPlayer.name << "!\n\n";
        cout << "1. Play Game\n";
        cout << "2. View Stats\n";
        cout << "3. Change Name\n";
        cout << "4. Exit\n";
        cout << "Choose option: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            // Increment games played before starting the game
            currentPlayer.gamesPlayed++;
            saveStats(currentPlayer);
            runGame();                      // Start SFML game
        }
        else if (choice == 2)
        {
            // Display player statistics
            cout << "\n--- PLAYER STATS ---\n";
            cout << "Name: " << currentPlayer.name << endl;
            cout << "Played: " << currentPlayer.gamesPlayed << endl;
            cout << "Won: " << currentPlayer.gamesWon << endl;
            system("pause");
        }
        else if (choice == 3)
        {
            // Change player name
            cout << "Enter new name: ";
            cin >> currentPlayer.name;

            // IMPORTANT UPDATE:
            // When name is changed, stats are reset
            // This treats the new name as a new player
            currentPlayer.gamesPlayed = 0;
            currentPlayer.gamesWon = 0;

            saveStats(currentPlayer);       // Save updated data
        }
        else if (choice == 4)
        {
            break;                          // Exit the program
        }
    }

    return 0;                               // Program ends
}
