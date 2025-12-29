#include <iostream>
#include <cstdio>
#include <time.h>
#include <SFML/Graphics.hpp>   //included SFML library
#include <optional>

using namespace sf;           // required to use SFML libraray
using namespace std;
int main()
{

    srand(time(0));

    const int gridSize = 12;     // 12 x 12 board
    const int tileSize = 32;    // pixels per tile
    const int windowWidth = 10 * tileSize;
    const int windowHeight = 10 * tileSize;

    RenderWindow game(VideoMode({(unsigned int)windowWidth, (unsigned int)windowHeight}), "SFML Minesweeper");   // create window object

    Texture tileTexture;   // Texture object to load image

    if(!tileTexture.loadFromFile("tiles.jpg"))   // load the image file into texture
    {
        cout << "Error loading tiles.jpg" <<endl;
        cout << "Press Enter to exit...";
        cin.get();
        return -1;                                       // exit if image not found
    }

    Sprite s(tileTexture);   // create sprite object from texture   (sprite is drawable object in SFML, we are pasting the texture over it)
    
    int w = 32;  // size of the tile in the image
    int grid[gridSize][gridSize] = {0};   // The "Truth" (Actual values) (2D array to represent the grid)
    int sgrid[gridSize][gridSize] = {0};  //  The "View" (What player sees)
    // = {0}; initializes all elements to zero to avoid garbage values
    
    //logic to randomly place mines in the grid
    for (int i = 1; i <= 10; i++) 
    {
        for (int j = 1; j <= 10; j++) 
        {
            sgrid[i][j] = 10; // Hidden
            if (rand() % 5 == 0)  // 20% chance to place a mine (0, 1, 2, 3, 4)
                grid[i][j] = 9;   // Mine
            else 
                grid[i][j] = 0;  // Empty
        }
    }

    //logic to calculate numbers for non-mine cells
    for (int i = 1; i <= 10; i++)             // 10 x 10 board inside the 12 x 12 grid
        for (int j = 1; j <= 10; j++)
        {
            if (grid[i][j] == 9) continue;   // skip mines
            int n = 0;                      // count of adjacent mines
            // checking all 8 adjacent cells
            if (grid[i+1][j] == 9) n++;     // down
            if (grid[i][j+1] == 9) n++;     // right
            if (grid[i-1][j] == 9) n++;     // up
            if (grid[i][j-1] == 9) n++;     // left
            if (grid[i+1][j+1] == 9) n++;   // down-right
            if (grid[i-1][j-1] == 9) n++;   // up-left
            if (grid[i-1][j+1] == 9) n++;   // up-right
            if (grid[i+1][j-1] == 9) n++;   // down-left
            grid[i][j] = n;   // set the count of adjacent mines (if 3)
        }

    
    // Game Loop
   while (game.isOpen())
    {
        // Handle Events
        while (const std::optional event = game.pollEvent())
        {
            // Close Window
            if (event->is<Event::Closed>())
                game.close();

            // Mouse Clicks
            if (const auto* mousePress = event->getIf<Event::MouseButtonPressed>())
            {
                // Get position from the mouse click directly
                int x = mousePress->position.x / w;
                int y = mousePress->position.y / w;

                // Safety Check: Stay inside the 1-10 grid
                if (x >= 0 && x < 10 && y >= 0 && y < 10) 
                {
                    // Left Click = Reveal
                    if (mousePress->button == Mouse::Button::Left)
                    {
                        sgrid[x+1][y+1] = grid[x+1][y+1];
                    
                        // Game Over Logic
                        if (grid[x+1][y+1] == 9) 
                        {
                            cout << "BOOM! Game Over!" << endl;
                            for (int i = 1; i <= 10; i++)
                                for (int j = 1; j <= 10; j++)
                                    sgrid[i][j] = grid[i][j];
                        }
                    }
                        
                    // Right Click = Flag
                    else if (mousePress->button == Mouse::Button::Right)
                    {
                        // Only flag if it's currently hidden (10) or already a flag (11)
                        if (sgrid[x+1][y+1] == 10) sgrid[x+1][y+1] = 11;
                        else if (sgrid[x+1][y+1] == 11) sgrid[x+1][y+1] = 10;
                    }
                }
            }
        }

        game.clear(Color::White);
        
        // Draw Grid
        for (int i = 1; i <= 10; i++)
        {
            for (int j = 1; j <= 10; j++)
            {
                // 1. Decide which picture to show
                int tileIndex = sgrid[i][j];
                
                // 2. Crop that picture from the strip
                s.setTextureRect(IntRect({tileIndex * w, 0}, {w, w}));
                
                // 3. Move the sticker to the right spot
                s.setPosition({(float)(i-1)*w, (float)(j-1)*w});
                
                // 4. Draw it
                game.draw(s);
            }
        }

        game.display();
        
}
    return 0;
}