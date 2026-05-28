#ifndef MENU_H
#define MENU_H

#define MAX_LIST 9

#include "space.h"

// Estrutura binária simples para ler/escrever o progresso no disco
typedef struct {
    int nivel;
    int vidas;
    long pontuacao;
} SaveData;

// Opções enumeradas para controlo do menu de introdução
typedef enum {
    OPCAO_PLAY,
    OPCAO_LOAD,
    OPCAO_COMANDOS,
    OPCAO_CREDITOS,
    OPCAO_MAX_SCORES,
    OPCAO_SAIR
} OpcoesMenu;

// Estrutura para gerir as pontuações máximas
typedef struct {
    char nome[8]; // Suporta até 7 caracteres + terminador '\0'
    long score;
} HighScore;


// Protótipos das funções principais do Menu
int executar_menu_principal(Player *p, int *nivel_alvo);
void guardar_jogo(int nivel, int vidas, long pontuacao);
int carregar_jogo(Player *p, int *nivel_alvo);
void verificar_e_gravar_highscore(long pontuacao_final);
void carregar_e_desenhar_highscores();

#endif
