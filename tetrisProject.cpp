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

bool gameCancelled = false; // Variable global para manejar la cancelación

// Funciones
void displayTitleScreen();
void resetGame();
void signalHandler(int signum);
void renderBoard();
Piece* createRandomPiece();

int main() {
   signal(SIGINT, signalHandler);  // Registra el manejador de señales
   displayTitleScreen();
   cin.get(); // Esperar Enter
   resetGame();
   cout << "Juego iniciado.\n";
   while (!gameCancelled) {
      renderBoard();
      Piece* currentPiece = createRandomPiece();
      delete currentPiece; // Liberar memoria para la pieza
   }
   cout << "Juego terminado.\n";
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
   cout << "Juego cancelado con señal " << signum << ".\n";
   gameCancelled = true;  // Detener el juego cuando se reciba una señal
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
