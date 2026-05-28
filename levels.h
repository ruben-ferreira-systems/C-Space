#ifndef LEVELS_H
#define LEVELS_H

#include "space.h"

// Estrutura que define os metadados de cada nível
typedef struct {
    int comprimento_mapa;    // Ex: 250 para Nível 1, 600 para Nível 2, 900 para Nível 3
    int chance_obstaculo;    // % de chance de nascer perigos
    int buffer_inicio;       // Espaço seguro inicial
    int buffer_fim;          // Onde começa a transição para a arena do Boss
} DefenicaoNivel;

// Declaração do array global de configurações de nível
extern DefenicaoNivel configuracao_niveis[MAX_NIVEIS];

// Nova assinatura da função de gerar mapa, agora dependente das configs dinâmicas
void gerar_mapa_dinamico(char mapa[ALTURA_TELA][LARGURA_MAPA], unsigned int seed, int nivel_atual);

#endif