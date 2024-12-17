#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

// Configuración Inicial
const int WIDTH = 10, HEIGHT = 20;

// Enumeración para las piezas tetromino
enum Tetronimo {I, J, L, O, S, T, Z};

struct Piece {
   vector<vector<int>> shape;
   int x, y;

   Piece(Tetromino type) : x(WIDTH / 2), y(0) {}
};

// Variables globales
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));

// Funciones
void displayBoard();

int main() {

   for (int i = 0; i < 10; i++) {
      fill(board[i].begin(), board[i].end(), 0);
   }
   cout << "¡Bienvenido a Tetris!" << endl;
   displayBoard();

   return 0;
}

void displayBoard() {
   for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
         cout << board[i][j] << "..";
      }
   }
}