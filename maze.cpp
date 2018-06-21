#include <curses.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <stack>
#include <chrono>
#include <thread>

using namespace std;

class DFS_maze_generator {
public:
    DFS_maze_generator(){}
    ~DFS_maze_generator(){}

    void operator()(vector<vector<int>>& maze) { dfs(maze, 0, 0); }

    enum { NOT_VISITED = 16};
    enum { L = 1, R = 2, U = 4, D = 8 };


private: 
    void dfs(vector<vector<int>>& maze, int r, int c) {
        maze[r][c] &= ~NOT_VISITED;
        vector<vector<int>> temp; temp.reserve(4);

        if (r - 1 >= 0               &&  (maze[r-1][c] & NOT_VISITED)) { temp.push_back({r - 1, c    }); }
        if (r + 1 <  maze.size()     &&  (maze[r+1][c] & NOT_VISITED)) { temp.push_back({r + 1, c    }); }
        if (c - 1 >= 0               &&  (maze[r][c-1] & NOT_VISITED)) { temp.push_back({r    , c - 1}); }
        if (c + 1 <  maze[0].size()  &&  (maze[r][c+1] & NOT_VISITED)) { temp.push_back({r    , c + 1}); }

        while (!temp.empty()) {
            int t = rand() % temp.size();
            int y = temp[t][0];
            int x = temp[t][1];
            temp.erase(temp.begin() + t);

            if ((maze[y][x] & NOT_VISITED) == 0) continue;
            
            if (y - r == 1) {
                maze[y][x] &= ~U;
                maze[r][c] &= ~D;
            }
            else if (r - y == 1) {
                maze[y][x] &= ~D;
                maze[r][c] &= ~U;
            }
            else if (x - c == 1) {
                maze[y][x] &= ~L;
                maze[r][c] &= ~R;
            }
            else if (c - x == 1) {
                maze[y][x] &= ~R;
                maze[r][c] &= ~L;
            }

            dfs(maze, y, x);
        }
    }
};


class DFS_maze_solver {
public: 
    DFS_maze_solver() { node_stack.push({0, 0}); }
    ~DFS_maze_solver(){}

    bool operator()(vector<vector<int>>& maze) { 
        int r = node_stack.top().first;
        int c = node_stack.top().second;
        maze[r][c] |= VISITED;
        maze[r][c] |= IN_PATH;

        if (r + 1 == maze.size()  &&  c + 1 == maze[0].size()) { 
            stack<pair<int, int>>({{0, 0}}).swap(node_stack);
            return true;
        }

        vector<pair<int, int>> temp = filter(maze, r, c);

        if (temp.empty()) {
            do {
                node_stack.pop();
                maze[r][c] &= ~IN_PATH;

                r           = node_stack.top().first;
                c           = node_stack.top().second;
                temp        = filter(maze, r, c);
            } while (temp.empty());

            return false;
        }

        int t = rand() % temp.size();
        node_stack.push(temp[t]);
        return false;
     }

    enum { VISITED = 32, IN_PATH = 64, };

private:
    stack<pair<int, int>> node_stack;

    vector<pair<int, int>> filter(vector<vector<int>>& maze, int r, int c) {
        vector<pair<int, int>> temp; temp.reserve(3);

        if (r - 1 >= 0               &&  (maze[r-1][c] & VISITED) == 0  &&  (maze[r][c] & DFS_maze_generator::U) == 0) { temp.push_back({r - 1, c    }); }
        if (r + 1 <  maze.size()     &&  (maze[r+1][c] & VISITED) == 0  &&  (maze[r][c] & DFS_maze_generator::D) == 0) { temp.push_back({r + 1, c    }); }
        if (c - 1 >= 0               &&  (maze[r][c-1] & VISITED) == 0  &&  (maze[r][c] & DFS_maze_generator::L) == 0) { temp.push_back({r    , c - 1}); }
        if (c + 1 <  maze[0].size()  &&  (maze[r][c+1] & VISITED) == 0  &&  (maze[r][c] & DFS_maze_generator::R) == 0) { temp.push_back({r    , c + 1}); }

        return temp;
    }
};


class Maze {
public:
    Maze() 
    : dfs_maze_generator(),
      maze(maze_height, vector<int>(maze_width, DFS_maze_generator::L|
                                                DFS_maze_generator::R|
                                                DFS_maze_generator::U|
                                                DFS_maze_generator::D|
                                                DFS_maze_generator::NOT_VISITED)) { init(); }


    void main_loop() {
        while (true) {
            dfs_maze_generator(maze);
            draw_maze();
            while (! dfs_maze_solver(maze)) { draw_maze(); this_thread::sleep_for(200ms); }
            draw_maze();
            this_thread::sleep_for(2s);
            vector<vector<int>>(maze_height, vector<int>(maze_width, DFS_maze_generator::L|
                                                                     DFS_maze_generator::R|
                                                                     DFS_maze_generator::U|
                                                                     DFS_maze_generator::D|
                                                                     DFS_maze_generator::NOT_VISITED)).swap(maze);
        }
    }


private: 
    int                   maze_width            = 32;
    int                   maze_height           = 12;
    int                   lattice_width         =  5;  // '-'
    int                   lattice_height        =  1;  // '|'
    vector<vector<int>>   maze;
    DFS_maze_generator    dfs_maze_generator;
    DFS_maze_solver       dfs_maze_solver;

    void init() {
        initscr();
        cbreak();
        noecho();
        curs_set(0);

        srand(time(0));
    }

    void generate_maze() {
        dfs_maze_generator(maze);
    }

    /* void solve_maze() {
        dfs_maze_solver(maze);
    } */

    void draw_maze() {
        clear();

        for (int y=0; y<maze_height; y++) {
            for (int x=0; x<maze_width; x++) {
                draw_lattice(y, x);
            } }
    }

    void draw_lattice(int y, int x) {
        int Y = y * (lattice_height + 1);
        int X = x * (lattice_width - 1);

        move(Y                     , X);                     addch('+');    // +   +
        move(Y                     , X + lattice_width - 1); addch('+');    //
        move(Y + lattice_height + 1, X);                     addch('+');    // +   +
        move(Y + lattice_height + 1, X + lattice_width - 1); addch('+');

        if (maze[y][x] & DFS_maze_solver::IN_PATH) {                        // +   +
            move(Y + 1, X + 2);                                             //   * 
            addch('*');                                                     // +   +
        }

        if (maze[y][x] & DFS_maze_generator::U) {                           // +---+
            for (int i=1; i<lattice_width-1; i++) {                         // 
                move(Y, X + i);                                             // +   +
                addch('-');
            } }

        if (maze[y][x] & DFS_maze_generator::D) {                           // +   +
            for (int i=1; i<lattice_width-1; i++) {                         //
                move(Y + lattice_height + 1, X + i);                        // +---+
                addch('-');
        } }

        if (maze[y][x] & DFS_maze_generator::L) {                           // +   +
            for (int i=1; i<=lattice_height; i++) {                         // |   
                move(Y + i, X);                                             // +   +
                addch('|');
        } }

        if (maze[y][x] & DFS_maze_generator::R) {                           // +   +
            for (int i=1; i<=lattice_height; i++) {                         //     |
                move(Y + i, X + lattice_width - 1);                         // +   +
                addch('|');
        } }

        refresh();
    }
};

int main(int argc, char** argv) 
{
    if (argc == 1) {
        Maze m;
        m.main_loop();
    }
    else {
        printf("there should be no argument \n");
    }
 

    return 0;
}