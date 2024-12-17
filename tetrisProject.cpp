#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <csignal>

using namespace std;

const int WIDTH = 10, HEIGHT = 20;

vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));
bool gameCancelled = false;

void displayTitleScreen() {
    cout << "=====================================\n"
         << "=         BIENVENIDO A TETRIS       =\n"
         << "=====================================\n\n"
         << "Presiona Enter para comenzar...\n";
}
// sss
void resetGame() {
    board.assign(HEIGHT, vector<int>(WIDTH, 0));
    cout << "Juego reiniciado.\n";
}

void signalHandler(int signum) {
    gameCancelled = true;
}

int main() {
    signal(SIGINT, signalHandler);
    displayTitleScreen();
    cin.get(); // Esperar Enter
    resetGame();
    cout << "Juego iniciado.\n";
    return 0;
}
