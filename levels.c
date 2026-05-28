#include "levels.h"
#include <stdlib.h>
#include <math.h>

// Configuração de cada um dos 3 níveis do jogo
DefenicaoNivel configuracao_niveis[MAX_NIVEIS] = {
    // { comprimento, % obstáculo, buffer_inicio, buffer_fim }
    { 250, 4, 15, 40 },  // Nível 1: Curto, introdutório, poucas rochas
    { 550, 5, 20, 50 },  // Nível 2: Médio, túnel com inimigos móveis
    { 850, 6, 25, 80 }   // Nível 3: Longo, arena final massiva
};

void gerar_mapa_dinamico(char mapa[ALTURA_TELA][LARGURA_MAPA], unsigned int seed, int nivel_atual) {
    srand(seed); 
    
    // Carrega as propriedades do nível atual
    DefenicaoNivel cfg = configuracao_niveis[nivel_atual];
    
    // Posição de drop da vida calculada de forma relativa ao fim do mapa atual
    int coordenada_vida_segura = cfg.comprimento_mapa - cfg.buffer_fim - 15;

    for (int y = 0; y < ALTURA_TELA; y++) {
        for (int x = 0; x < LARGURA_MAPA; x++) {
            // Se passar do comprimento máximo deste nível, preenche com vazio/segurança
            if (x >= cfg.comprimento_mapa) {
                mapa[y][x] = ' ';
                continue;
            }
            
            // Paredes laterais absolutas do fim do nível atual
            if (x == 0 || x == cfg.comprimento_mapa - 1) {
                mapa[y][x] = '#';
                continue;
            }

            // ARENA DO BOSS (Espaço retangular limpo no fim do nível atual)
            if (x >= cfg.comprimento_mapa - cfg.buffer_fim) {
                mapa[y][x] = (y == 0 || y == ALTURA_TELA - 1) ? '#' : ' ';
                continue;
            }

            // BUFFER DE INÍCIO DO JOGADOR
            if (x < cfg.buffer_inicio) {
                mapa[y][x] = (y == 0 || y == ALTURA_TELA - 1) ? '#' : ' ';
                continue;
            }

            // CÁLCULO DAS ONDAS DO TÚNEL (Idêntico ao teu original)
            double freq1 = (double)x / 8.0 + (double)seed;
            double freq2 = (double)x / 12.0 - (double)seed;
            int onda_topo = 2 + (int)(abs((int)(sin(freq1) * 3.5))); 
            int onda_fundo = 2 + (int)(abs((int)(cos(freq2) * 3.5)));

            if (onda_topo + onda_fundo >= ALTURA_TELA - 2) {
                onda_topo = 4;
                onda_fundo = 4;
            }

            if (y < onda_topo || y >= (ALTURA_TELA - onda_fundo)) {
                mapa[y][x] = '#';
            } else if (x < 7) {
                mapa[y][x] = ' ';
            } else {
                // DROP FIXO DA VIDA ANTES DO BOSS (Dinâmico para o tamanho do mapa)
                if (y == 7 && x == coordenada_vida_segura) {
                    mapa[y][x] = '+';
                }
                // ZONA DE TRANSIÇÃO LIMPA ANTES DO BOSS
                else if (x >= cfg.comprimento_mapa - (cfg.buffer_fim + 30)) {
                    mapa[y][x] = ' ';
                }
                // ZONA DE JOGO NORMAL
                else {
                    if ((rand() % 100) < cfg.chance_obstaculo) {
                        mapa[y][x] = (nivel_atual == 0) ? 'O' : 'x'; 
                    } else {
                        mapa[y][x] = ' ';
                    }
                }
            }
        }
    }
}
