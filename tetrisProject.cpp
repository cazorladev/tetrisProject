#include <iostream>     // Manejo de entrada/salida estándar
#include <vector>       // Uso de vectores para las piezas y el tablero
#include <random>       // Uso de números aleatorios para la posición inicial de las piezas
#include <thread>       // Manejo de hilos para pausas y temporización
#include <chrono>       // Gestión precisa de tiempo
#include <algorithm>    // Funciones de utilidad como all_of() y fill()
#include <queue>        // Cola para manejar las piezas próximas
#include <csignal>      // Manejo de señales del sistema (como SIGINT)
#include <locale.h>

// Libreria para multiplataformas
#ifdef _WIN32 // Windows
#include <conio.h> // Captura de teclas en Windows
#include <windows.h> // Captura de teclas en Windows
#else
#include <termios.h> // Captura de teclas en Linux/Unix
#include <unistd.h> // Funciones del sistema en Linux/Unix
#endif

using namespace std;

const int WIDTH = 10, HEIGHT = 20;
const int PUNTOS_POR_LINEA = 100;
const int NIVEL_INCREMENTO = 5;
const int VELOCIDAD_MINIMA = 100;

enum Tetromino {I, J, L, O, S, T, Z};

const vector<vector<vector<int>>> TETROMINO_SHAPES = {
   // Definición de las formas de las piezas Tetromino
   {{1, 1, 1, 1}},         // I
   {{1, 1}, {1, 1}},       // O
   {{0, 1, 0}, {1, 1, 1}}, // T
   {{1, 0, 0}, {1, 1, 1}}, // L
   {{0, 0, 1}, {1, 1, 1}}, // J
   {{0, 1, 1}, {1, 1, 0}}, // S
   {{1, 1, 0}, {0, 1, 1}}  // Z
};

// Estructura de datos para representar una pieza
struct Piece {
   vector<vector<int>> shape;
   int x, y;

   Piece(Tetromino type) : shape(TETROMINO_SHAPES[type]), x(WIDTH / 2 - shape[0].size() / 2), y(0) {}
   Piece(const vector<vector<int>> &newShape, int startX, int startY) : shape(newShape), x(startX), y(startY) {}
};

// Declaración de variables globales
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));
int score = 0, linesCleared = 0, level = 1, speed = 500;
queue<Piece *> upcomingPieces;
bool gameCancelled = false;   // Variable global para manejar la cancelación

// === Declaración de Funciones ===
// estructura-base
void displayTitleScreen();
void resetGame();
void clearConsole();

// Renderización
void renderGame(const Piece *activePiece, const Piece *nextPiece, bool isPaused);

// Game-mechanics
Piece *createRandomPiece(); 
bool canPlacePiece(const Piece *piece, int dx, int dy);
void placePiece(const Piece *piece);                                               // Colocar una pieza en el tablero
void clearFullLines();  
void rotatePiece(Piece *piece); 

// User-input
char getKeyPress();
void handleInput(Piece *activePiece, bool &isPaused); 

// Función principal del juego
void gameLoop();

// Funciones de finalización del juego
void freePieceMemory(Piece *activePiece, Piece *nextPiece);
void displayGameOver();
void signalHandler(int signum);

int main() {
   setlocale(LC_ALL, "es_ES.UTF-8");            // Establecer el idioma de la consola
   signal(SIGINT, signalHandler);               // Manejar interrupciones con Ctrl + C            
   displayTitleScreen();                        // Mostrar pantalla de bienvenida
   getKeyPress();                               // Esperar una tecla para iniciar
   clearConsole();                              // Limpiar la consola
   resetGame();                                 // Reiniciar variables del juego
   gameLoop();                                  // Iniciar el ciclo principal del juego
   cout << "\nGracias por jugar Tetris!\n";
   return 0;
}

void displayTitleScreen() {               // Muestra la pantalla de bienvenida.
   clearConsole();
   cout  << "\n=====================================\n"
         << "=                                   =\n"
         << "=         BIENVENIDO A TETRIS       =\n"
         << "=                                   =\n"
         << "=====================================\n\n"
         << "          CONTROLES:\n"
         << "          W: Rotar\n"
         << "          A: Mover a la izquierda\n"
         << "          D: Mover a la derecha\n"
         << "          S: Caída rápida\n"
         << "          ESPACIO: Caída instantánea\n"
         << "          P: Pausar/Reanudar\n\n"
         << "Presiona Enter para comenzar...\n";
}

void resetGame() {               // Reinicia el estado del tablero y las estadísticas
   board.assign(HEIGHT, vector<int>(WIDTH, 0));    
   score = linesCleared = 0;
   level = 1;
   speed = 500;                                    // Velocidad inicial del juego
   while (!upcomingPieces.empty()) {
      upcomingPieces.pop();
   }
   upcomingPieces.push(createRandomPiece());
   clearConsole();
}

void clearConsole() {            // Limpia la pantalla
   cout << "\033[2J\033[H";
}

void renderGame(const Piece *activePiece, const Piece *nextPiece, bool isPaused) {              // Renderiza el tablero y las piezas
   static vector<string> previousScreen;
   vector<string> currentScreen(HEIGHT + 10);

   for (int i = 0; i < HEIGHT; ++i) {                 // Construcción visual del tablero
      string row = "<|";
      for (int j = 0; j < WIDTH; ++j) {
         bool isPiece = false;
         for (int pi = 0; pi < activePiece->shape.size(); ++pi) {
            for (int pj = 0; pj < activePiece->shape[pi].size(); ++pj) {
               if (activePiece->shape[pi][pj] && activePiece->y + pi == i && activePiece->x + pj == j) {
                  isPiece = true;
                  break;
               }
            } if (isPiece) {
               break;
            }
         }
         row += (isPiece || board[i][j]) ? "[]" : ".."; // Representación visual de las piezas y el tablero
      }

      row += "|>";
      row += i == 1 ? "   Puntaje: " + to_string(score) : "";
      row += i == 2 ? "   Líneas eliminadas: " + to_string(linesCleared) : "";
      row += i == 3 ? "   Nivel: " + to_string(level) : "";
      row += i == 5 ? "   Próxima pieza:" : "";
      if (i >= 6 && i < 6 + 4) {
         row += "   ";
         int offsetRow = i - 6;
         for (int pj = 0; pj < 4; ++pj) {
            row += (offsetRow < nextPiece->shape.size() && pj < nextPiece->shape[offsetRow].size() && nextPiece->shape[offsetRow][pj]) ? "[]" : "  ";
         }
      }
      row += i == 11 ? "   Controles:" : "";
      if (i >= 12 && i <= 17) {
         const char *controls[] = {"     W: Rotar", "     A: Mover a la izquierda", "     D: Mover a la derecha", "     S: Caída rápida", "     ESPACIO: Caída instantánea", "     P: Pausar/Reanudar"};
         row += controls[i - 12];
      }
      currentScreen[i] = row;
   }
   currentScreen[HEIGHT] = "<|" + string(WIDTH * 2, '=') + "|>";
   if (isPaused) {
      currentScreen[HEIGHT + 2] = " |====================|";
      currentScreen[HEIGHT + 3] = " |  JUEGO EN PAUSA    |";
      currentScreen[HEIGHT + 4] = " |====================|";
   } else {
      fill(currentScreen.begin() + HEIGHT + 2, currentScreen.begin() + HEIGHT + 5, string(WIDTH * 2 + 3, ' '));
   }
   for (int i = 0; i < currentScreen.size(); ++i) {
      if (previousScreen.empty() || i >= previousScreen.size() || currentScreen[i] != previousScreen[i]) {
         cout << "\033[" << i + 1 << ";1H" << currentScreen[i];
      }
   }
   previousScreen = currentScreen;
}

Piece *createRandomPiece() {
    static std::random_device rd;           // Fuente de entropía
    static std::mt19937 gen(rd());          // Generador basado en Mersenne Twister
    static std::uniform_int_distribution<> distrib(0, TETROMINO_SHAPES.size() - 1); // Distribución uniforme

    // Generar un índice aleatorio y crear la pieza
    return new Piece(static_cast<Tetromino>(distrib(gen)));
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

void rotatePiece(Piece *piece) {                // Rota la pieza activa
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

char getKeyPress() {          // Obtiene la tecla presionada por el usuario
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
      int key = getch();;
      if (key == 0 || key == 224) {
         int arrowKey = getch();
         if (!isPaused) {
            if (arrowKey == 75 && canPlacePiece(activePiece, -1, 0)) {  // Flecha izquierda
            activePiece->x--;
            } else if (arrowKey == 77 && canPlacePiece(activePiece, 1, 0)) {  // Flecha derecha 
            activePiece->x++;
            } else if (arrowKey == 80 && canPlacePiece(activePiece, 0, 1)) {  // Flecha abajo
            activePiece->y++;
            } else if (arrowKey == 72) {  // Flecha arriba (rotar la pieza)                                       // Rotar la pieza
            rotatePiece(activePiece);
            }
         }
      } else if (key == 'p' || key == 'P') {  // Pausar/reanudar
         isPaused = !isPaused;
         } else if (!isPaused) {
            if (key == 'a' && canPlacePiece(activePiece, -1, 0)) {  // 'a' para mover izquierda   
               activePiece->x--;
         } else if (key == 'd' && canPlacePiece(activePiece, 1, 0)) {  // 'd' para mover derecha
            activePiece->x++;
         } else if (key == 's' && canPlacePiece(activePiece, 0, 1)) {  // 's' para bajar
            activePiece->y++;
         } else if (key == 'w') {  // 'w' para rotar
            rotatePiece(activePiece);
         } else if (key == ' ') {  // Barra espaciadora para colocar la pieza
            while (canPlacePiece(activePiece, 0, 1)) {
               activePiece->y++;
            }
         }
      }
   }
}

void gameLoop() {                // Bucle principal del juego
   cout << "\033[?25l";
   Piece *activePiece = createRandomPiece();
   upcomingPieces.push(createRandomPiece());
   Piece *nextPiece = upcomingPieces.front();
   upcomingPieces.pop();
   bool gameOver = false, isPaused = false;
   auto lastDropTime = chrono::steady_clock::now();

   while (!gameOver) {
      renderGame(activePiece, nextPiece, isPaused);

      handleInput(activePiece, isPaused);                // Llama a la función handleInput para manejar la entrada del usuario

      if (!isPaused && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - lastDropTime).count() >= speed) {
         if (canPlacePiece(activePiece, 0, 1)) {
            activePiece->y++;
         } else {
            placePiece(activePiece);
            clearFullLines();
            delete activePiece;
            activePiece = nextPiece;
            if (upcomingPieces.empty()) {
               upcomingPieces.push(createRandomPiece());
            }
            nextPiece = upcomingPieces.front();
            upcomingPieces.pop();
            if (!canPlacePiece(activePiece, 0, 0)) {
               gameOver = true;
            }
         }
         lastDropTime = chrono::steady_clock::now();
      }

      if (gameCancelled) {
         gameOver = true;
      }

      this_thread::sleep_for(chrono::milliseconds(10));
   }

   displayGameOver();
   freePieceMemory(activePiece, nextPiece);
}

void freePieceMemory(Piece *activePiece, Piece *nextPiece) {               // Libera la memoria de las piezas activas
   delete activePiece;
   delete nextPiece;
}

void displayGameOver() {               // Muestra el mensaje de Game Over
   cout << "\033[2J\033[H"
        << "|====================|\n"
        << "|    FIN DEL JUEGO   |\n"
        << "|====================|\n\n"
        << "Puntaje final: " << score << "\n";
}

void signalHandler(int signum) {             // Maneja señales como SIGINT
   gameCancelled = true;
}

//bkjbkb