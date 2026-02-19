#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>

using namespace sf;
using namespace std;

// --- DATA STRUCTURES ---
struct Player {
    string name = "Guest";
    int gamesPlayed = 0;
    int gamesWon = 0;
};

// --- CONSTANTS ---
const int TILE_SIZE = 32;
const int OFFSET_X = 40; 
const int OFFSET_Y = 40;
const int WINDOW_WIDTH = 400; 
const int WINDOW_HEIGHT = 500; 

// --- COLORS ---
const Color BG_CLR(20, 22, 28);         
const Color ACCENT_CLR(0, 200, 220);    
const Color WHITE_CLR(220, 220, 220);   
const Color MUTED_CLR(100, 100, 100);   
const Color DANGER_CLR(220, 70, 70);    
const Color SUCCESS_CLR(80, 220, 150);  

// --- FILE IO ---
void saveStats(const Player& p) {
    ofstream f("stats.txt");
    if (f.is_open()) { f << p.name << "\n" << p.gamesPlayed << "\n" << p.gamesWon; f.close(); }
}
Player loadStats() {
    Player p; ifstream f("stats.txt");
    if (f.is_open()) { getline(f, p.name); f >> p.gamesPlayed >> p.gamesWon; f.close(); } 
    else saveStats(p); return p;
}

// --- GAME LOGIC ---
void initGrid(int grid[12][12], int sgrid[12][12]) {
    for (int i=0; i<12; i++) for (int j=0; j<12; j++) { sgrid[i][j]=10; grid[i][j]=0; }
    for (int i=1; i<=10; i++) for (int j=1; j<=10; j++) if (rand()%7==0) grid[i][j]=9;
    for (int i=1; i<=10; i++) for (int j=1; j<=10; j++) {
        if (grid[i][j]==9) continue;
        int c=0;
        for (int dx=-1; dx<=1; dx++) for (int dy=-1; dy<=1; dy++) if (grid[i+dx][j+dy]==9) c++;
        grid[i][j]=c;
    }
}
void openEmpty(int x, int y, int grid[12][12], int sgrid[12][12]) {
    if (x<1 || x>10 || y<1 || y>10 || sgrid[x][y]!=10) return;
    sgrid[x][y] = grid[x][y];
    if (grid[x][y]==0) for (int i=-1; i<=1; i++) for (int j=-1; j<=1; j++) openEmpty(x+i, y+j, grid, sgrid);
}

// --- UI HELPER ---
void centerText(Text& t, float x, float y) {
    FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width/2.0f, r.top + r.height/2.0f);
    t.setPosition(x, y);
}

// --- MAIN ---
int main() {
    srand(static_cast<unsigned int>(time(0)));
    RenderWindow w(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Minesweeper Ultimate", Style::Titlebar | Style::Close);
    w.setFramerateLimit(60);

    Font font; 
    // Fallback if font missing
    if (!font.loadFromFile("arial.ttf")) cout << "Font missing!" << endl;

    Texture tex; tex.loadFromFile("tiles.jpg"); Sprite sp(tex);

    Music music; music.openFromFile("music.wav"); music.setLoop(true); 
    float masterVol = 50.0f; music.setVolume(masterVol); music.play();

    SoundBuffer b1,b2,b3,b4; 
    b1.loadFromFile("click.wav"); b2.loadFromFile("explosion.wav");
    b3.loadFromFile("win.wav"); b4.loadFromFile("type.wav");
    Sound sndClick(b1), sndBoom(b2), sndWin(b3), sndType(b4);

    Player player = loadStats();
    int grid[12][12], sgrid[12][12];
    bool over=false, won=false, dragging=false;
    
    bool mouseReleased = true; // Safety latch

    enum State { MENU, OPTIONS, PLAY, NAME, HISTORY };
    State state = MENU;

    int sel = 0;
    string mainM[] = { "PLAY", "OPTIONS", "EXIT" };
    string optM[] = { "STATS", "CHANGE NAME", "HISTORY", "BACK" };
    string input = "";
    int fade = 0;

    while (w.isOpen()) {
        Event e;
        Vector2f m = w.mapPixelToCoords(Mouse::getPosition(w));

        while (w.pollEvent(e)) {
            if (e.type == Event::Closed) w.close();
            
            // Safety Latch Reset
            if (e.type == Event::MouseButtonReleased && e.mouseButton.button == Mouse::Left) {
                mouseReleased = true;
                dragging = false;
            }

            // --- MENU & OPTIONS INTERACTION ---
            if (state == MENU || state == OPTIONS) {
                int count = (state == MENU) ? 3 : 4;
                
                // Slider Dragging
                if (state == OPTIONS && e.type == Event::MouseButtonPressed && m.x > 90 && m.x < 290 && m.y > 300 && m.y < 330) {
                     dragging = true;
                }

                for (int i = 0; i < count; i++) {
                    // Manual Layout Engine (Matches Drawing Logic)
                    float yPos = 0;
                    if (state == MENU) yPos = 200 + (i * 50);
                    else {
                        // ADJUSTED POSITIONS TO FIX MERGING
                        if (i == 0) yPos = 160; // Stats
                        if (i == 1) yPos = 205; // Name
                        if (i == 2) yPos = 250; // History (Moved up away from slider)
                        if (i == 3) yPos = 420; // Back
                    }

                    // Hitbox
                    if (m.y > yPos - 20 && m.y < yPos + 20 && m.x > 50 && m.x < 350 && !dragging) {
                        if (sel != i) { sel = i; sndClick.play(); }
                        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left && mouseReleased) {
                             if (state == MENU) {
                                if (i == 0) { 
                                    state = PLAY; over=false; won=false; fade=0; 
                                    mouseReleased = false; 
                                    player.gamesPlayed++; saveStats(player); initGrid(grid, sgrid); 
                                }
                                if (i == 1) { 
                                    state = OPTIONS; sel = 0; 
                                    mouseReleased = false; // Prevent double click
                                    break; // FIX: Stop loop immediately to prevent bleed-through
                                }
                                if (i == 2) w.close();
                            } 
                            else if (state == OPTIONS) {
                                if (i == 0) state = HISTORY;
                                if (i == 1) { state = NAME; input = ""; }
                                if (i == 2) state = HISTORY;
                                if (i == 3) { state = MENU; sel = 0; break; } // Fix: Break loop
                            }
                        }
                    }
                }
            }
            // --- GAMEPLAY ---
            else if (state == PLAY) {
                if (e.type == Event::MouseButtonPressed && !over && !won && mouseReleased) {
                    int gx = (m.x - OFFSET_X) / TILE_SIZE + 1;
                    int gy = (m.y - OFFSET_Y) / TILE_SIZE + 1;
                    if (gx >= 1 && gx <= 10 && gy >= 1 && gy <= 10) {
                        if (e.mouseButton.button == Mouse::Left) {
                            if (grid[gx][gy] == 9) {
                                over = true; sndBoom.play();
                                for (int i=1; i<=10; i++) for (int j=1; j<=10; j++) sgrid[i][j] = grid[i][j];
                            } else {
                                sndClick.play();
                                openEmpty(gx, gy, grid, sgrid);
                            }
                        }
                        if (e.mouseButton.button == Mouse::Right) {
                             sndClick.play();
                             sgrid[gx][gy] = (sgrid[gx][gy] == 10) ? 11 : (sgrid[gx][gy] == 11 ? 10 : sgrid[gx][gy]);
                        }
                        int hidden=0, mines=0;
                        for (int i=1; i<=10; i++) for (int j=1; j<=10; j++) {
                            if (grid[i][j] == 9) mines++;
                            if (sgrid[i][j] >= 10) hidden++;
                        }
                        if (hidden == mines && !over) { won = true; sndWin.play(); player.gamesWon++; saveStats(player); }
                    }
                }
                if ((over || won) && e.type == Event::KeyPressed) state = MENU;
            }
            // --- TEXT INPUT ---
            else if (state == NAME && e.type == Event::TextEntered) {
                if (e.text.unicode == 8 && !input.empty()) input.pop_back();
                else if (e.text.unicode == 13) { if(!input.empty()) player.name = input; saveStats(player); state = OPTIONS; }
                else if (e.text.unicode < 128) { input += static_cast<char>(e.text.unicode); sndType.play(); }
            }
            else if (state == HISTORY && e.type == Event::KeyPressed) state = OPTIONS;
        }

        // --- UPDATES ---
        if (state == OPTIONS && dragging) {
            float mx = max(100.0f, min(m.x, 300.0f));
            masterVol = ((mx - 100.0f) / 200.0f) * 100.0f;
            music.setVolume(masterVol); sndClick.setVolume(masterVol); sndBoom.setVolume(masterVol);
        }

        w.clear(BG_CLR);

        // --- DRAWING ---
        if (state == MENU || state == OPTIONS) {
            Text title("MINESWEEPER\n  ULTIMATE", font, 38);
            title.setFillColor(ACCENT_CLR); title.setStyle(Text::Bold);
            centerText(title, WINDOW_WIDTH/2, 80); w.draw(title);

            int count = (state == MENU) ? 3 : 4;
            for (int i = 0; i < count; i++) {
                Text item; item.setFont(font); item.setCharacterSize(22);
                item.setString((state == MENU) ? mainM[i] : optM[i]);
                item.setFillColor(i == sel ? ACCENT_CLR : WHITE_CLR);
                
                // Matches Hitbox Logic Exactly
                float yPos = 0;
                if (state == MENU) yPos = 200 + (i * 50);
                else {
                    if (i == 0) yPos = 160; 
                    if (i == 1) yPos = 205;
                    if (i == 2) yPos = 250;
                    if (i == 3) yPos = 420;
                }
                centerText(item, WINDOW_WIDTH/2, yPos); w.draw(item);
            }

            if (state == OPTIONS) {
                // Slider at Y=310, Text at Y=335. Clean separation from History(250).
                RectangleShape bar(Vector2f(200, 6)), knob(Vector2f(10, 20));
                bar.setOrigin(100, 3); bar.setPosition(WINDOW_WIDTH/2, 310); bar.setFillColor(MUTED_CLR);
                knob.setOrigin(5, 10); knob.setPosition(100 + (masterVol/100.0f)*200, 310); knob.setFillColor(ACCENT_CLR);
                w.draw(bar); w.draw(knob);
                Text vText("Volume: "+to_string((int)masterVol)+"%", font, 14);
                vText.setFillColor(MUTED_CLR); centerText(vText, WINDOW_WIDTH/2, 335); w.draw(vText);
            }
        }
        else if (state == PLAY) {
            for (int i=1; i<=10; i++) for (int j=1; j<=10; j++) {
                sp.setTextureRect(IntRect(sgrid[i][j]*32, 0, 32, 32));
                sp.setPosition(OFFSET_X+(i-1)*32, OFFSET_Y+(j-1)*32); w.draw(sp);
            }
            if (over || won) {
                fade = min(fade+5, 200);
                RectangleShape ov(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT)); ov.setFillColor(Color(0,0,0,fade)); w.draw(ov);
                Text res(over ? "GAME OVER" : "VICTORY!", font, 36);
                res.setFillColor(over ? DANGER_CLR : SUCCESS_CLR); centerText(res, WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - 20); w.draw(res);
                Text sub("Press any key", font, 16); sub.setFillColor(WHITE_CLR); centerText(sub, WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 30); w.draw(sub);
            }
        }
        else if (state == HISTORY || state == NAME) {
             Text t(state==NAME ? "ENTER NEW NAME:\n\n"+input+"|" : "PLAYER: "+player.name+"\n\nGAMES: "+to_string(player.gamesPlayed)+"\nWINS: "+to_string(player.gamesWon), font, 20);
             t.setFillColor(WHITE_CLR); centerText(t, WINDOW_WIDTH/2, WINDOW_HEIGHT/2); w.draw(t);
        }
        w.display();
    }
    return 0;
}