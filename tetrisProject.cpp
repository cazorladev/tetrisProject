#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <algorithm>
#include <queue>
#include <csignal>

// Libreria para multiplataformas
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

const int WIDTH = 10, HEIGHT = 20;
const int PUNTOS_POR_LINEA = 100;
const int NIVEL_INCREMENTO = 5;
const int VELOCIDAD_MINIMA = 100;

enum Tetromino {I, J, L, O, S, T, Z};

const vector<vector<vector<int>>> TETROMINO_SHAPES = {
   {{1, 1, 1, 1}},            // I
   {{1, 1}, {1, 1}},          // O
   {{0, 1, 0}, {1, 1, 1}},    // T
   {{1, 0, 0}, {1, 1, 1}},    // L
   {{0, 0, 1}, {1, 1, 1}},    // J
   {{0, 1, 1}, {1, 1, 0}},    // S
   {{1, 1, 0}, {0, 1, 1}}     // Z
};

// Estructura de datos para representar una pieza
struct Piece {
   vector<vector<int>> shape;
   int x, y;

    Piece(Tetromino type) : shape(TETROMINO_SHAPES[type]), x(WIDTH / 2 - shape[0].size() / 2), y(0) {}
    Piece(const std::vector<std::vector<int>> &newShape, int startX, int startY) : shape(newShape), x(startX), y(startY) {}
};

// Declaración de variables globales
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));
int score = 0, linesCleared = 0, level = 1, speed = 500;
queue<Piece*> upcomingPieces;
bool gameCancelled = false; // Variable global para manejar la cancelación

// === Declaración de Funciones ===
// estructura-base
void displayTitleScreen();
void resetGame();
void clearConsole();

// Renderización
void renderGame(const Piece *activePiece, const Piece *nextPiece, bool isPaused);

// Game-mechanics
Piece* createRandomPiece();
bool canPlacePiece(const Piece *piece, int dx, int dy);
void placePiece(const Piece *piece);                                               // Colocar una pieza en el tablero
void clearFullLines();
void rotatePiece(Piece *piece);

// User-input
char getKeyPress();
void handleInput(Piece *activePiece, bool *nextPiece);

// Función principal del juego
void gameLoop();

// Funciones de finalización del juego
void freePieceMemory(Piece *activePiece, Piece *nextPiece);
void displayGameOver();
void signalHandler(int signum);



void renderBoard();




int main() {
   srand(static_cast<unsigned int>(time(0)));
   setlocale(LC_ALL, "es_ES.UTF-8");
   signal(SIGINT, signalHandler);
   displayTitleScreen();
   getKeyPress();
   clearConsole();


   resetGame();

   cout << "Juego terminado.\n";
   return 0;
}

void displayTitleScreen() {
   clearConsole();
   cout  << "=====================================\n"
         << "=         BIENVENIDO A TETRIS       =\n"
         << "=====================================\n\n"
         << "          CONTROLES:\n"
         << "          W: Rotar\n"
         << "          A: Mover a la izquierda\n"
         << "          D: Mover a la derecha\n"
         << "          S: Caída rápida\n"
         << "          ESPACIO: Caída instantánea\n"
         << "          P: Pausar / Reanudar\n\n"
         << "Presiona Enter para comenzar...\n";
}

void resetGame() {
   board.assign(HEIGHT, vector<int>(WIDTH, 0));
   score = linesCleared = 0;
   level = 1;
   speed = 500;
   while (!upcomingPieces.empty()) {
      upcomingPieces.pop();
   }
    upcomingPieces.push(createRandomPiece());
   clearConsole();
}

void clearConsole() {
   cout << "\033[2J\033[H";
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

bool canPlacePiece(Piece* piece, int dx, int dy) {
   for (int i = 0; i < piece->shape[i].size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         if (piece->shape[i][j]) {
            int newX = piece->x + j + dx;
            int newY = piece->y + i + dy;
            if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT || board[newY][newX]) {
               return false;
            }
         }
      }
   }
   return true;   
}

void gameLoop() {
   Piece* activePiece = createRandomPiece();
   bool gameOver = false;
   while (!gameOver) {
      renderBoard();
      if (canPlacePiece(activePiece, 0, 1)) {
         activePiece->y++;
         } else {
            gameOver = true;
      }
      this_thread::sleep_for(chrono::milliseconds(500));
   }
   cout << "Juego terminado.\n";
}

char getKeyPress() {
#ifdef _WIN32
   return _getch();
#else
   struct termios oldt, newt;
   char ch;
   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   ch = getchar();
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
   return ch;
#endif
}

void handleInput(Piece *activePiece, bool &isPaused) {                  // Procesa los comandos del jugador
   if (_kbhit()) {
      char key = getKeyPress();
      if (key == 'p' || key == 'P') {
         isPaused = !isPaused;
      } else if (!isPaused) {
         if (key == 'a' && canPlacePiece(activePiece, -1, 0)) {         // Mover la pieza a la izquierda
            activePiece->x--;
         } else if (key == 'd' && canPlacePiece(activePiece, 1, 0)) {   // Mover la pieza a la derecha 
            activePiece->x++;
         } else if (key == 's' && canPlacePiece(activePiece, 0, 1)) {   // Bajar la pieza
            activePiece->y++;
         } else if (key == 'w') {                                       // Rotar la pieza
            rotatePiece(activePiece);
         } else if (key == ' ') {                                       // Colocar la pieza
            while (canPlacePiece(activePiece, 0, 1)) {
               activePiece->y++;
            }
         }
      }
   }
}

void rotatePiece(Piece *piece) {
   vector<vector<int>> rotated(piece->shape[0].size(), vector<int>(piece->shape.size()));
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         rotated[j][piece->shape.size() - i - 1] = piece->shape[i][j];
      }
   }
   if (canPlacePiece(new Piece(rotated, piece->x, piece->y), 0, 0)) {
      piece->shape = rotated;
   } else {
      piece->x = canPlacePiece(new Piece(rotated, piece->x - 1, piece->y), 0, 0) ? piece->x - 1 : canPlacePiece(new Piece(rotated, piece->x + 1, piece->y), 0, 0) ? piece->x + 1
                                                                                                                                                                  : piece->x;
      piece->shape = canPlacePiece(new Piece(rotated, piece->x, piece->y), 0, 0) ? rotated : piece->shape;
   }
}

Piece *createRandomPiece() {              // Genera una nueva pieza aleatoria
   return new Piece(static_cast<Tetromino>(rand() % TETROMINO_SHAPES.size()));
}

bool canPlacePiece(const Piece *piece, int dx, int dy) {             // Verifica si se puede colocar la pieza en la posición deseada
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         if (piece->shape[i][j]) {
            int newX = piece->x + j + dx, newY = piece->y + i + dy;
            if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT || board[newY][newX]) {
               return false;
            }
         }
      }
   }
   return true;
}

void placePiece(const Piece *piece) {              // Coloca la pieza en el tablero
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         board[piece->y + i][piece->x + j] = piece->shape[i][j] ? 1 : board[piece->y + i][piece->x + j];
      }
   }
}

void clearFullLines() {             // Limpia las líneas completas del tablero
   for (int i = 0; i < HEIGHT; ++i) {
      if (all_of(board[i].begin(), board[i].end(), [](int cell) { return cell == 1; })) {
         board.erase(board.begin() + i);                          // Elimina la línea completa
         board.insert(board.begin(), vector<int>(WIDTH, 0));      // Agrega una nueva línea vacía
         score += PUNTOS_POR_LINEA;                               // Usar constante para puntos
         linesCleared++;
         level = linesCleared % NIVEL_INCREMENTO == 0 ? level + 1 : level;                         // Usar constante para nivel
         speed = linesCleared % NIVEL_INCREMENTO == 0 ? max(VELOCIDAD_MINIMA, speed - 25) : speed; // Usar constante para velocidad
      }
   }
}

void signalHandler(int signum) {
   cout << "Juego cancelado con señal " << signum << ".\n";
   gameCancelled = true;  // Detener el juego cuando se reciba una señal
}