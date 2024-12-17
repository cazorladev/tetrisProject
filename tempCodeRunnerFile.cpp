#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <csignal>

using namespace std;

const int WIDTH = 10, HEIGHT = 20;
enum Tetromino {I, J, L, O, S, T, Z};

const vector<vector<vector<int>>> TETROMINO_SHAPES = {
   {{1, 1, 1, 1}},
   {{1, 1}, {1, 1}},
   {{0, 1, 0}, {1, 1, 1}},
   {{1, 0, 0}, {1, 1, 1}},
   {{0, 0, 1}, {1, 1, 1}},
   {{0, 1, 1}, {1, 1, 0}},
   {{1, 1, 0}, {0, 1, 1}}
};

struct Piece {
   vector<vector<int>> shape;
   int x, y;

   Piece(Tetromino type)
      : shape(TETROMINO_SHAPES[type]), x(WIDTH / 2 - shape[0].size() / 2), y(0) {}
};

vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));

// Funciones
void displayTitleScreen();
void resetGame();
void signalHandler();
void renderBoard();
Piece* createRandomPiece();

int main() {
   signal(SIGINT, signalHandler);
   displayTitleScreen();
   cin.get(); // Esperar Enter
   resetGame();
   cout << "Juego iniciado.\n";
   return 0;
}

void displayTitleScreen() {
   cout  << "=====================================\n"
         << "=         BIENVENIDO A TETRIS       =\n"
         << "=====================================\n\n"
         << "Presiona Enter para comenzar...\n";
}

void resetGame() {
   board.assign(HEIGHT, vector<int>(WIDTH, 0));
   cout << "Juego reiniciado.\n";
}

void signalHandler(int signum) {
   bool gameCancelled = false;
}

void renderBoard() {
   for (int i = 0; i < HEIGHT; ++i) {
      for (int j = 0; j < WIDTH; ++j) {
         cout << (board[i][j] ? "[]" : "..");
      }
        cout << endl;
   }
}

Piece* createRandomPiece() {
   return new Piece(static_cast<Tetromino>(rand() % TETROMINO_SHAPES.size()));
}