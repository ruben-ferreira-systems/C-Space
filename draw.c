#include "draw.h"
#include "boss.h"
#include "levels.h"
#include <unistd.h>
#include <stdlib.h>

void draw_game(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, int nivel_atual, Boss *b, Projétil tiros_ini[MAX_TIROS_INIMIGO])
{
    clear();

  // 1. Desenha o mapa/cenário visível com Filtro CRT ASCII Puro
    for (int y = 0; y < ALTURA_TELA; y++)
    {
        for (int x = 0; x < LARGURA_TELA; x++)
        {
            int mundo_x = x + offset;
            
            // Margens laterais fixas do ecrã
            if (x == 0 || x == LARGURA_TELA - 1)
            {
                mvaddch(y, x, '|'); // Delimitador vertical limpo
            }
            else if (mundo_x >= 0 && mundo_x < configuracao_niveis[nivel_atual].comprimento_mapa)
            {
                char ch = mapa[y][mundo_x];

                if (ch == '#') 
                {
                    // EFEITO CRT SCANLINE: Linhas pares desenham '#', linhas ímpares desenham '='
                    // Isto cria riscas horizontais perfeitas no ecrã de forma puramente textual!
                    if (y % 2 == 0)
                        mvaddch(y, x, '#');
                    else
                        mvaddch(y, x, '=');
                }
                // Efeito de brilho dinâmico do item de vida extra no túnel
                else if (ch == '+')
                {
                    if (offset % 4 < 2)
                        mvaddch(y, x, '+');
                    else
                        mvaddch(y, x, '*');
                }
                else
                {
                    mvaddch(y, x, ch);
                }
            }
        }
    }


    // 2. Desenhar tiros do jogador (|)
    for (int i = 0; i < MAX_TIROS; i++)
    {
        if (p->tiro[i].ativo == 1)
        {
            int ecra_x = p->tiro[i].x - offset;
            if (ecra_x > 0 && ecra_x < LARGURA_TELA - 1)
            {
                mvaddch(p->tiro[i].y, ecra_x, '-');
            }
        }
    }

    // 3. Desenhar tiros dos Inimigos / Itens de Cura em Queda Física
    for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
    {
        if (tiros_ini[i].ativo == 1) // Projétil inimigo normal
        {
            int ecra_x = tiros_ini[i].x - offset;
            if (ecra_x > 0 && ecra_x < LARGURA_TELA - 1)
            {
                mvaddch(tiros_ini[i].y, ecra_x, '-');
            }
        }
        else if (tiros_ini[i].ativo == 2) // Item de vida extra em queda livre (Brilho dinâmico)
        {
            int ecra_x = tiros_ini[i].x - offset;
            if (ecra_x > 0 && ecra_x < LARGURA_TELA - 1)
            {
                if (offset % 4 < 2)
                    mvaddch(tiros_ini[i].y, ecra_x, '+');
                else
                    mvaddch(tiros_ini[i].y, ecra_x, '*');
            }
        }
    }

    // 4. Escudo termal de imunidade do Jogador
    if (p->x > 0 && p->x < LARGURA_TELA - 1)
    {
        if (p->imunidade > 0)
        {
            if (p->imunidade % 2 == 0)
            {
                mvprintw(p->y, p->x - 1, "(>");
                mvaddch(p->y, p->x + 1, ')');
            }
            else
            {
                mvaddch(p->y, p->x, '>');
            }
        }
        else
        {
            mvaddch(p->y, p->x, '>');
        }
    }

    // 5. Desenhar o Boss Final (Delega a renderização por nível no boss.c)
    if (b->ativo == 1)
    {
        desenhar_boss(b, offset, nivel_atual);
    }

    // 6. HUD DINÂMICO UNIFICADO (Trancado na Linha 0) - Corrigido para 3 dígitos
    if (b->ativo == 1)
    {
        mvprintw(0, 2, " LIVES: %d ", p->vidas);
        // CORREÇÃO: %03d garante espaço limpo para vidas até 999 sem quebrar o texto
        mvprintw(0, LARGURA_TELA - 21, " BOSS HP: %03d/%d ", b->vida, BOSS_VIDA_MAX);
    }
    else
    {
        mvprintw(0, 2, " LIVES: %d ", p->vidas);
        mvprintw(0, 14, " SCORE: %07ld ", p->pontuacao);
        mvprintw(0, LARGURA_TELA - 11, " STAGE: %d ", nivel_atual + 1);
    }

    refresh();
}

// Função draw_transition isolada como uma entidade global limpa
void draw_transition(int nivel_atual)
{
    nodelay(stdscr, FALSE);
    flushinp();

    // Animação de Explosão Retro
    for (int raio = 1; raio <= 4; raio++)
    {
        clear();
        for (int i = 0; i < LARGURA_TELA; i++)
        {
            mvaddch(0, i, '#');
            mvaddch(ALTURA_TELA - 1, i, '#');
        }
        mvaddch(7 - raio, 34 - raio, '*');
        mvaddch(7 - raio, 34 + raio, '+');
        mvaddch(7 + raio, 34 - raio, '.');
        mvaddch(7 + raio, 34 + raio, '*');
        if (raio > 2)
        {
            mvaddch(7, 34 - (raio * 2), '<');
            mvaddch(7, 34 + (raio * 2), '>');
        }
        refresh();
        usleep(150000);
    }

    clear();
    refresh();
    usleep(2000000);

    // WARM-UP EXCLUSIVO / TEXTO DIDÁTICO NO INÍCIO DA FASE 2
    if (nivel_atual == 0)
    {
        for (int pisca = 0; pisca < 3; pisca++)
        {
            clear();
            for (int i = 0; i < LARGURA_TELA; i++)
            {
                mvaddch(ALTURA_TELA / 2 - 3, i, '=');
                mvaddch(ALTURA_TELA / 2 + 3, i, '=');
            }
            mvprintw(ALTURA_TELA / 2 - 1, (LARGURA_TELA / 2) - 8, "UPGRADE UNLOCKED!");
            mvprintw(ALTURA_TELA / 2 + 1, (LARGURA_TELA / 2) - 12, "USE A / D TO MOVE < & >");
            refresh();
            usleep(800000);
            clear();
            refresh();
            usleep(200000);
        }
    }

    // Ecrã de Carregamento de Próximo Nível (Inglês)
    for (int pisca = 0; pisca < 2; pisca++)
    {
        clear();
        for (int i = 0; i < LARGURA_TELA; i++)
        {
            mvaddch(ALTURA_TELA / 2 - 3, i, '-');
            mvaddch(ALTURA_TELA / 2 + 3, i, '-');
        }
        mvprintw(ALTURA_TELA / 2 - 1, (LARGURA_TELA / 2) - 9, "STAGE %d COMPLETED!", nivel_atual + 1);
        mvprintw(ALTURA_TELA / 2 + 1, (LARGURA_TELA / 2) - 11, "Preparing next zone...");
        refresh();
        usleep(700000);
        clear();
        refresh();
        usleep(300000);
    }
}
void animacao_intro_binaria()
{
    nodelay(stdscr, TRUE); // Garante que a animação corre em tempo real
    int colunas_y[LARGURA_TELA] = {0}; // Controla a altura de queda de cada coluna

    // Inicializa as colunas com alturas aleatórias negativas para entrarem faseadas
    for (int i = 0; i < LARGURA_TELA; i++) {
        colunas_y[i] = -(rand() % ALTURA_TELA);
    }

    // Corre a animação durante exatamente 80 frames (cerca de 2.5 segundos)
    for (int frame = 0; frame < 80; frame++)
    {
        // 1. Atualizar e desenhar a chuva de 0s e 1s
        for (int x = 1; x < LARGURA_TELA - 1; x++)
        {
            // Apaga o rasto antigo na linha anterior
            if (colunas_y[x] - 1 >= 0 && colunas_y[x] - 1 < ALTURA_TELA) {
                mvaddch(colunas_y[x] - 1, x, ' ');
            }

            // Desenha o dígito binário atual
            if (colunas_y[x] >= 0 && colunas_y[x] < ALTURA_TELA) {
                char digito = (rand() % 2 == 0) ? '0' : '1';
                mvaddch(colunas_y[x], x, digito);
            }

            // Move a gota binária para baixo
            colunas_y[x]++;

            // Se a gota sair do ecrã, reinicia no topo aleatoriamente
            if (colunas_y[x] >= ALTURA_TELA) {
                colunas_y[x] = 0;
            }
        }

        // 2. A meio da animação (frame 35), o título começa a hackear e a aparecer
        if (frame > 35)
        {
            int cx = LARGURA_TELA / 2;
            int cy = ALTURA_TELA / 2;
            
            // Desenha uma caixa preta limpa no centro para destacar o título
            for (int ty = cy - 2; ty <= cy + 2; ty++) {
                for (int tx = cx - 12; tx <= cx + 12; tx++) {
                    mvaddch(ty, tx, ' ');
                }
            }
            // Moldura da mensagem de boot
            mvprintw(cy - 2, cx - 11, "======================");
            mvprintw(cy - 1, cx - 10, " C-SPACE BOOT SEQUENCE ");
            mvprintw(cy,     cx - 9,  "  SYSTEMS: [ONLINE]   ");
            mvprintw(cy + 2, cx - 11, "======================");
        }

        refresh();
        usleep(40000); // Velocidade da chuva binária
    }

    // Limpeza final para entregar o ecrã ao Nível 1
    clear();
    refresh();
}

