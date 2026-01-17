#ifndef TETRISONLINE_GAME_H
#define TETRISONLINE_GAME_H

#include <random>
#include "objets.h"
#include <vector>
#include <string>
#include <set>

class Game {

    private:
    int score;
    int niveau;

    std::string msg;

    public:
    object grid;
    std::vector<object> objs;
    object current;
    object next;
    Music music;
    Sound rotate_sound;
    Sound destroy_sound;

    std::string get_msg() {
        return msg;
    }
    int get_score() {
        return score;
    }
    int get_niveau() {
        return niveau;
    }

    void set_msg(std::string m)
    {
        msg = m;
    }

    void set_score(int s) {
        score = s;
    }
    void set_niveau(int n) {
        niveau = n;
    }


    std::vector<object> get_all_objects() {
        object objT = object();
        objT.make_T();
        object objO = object();
        objO.make_O();
        object objI = object();
        objI.make_I();
        object objJ = object();
        objJ.make_J();
        object objL = object();
        objL.make_L();
        object objS = object();
        objS.make_S();
        object objZ = object();
        objZ.make_Z();
        return  {objT, objO, objI, objJ, objL, objS, objZ};

    }

    object get_random_object() {

        if (objs.empty()) {
            objs = get_all_objects();
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, objs.size() -1);

        int rand_num = dist(gen);
        object myobj= objs[rand_num];
        objs.erase(objs.begin() + rand_num);
        return myobj;
    }

    Game() {
        grid = object();
        //grid.matrice[grid.line-1][0] = 1;
        objs = get_all_objects();
        current = get_random_object();
        next = get_random_object();
        set_score(0);
        set_niveau(0);
        set_msg("");
        //InitAudioDevice();
        music = LoadMusicStream(    "sounds/cover.mp3");
        PlayMusicStream(music);
        rotate_sound = LoadSound("sounds/rotate.wav");
        destroy_sound = LoadSound("sounds/destroy.wav");
    }

    ~Game() {
        UnloadMusicStream(music);
        UnloadSound(destroy_sound);
        UnloadSound(rotate_sound);
        //CloseAudioDevice();
    }

    void apply_network_message(const std::string& msg) {
        if (msg.find("LINES|") == 0) {
            int n = std::stoi(msg.substr(6));
            add_garbage_lines(n);
            set_msg("ATTACK!");
        }

        if (msg.find("GAMEOVER") == 0) {
            set_msg("ENNEMI PERDU");
        }
    }

    void add_garbage_lines(int n) {
        for (int k = 0; k < n; k++) {

            // 1️⃣ Vérifier overflow (game over)
            for (int x = 0; x < grid.line; x++) {
                if (grid[0][x] != 0) {
                    game_over = true;
                    set_msg("GAME OVER");
                    return;
                }
            }

            // 2️⃣ Décaler la grille vers le haut
            for (int y = 0; y < HEIGHT - 1; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    grid[y][x] = grid[y + 1][x];
                }
            }

            // 3️⃣ Créer une ligne pleine avec un trou
            int hole = GetRandomValue(0, WIDTH - 1);

            for (int x = 0; x < WIDTH; x++) {
                grid[HEIGHT - 1][x] = (x == hole) ? 0 : 8;
            }
        }
    }




    void dessiner() {
        int x = grid.line;
        int y = grid.column;
        object temp1 = object(x, y);
        object temp2 = object(x, y);
        temp1 = grid;
        temp2 = current;
        if (temp2.checkintersection(temp1)) temp1.add(temp2);
        if (!loose()) temp1.dessiner();
        else grid.dessiner();

    }
    void reset() {
        grid = object();
        objs = get_all_objects();
        current = get_random_object();
        next = get_random_object();
        set_score(0);
        set_niveau(0);
        set_msg("");
    }

    void input() {
        int pressed_key = GetKeyPressed();
        if (pressed_key != 0 && loose()) {
            reset();
        }
        switch (pressed_key) {
            case KEY_RIGHT:
                if (current.check_right(grid)) {
                    current.translate_d();
                    break;
                }
                else break;


            case KEY_LEFT:
                if (current.check_left(grid)) {
                    current.translate_g();
                    break;
                }
                else break;

            case KEY_DOWN:
                if (current.check_collision(grid)==true && !loose() && current.checkintersection(grid)) {
                    grid.add(current);
                    //grid.printmatrice();
                    current = next;
                    next = get_random_object();
                }
                else current.translate_bas();
                break;

            case KEY_UP:
                if (current.check_rotate(grid)) {
                    rotate();
                    break;
                }
                else break;


            case KEY_ENTER:
                current.translate_haut();
                break;

        }
    }

    void rotate() {
        PlaySound(rotate_sound);
        current.rotate();
    }

    void move_down() {
        if (!loose()) {
            if (!current.checkintersection(grid)) {
                std::vector<std::vector<int>> P = current.get_pos();
                std::set<int> s(P[1].begin(), P[1].end());
                int n = s.size();
                for (int j = 0; j<n; j++) {
                    for (int i=0; i<current.line; i++) {
                        current.matrice[i][j] = 0;
                        current.translate_haut();
                    }
                }

            }
            if (current.check_collision(grid)) {
                grid.add(current);
                //grid.printmatrice();
                if (!loose()) {
                    current = next;
                    next = get_random_object();
                }

            }
            else {
                current.translate_bas();

            }
        }

    }

    bool loose() {
        for (int i=0; i < grid.line; i++) {
            if (grid.matrice[i][0]!=0) {
                set_msg("GAME OVER");
                return true;
            }
        }
        return false;
    }

    int calcscore0(int nb) {
        if (nb==1) return 40;
        if (nb==2) return 100-40;
        if (nb==3) return 300-100-40;
        if (nb==4) return 1200-200;
        return 0;
    }

    int calcscore(int nb, int niv) {
        int p = calcscore0(nb);
        return p*(niv+1);
    }

    void dessiner_next() {
        std::vector<std::vector<int>> P = next.get_pos();
        for (int i=0; i < 4; i++) {
            int ind_col = next.matrice[P[0][i]][P[1][i]];
            DrawRectangle((P[0][i])*next.cellsize +255, P[1][i]*next.cellsize +281, next.cellsize-1, next.cellsize-1, next.GetCellColors()[ind_col]);
        }
    }

    void destroy() {
        int nb = grid.destroy();
        if (nb!=0) {
            PlaySound(destroy_sound);
            int niv = get_niveau();
            int sc = get_score() + calcscore(nb, niv);
            set_score(sc) ;
            set_niveau(sc/1000);
            net.send_message("LINES|" + std::to_string(nb) + "\n");

        }

    }

};


#endif //TETRISONLINE_GAME_H

