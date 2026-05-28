#ifndef DRAW_H
#define DRAW_H

#include "space.h"

// Renderiza todos os objetos ativos na janela do terminal ncurses
void draw_game(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, int nivel_atual, Boss *b, Projétil tiros_ini[MAX_TIROS_INIMIGO]);

// Apresenta as animações retro e ecrãs de transição intermédios
void draw_transition(int nivel_atual);

void animacao_intro_binaria();

#endif