#include <iostream>     // Manejo de entrada/salida estándar
#include <vector>       // Uso de vectores para las piezas y el tablero
#include <random>       // Uso de números aleatorios para la posición inicial de las piezas
#include <thread>       // Manejo de hilos para pausas y temporización
#include <chrono>       // Gestión precisa de tiempo
#include <algorithm>    // Funciones de utilidad como all_of() y fill()
#include <queue>        // Cola para manejar las piezas próximas
#include <csignal>      // Manejo de señales para terminar el programa
#include <locale.h>     // configura la localización de la aplicación para trabajar con un idioma y formato específicos

// Libreria para multiplataformas
#ifdef _WIN32 // Directiva de preprocesador para Windows
#include <conio.h> // Captura de teclas en Windows
#include <windows.h> // Captura de teclas en Windows
#else
#include <termios.h> // Captura de teclas en Linux/Unix
#include <unistd.h> // Funciones del sistema en Linux/Unix
#endif

using namespace std;

// Dimensiones del tablero y parámetros del juego
const int WIDTH = 10, HEIGHT = 20;     // Dimensiones del tablero
const int PUNTOS_POR_LINEA = 100;      // Puntos por línea eliminada
const int NIVEL_INCREMENTO = 5;        // Incremento de nivel
const int VELOCIDAD_MINIMA = 100;      // Velocidad minima 

// Enumeración para los tipos de Tetrominos
enum Tetromino {I, J, L, O, S, T, Z};

// Definición de las formas de los Tetrominos
const vector<vector<vector<int>>> TETROMINO_SHAPES = {
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
   vector<vector<int>> shape; // Forma de la pieza
   int x, y;                  // Posición de la pieza en el tablero

   // Constructor para crear una pieza aleatoria
   Piece(Tetromino type) : shape(TETROMINO_SHAPES[type]), x(WIDTH / 2 - shape[0].size() / 2), y(0) {}
   // Constructor para crear una pieza con forma y posición específicas
   Piece(const vector<vector<int>> &newShape, int startX, int startY) : shape(newShape), x(startX), y(startY) {}
};

// Declaración de variables globales
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));         // Tablero del juego      
int score = 0, linesCleared = 0, level = 1, speed = 500;          // Estadísticas del juego
queue<Piece *> upcomingPieces;                                    // Cola para manejar las piezas próximas
bool gameCancelled = false;                                       // Variable global para manejar la cancelación del juego

// === Declaración de Funciones ===
// Funciones de visualización y manejo del juego
void displayTitleScreen();       // Muestra la pantalla de bienvenida
void resetGame();                // Reinicia el estado del juego
void clearConsole();             // Limpia la consola

// Funciones de renderización
void renderGame(const Piece *activePiece, const Piece *nextPiece, bool isPaused);   // Renderiza el estado del juego

// Funciones de mecánica del juego
Piece *createRandomPiece();                                 // Crea una pieza Tetromino aleatoria
bool canPlacePiece(const Piece *piece, int dx, int dy);     // Verifica si se puede colocar la pieza
void placePiece(const Piece *piece);                        // Coloca la pieza en el tablero
void clearFullLines();                                      // Limpia las líneas completas del tablero     
void rotatePiece(Piece *piece);                             // Rota la pieza activa 

// Funciones de entrada del usuario
char getKeyPress();                                      // Obtiene la tecla presionada por el usuario
void handleInput(Piece *activePiece, bool &isPaused);    // Maneja la entrada del usuario        

// Función principal del juego
void gameLoop();                                         // Inicia el ciclo principal del juego

// Funciones de finalización del juego
void freePieceMemory(Piece *activePiece, Piece *nextPiece);       // Libera la memoria de las piezas
void displayGameOver();                                           // Muestra el mensaje de Game Over
void signalHandler(int signum);                                   // Maneja señales del sistema

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

void displayTitleScreen() {   // Muestra la pantalla de bienvenida.
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
         << "Presiona Enter para comenzar...\n";   // Instrucciones para el jugador
}

void resetGame() {   // Reinicia el estado del tablero y las estadísticas
   board.assign(HEIGHT, vector<int>(WIDTH, 0));    // Reinicia el tablero    
   score = linesCleared = 0;                       // Reinicia el puntaje y líneas eliminadas     
   level = 1;                                      // Reinicia el nivel
   speed = 500;                                    // Velocidad inicial del juego
   while (!upcomingPieces.empty()) {
      upcomingPieces.pop();                        // Limpia la cola de piezas próximas
   }
   upcomingPieces.push(createRandomPiece());       // Agrega una pieza aleatoria a la cola
   clearConsole();
}

void clearConsole() {   // Limpia la pantalla
   #ifdef _WIN32
      system("cls");  // Limpia la pantalla en Windows
   #else
      cout << "\033[2J\033[H";  // Limpia la pantalla en Unix/Linux
   #endif
}

void renderGame(const Piece *activePiece, const Piece *nextPiece, bool isPaused) {  // Renderiza el tablero y las piezas
   static vector<string> previousScreen;     // Almacena la pantalla anterior para optimizar la renderización
   vector<string> currentScreen(HEIGHT + 10);   // Pantalla actual

   for (int i = 0; i < HEIGHT; ++i) {  // Construcción visual del tablero (filas)
      string row = "<|";
      for (int j = 0; j < WIDTH; ++j) { // Construcción visual del tablero (columnas)
         bool isPiece = false;      // Bandera para verificar si hay una pieza en la posición
         for (int pi = 0; pi < activePiece->shape.size(); ++pi) {
            for (int pj = 0; pj < activePiece->shape[pi].size(); ++pj) {
               if (activePiece->shape[pi][pj] && activePiece->y + pi == i && activePiece->x + pj == j) {
                  isPiece = true;   // Se encontró una pieza en la posición
                  break;
               }
            } if (isPiece) {
               break;
            }
         }
         row += (isPiece || board[i][j]) ? "[]" : ".."; // Representación visual de las piezas y el tablero
      }

      row += "|>";
      row += i == 1 ? "   Puntaje: " + to_string(score) : "";                    // Muestra el puntaje
      row += i == 2 ? "   Líneas eliminadas: " + to_string(linesCleared) : "";   // Muestra líneas eliminadas
      row += i == 3 ? "   Nivel: " + to_string(level) : "";                      // Muestra el nivel
      row += i == 5 ? "   Próxima pieza:" : "";                                  // Indica la próxima pieza
      if (i >= 6 && i < 6 + 4) {    // Renderiza la próxima pieza
         row += "   ";
         int offsetRow = i - 6;
         for (int pj = 0; pj < 4; ++pj) {
            row += (offsetRow < nextPiece->shape.size() && pj < nextPiece->shape[offsetRow].size() && nextPiece->shape[offsetRow][pj]) ? "[]" : "  "; // Representación visual de la próxima pieza
         }
      }
      row += i == 11 ? "   Controles:" : "";
      if (i >= 12 && i <= 17) {     // Muestra los controles
         const char *controls[] = {"     W: Rotar", "     A: Mover a la izquierda", "     D: Mover a la derecha", "     S: Caída rápida", "     ESPACIO: Caída instantánea", "     P: Pausar/Reanudar"};
         row += controls[i - 12];
      }
      currentScreen[i] = row;    // Almacena la fila en la pantalla actual
   }
   currentScreen[HEIGHT] = "<|" + string(WIDTH * 2, '=') + "|>";
   if (isPaused) {      // Si el juego está en pausa
      currentScreen[HEIGHT + 2] = " |====================|";
      currentScreen[HEIGHT + 3] = " |  JUEGO EN PAUSA    |";
      currentScreen[HEIGHT + 4] = " |====================|";
   } else {
      fill(currentScreen.begin() + HEIGHT + 2, currentScreen.begin() + HEIGHT + 5, string(WIDTH * 2 + 3, ' '));   // Limpia el área de pausa
   }
   for (int i = 0; i < currentScreen.size(); ++i) {   // Renderiza la pantalla actual
      if (previousScreen.empty() || i >= previousScreen.size() || currentScreen[i] != previousScreen[i]) {
         cout << "\033[" << i + 1 << ";1H" << currentScreen[i];   // Mueve el cursor y muestra la fila
      }
   }
   previousScreen = currentScreen;  // Actualiza la pantalla anterior
}

Piece *createRandomPiece() {  // Crea una pieza Tetromino aleatoria
   static std::random_device rd;
   static std::mt19937 gen(rd());          // Generador basado en Mersenne Twister
   static std::uniform_int_distribution<> distrib(0, TETROMINO_SHAPES.size() - 1); // Distribución uniforme

   return new Piece(static_cast<Tetromino>(distrib(gen)));  // Generar un índice aleatorio y crear la pieza
}

bool canPlacePiece(const Piece *piece, int dx, int dy) {    // Verifica si se puede colocar la pieza en la posición deseada
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         if (piece->shape[i][j]) {
            int newX = piece->x + j + dx, newY = piece->y + i + dy;     // Calcula la nueva posición
            if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT || board[newY][newX]) {
               return false;     // Retorna falso si la posición es inválida
            }
         }
      }
   }
   return true;   // Retorna verdadero si la posición es válida
}

void placePiece(const Piece *piece) {     // Coloca la pieza en el tablero
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         board[piece->y + i][piece->x + j] = piece->shape[i][j] ? 1 : board[piece->y + i][piece->x + j]; // Actualiza el tablero
      }
   }
}

void clearFullLines() {    // Limpia las líneas completas del tablero
   for (int i = 0; i < HEIGHT; ++i) {
      if (all_of(board[i].begin(), board[i].end(), [](int cell) { return cell == 1; })) {    // Verifica si la línea está completa
         board.erase(board.begin() + i);                                                     // Elimina la línea completa
         board.insert(board.begin(), vector<int>(WIDTH, 0));                                 // Agrega una nueva línea vacía
         score += PUNTOS_POR_LINEA;                                                          // Incrementa el puntaje
         linesCleared++;
         level = linesCleared % NIVEL_INCREMENTO == 0 ? level + 1 : level;                         // Incrementa el nivel si se han eliminado suficientes líneas
         speed = linesCleared % NIVEL_INCREMENTO == 0 ? max(VELOCIDAD_MINIMA, speed - 25) : speed; // Aumenta la velocidad del juego
      }
   }
}

void rotatePiece(Piece *piece) {    // Rota la pieza activa
   vector<vector<int>> rotated(piece->shape[0].size(), vector<int>(piece->shape.size()));    // Crea una nueva matriz para la pieza rotada
   for (int i = 0; i < piece->shape.size(); ++i) {
      for (int j = 0; j < piece->shape[i].size(); ++j) {
         rotated[j][piece->shape.size() - i - 1] = piece->shape[i][j];     // Rota la pieza 90 grados
      }
   }
   // Verifica si la pieza rotada puede ser colocada en la posición actual
   if (canPlacePiece(new Piece(rotated, piece->x, piece->y), 0, 0)) {
      piece->shape = rotated;    // Actualiza la forma de la pieza
   } else {
      // Intenta ajustar la posición de la pieza rotada si no cabe
      piece->x = canPlacePiece(new Piece(rotated, piece->x - 1, piece->y), 0, 0) ? piece->x - 1 : canPlacePiece(new Piece(rotated, piece->x + 1, piece->y), 0, 0) ? piece->x + 1
                                                                                                                                                                  : piece->x;
      piece->shape = canPlacePiece(new Piece(rotated, piece->x, piece->y), 0, 0) ? rotated : piece->shape;     // Mantiene la forma original si no se puede rotar
   }
}

char getKeyPress() {    // Obtiene la tecla presionada por el usuario
#ifdef _WIN32
   return _getch();     // Captura la tecla en Windows
#else
   struct termios oldt, newt;    // Configuración para capturar teclas en Linux/Unix
   char ch;
   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.lflag &= ~(ICANON | ECHO);     // Desactiva el modo canónico y el eco
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   ch = getchar();      // Captura la tecla
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);     // Restaura la configuración anterior
   return ch;
#endif
}

void handleInput(Piece *activePiece, bool &isPaused) {      // Procesa los comandos del jugador
   if (_kbhit()) {                                          // Verifica si hay una tecla presionada
      int key = getch();;                                   // Obtiene la tecla presionada
      if (key == 0 || key == 224) {                         // Manejo de teclas de flecha
         int arrowKey = getch();
         if (!isPaused) {                                                     // Si el juego no está en pausa
            if (arrowKey == 75 && canPlacePiece(activePiece, -1, 0)) {        // Flecha izquierda
            activePiece->x--;                                                 // Mueve la pieza a la izquierda
            } else if (arrowKey == 77 && canPlacePiece(activePiece, 1, 0)) {  // Flecha derecha 
            activePiece->x++;                                                 // Mueve la pieza a la derecha
            } else if (arrowKey == 80 && canPlacePiece(activePiece, 0, 1)) {  // Flecha abajo
            activePiece->y++;                                                 // Mueve la pieza hacia abajo
            } else if (arrowKey == 72) {                                      // Flecha arriba (rotar la pieza)
            rotatePiece(activePiece);                                         // Rota la pieza
            }
         }
      } else if (key == 'p' || key == 'P') {    // Pausar/reanudar
         isPaused = !isPaused;                  // Cambia el estado de pausa  
         } else if (!isPaused) {                // Si el juego no está en pausa
            if (key == 'a' && canPlacePiece(activePiece, -1, 0)) {            // 'a' para mover izquierda   
               activePiece->x--;
         } else if (key == 'd' && canPlacePiece(activePiece, 1, 0)) {         // 'd' para mover derecha
            activePiece->x++;
         } else if (key == 's' && canPlacePiece(activePiece, 0, 1)) {         // 's' para bajar
            activePiece->y++;
         } else if (key == 'w') {                                             // 'w' para rotar
            rotatePiece(activePiece);
         } else if (key == ' ') {                                             // Barra espaciadora para colocar la pieza
            while (canPlacePiece(activePiece, 0, 1)) {
               activePiece->y++;                                              // Coloca la pieza en la parte más baja posible
            }
         }
      }
   }
}

void gameLoop() {    // Bucle principal del juego                                     
   cout << "\033[?25l";
   Piece *activePiece = createRandomPiece();          // Crea la pieza activa
   upcomingPieces.push(createRandomPiece());          // Agrega una nueva pieza a la cola
   Piece *nextPiece = upcomingPieces.front();         // Obtiene la próxima pieza
   upcomingPieces.pop();                              // Elimina la próxima pieza de la cola
   bool gameOver = false, isPaused = false;           // Estado del juego
   auto lastDropTime = chrono::steady_clock::now();   // Tiempo de la última caída

   while (!gameOver) {     // Bucle del juego
      renderGame(activePiece, nextPiece, isPaused);  // Renderiza el estado del juego    
      handleInput(activePiece, isPaused);            // Llama a la función handleInput para manejar la entrada del usuario

      if (!isPaused && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - lastDropTime).count() >= speed) {
         if (canPlacePiece(activePiece, 0, 1)) {         // Mueve la pieza hacia abajo
            activePiece->y++;
         } else {
            placePiece(activePiece);                     // Coloca la pieza en el tablero
            clearFullLines();                            // Limpia las líneas completas
            delete activePiece;                          // Libera la memoria de la pieza activa
            activePiece = nextPiece;                     // La próxima pieza se convierte en la activa
            if (upcomingPieces.empty()) {
               upcomingPieces.push(createRandomPiece()); // Agrega una nueva pieza si la cola está vacía   
            }
            nextPiece = upcomingPieces.front();          // Obtiene la nueva próxima pieza
            upcomingPieces.pop();                        // Elimina la próxima pieza de la cola
            if (!canPlacePiece(activePiece, 0, 0)) {
               gameOver = true;                          // Termina el juego si no se puede colocar la pieza
            }
         }
         lastDropTime = chrono::steady_clock::now();     // Actualiza el tiempo de la última caída
      }

      if (gameCancelled) {
         gameOver = true;                                // Termina el juego si se cancela
      }

      this_thread::sleep_for(chrono::milliseconds(10));  // Pausa breve para evitar el uso excesivo de CPU
   }

   displayGameOver();                                    // Muestra el mensaje de Game Over
   freePieceMemory(activePiece, nextPiece);              // Libera la memoria de las piezas
}

void freePieceMemory(Piece *activePiece, Piece *nextPiece) {   // Libera la memoria de las piezas activas
   delete activePiece;     // Libera la pieza activa
   delete nextPiece;       // Libera la próxima pieza
}

void displayGameOver() {   // Muestra el mensaje de Game Over
   cout << "\033[2J\033[H"
        << "|====================|\n"
        << "|    FIN DEL JUEGO   |\n"
        << "|====================|\n\n"
        << "Puntaje final: " << score << "\n";  // Muestra el puntaje final
}

void signalHandler(int signum) {    // Maneja señales como SIGINT
   gameCancelled = true;   // Cambia el estado de cancelación del juego
}