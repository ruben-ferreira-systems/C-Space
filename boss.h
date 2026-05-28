#ifndef BOSS_H
#define BOSS_H

#include "space.h"

void reset_boss_ia();

void IA_gerir_boss(Boss *b, Player *p, int offset, Projétil tiros_ini[], int nivel_atual, char mapa[ALTURA_TELA][LARGURA_MAPA]);


void desenhar_boss(Boss *b, int offset, int nivel_atual);

#endif // BOSS_H