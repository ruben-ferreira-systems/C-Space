#include "boss.h"
#include "levels.h"
#include <stdlib.h>
#include <math.h>

// Variáveis de controlo internas da IA do Boss
static int boss_frame_count = 0;
static int estado_ia = 0; // No Lvl 3: 0=Mira, 1=Laser, 2=Investida, 3=Recuo
static int alvo_fixado_x = 0;
static int alvo_fixado_y = 0;
static int boss_tiro_timer = 0;
static int vida_inicial_combate = 0;

// Temporizadores exclusivos para a mecânica do Mega Laser no Nível 3
static int laser_timer = 0;

void reset_boss_ia()
{
    boss_frame_count = 0;
    estado_ia = 0;
    alvo_fixado_x = 0;
    alvo_fixado_y = 0;
    boss_tiro_timer = 0;
    vida_inicial_combate = 0;
    laser_timer = 0;
}

void IA_gerir_boss(Boss *b, Player *p, int offset, Projétil tiros_ini[], int nivel_atual, char mapa[ALTURA_TELA][LARGURA_MAPA])
{
    // --- 1. INICIALIZAÇÃO UNIFICADA DO COMBATE ---
    boss_frame_count++; // Incrementa o contador de frames para a IA do Boss
    if (vida_inicial_combate == 0)
    {
        if (nivel_atual == 2)
        {
            b->vida = 150;      // Fixado nos 150 HP lineares para o Nível 3
            b->fase_atual = 1;  // Começa na Fase de Desarmamento
        }
        vida_inicial_combate = b->vida;
    }

    // --- 2. INTELIGÊNCIA DOS NÍVEIS 1 E 2 (0 E 1) ---
    if (nivel_atual == 0 || nivel_atual == 1)
    {
        if (b->vida == vida_inicial_combate)
        {
            if (boss_frame_count >= 3)
            {
                boss_frame_count = 0;
                b->y += b->direcao_y;
                b->x += b->direcao_x;
                if (b->y - 2 < 0 || b->y + 2 >= ALTURA_TELA) b->direcao_y *= -1;
                if (b->x < LARGURA_TELA / 2 || b->x > LARGURA_TELA - 5) b->direcao_x *= -1;
            }
        }
        else
        {
            int velocidade_corte = (estado_ia == 1) ? 1 : 3;
            if (boss_frame_count >= velocidade_corte)
            {
                boss_frame_count = 0;
                if (estado_ia == 0)
                {
                    alvo_fixado_x = p->x;
                    alvo_fixado_y = p->y;
                    estado_ia = 1;
                    if (som_ativo) (void)system("play -q -n synth 0.15 sine 300:900 vol 0.35 > /dev/null 2>&1 &");
                }
                else if (estado_ia == 1)
                {
                    if (b->x < alvo_fixado_x) b->x++;
                    else if (b->x > alvo_fixado_x) b->x--;
                    if (b->y < alvo_fixado_y) b->y++;
                    else if (b->y > alvo_fixado_y) b->y--;
                    
                    if ((b->x == alvo_fixado_x && b->y == alvo_fixado_y) || b->x <= 1) estado_ia = 2;
                }
                else if (estado_ia == 2)
                {
                    if (b->x < LARGURA_TELA - 6)
                    {
                        b->x++;
                        b->y += (rand() % 3) - 1;
                        if (b->y - 2 < 0) b->y = 2;
                        if (b->y + 2 >= ALTURA_TELA) b->y = ALTURA_TELA - 3;
                    }
                    else
                    {
                        estado_ia = 0;
                    }
                }
            }
        }

        boss_tiro_timer++;
        if (boss_tiro_timer >= 12)
        {
            boss_tiro_timer = 0;
            if (nivel_atual == 1)
            {
                int tiros_criados = 0;
                for (int t_y = b->y - 1; t_y <= b->y + 1; t_y++)
                {
                    for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                    {
                        if (tiros_ini[i].ativo == 0)
                        {
                            tiros_ini[i].x = b->x - 1 + offset;
                            tiros_ini[i].y = t_y;
                            tiros_ini[i].ativo = 1;
                            tiros_criados++;
                            break;
                        }
                    }
                    if (tiros_criados >= 3) break;
                }
            }
            else
            {
                for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                {
                    if (tiros_ini[i].ativo == 0)
                    {
                        tiros_ini[i].x = b->x - 1 + offset;
                        tiros_ini[i].y = b->y;
                        tiros_ini[i].ativo = 1;
                        break;
                    }
                }
            }
        }
    }

    // ===================================================================
    // --- 3. INTELIGÊNCIA DO NÍVEL 3 (NIVEL_ATUAL == 2) ---
    // ===================================================================
    else if (nivel_atual == 2)
    {
        // Transição de Fase quando a vida global desce para 50 ou menos
        if (b->fase_atual == 1 && b->vida <= 50)
        {
            b->fase_atual = 2;
            estado_ia = 0; 
            boss_frame_count = 0;
            laser_timer = 0;
        }

        // --- FASE 1: DESARMAMENTO (Vida de 150 a 51) ---
        if (b->fase_atual == 1)
        {
            if (boss_frame_count >= 3)
            {
                boss_frame_count = 0;
                b->y += b->direcao_y;
                b->x += b->direcao_x;
                if (b->y - 2 < 0 || b->y + 2 >= ALTURA_TELA) b->direcao_y *= -1;
                if (b->x < LARGURA_TELA / 2 || b->x > LARGURA_TELA - 8) b->direcao_x *= -1;
            }

            boss_tiro_timer++;
            if (boss_tiro_timer >= 12)
            {
                boss_tiro_timer = 0;

                int arma_superior_viva = (b->vida > 100);
                int arma_inferior_viva = (b->vida > 50);
                int configs_tiro[3][2] = {{arma_superior_viva, -1}, {1, 0}, {arma_inferior_viva, 1}};

                for (int t = 0; t < 3; t++)
                {
                    if (configs_tiro[t][0] > 0)
                    {
                        for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                        {
                            if (tiros_ini[i].ativo == 0)
                            {
                                tiros_ini[i].x = b->x - 1 + offset;
                                tiros_ini[i].y = b->y + configs_tiro[t][1];
                                tiros_ini[i].ativo = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        // --- FASE 2: FÚRIA DO MEGA LASER E INVESTIDA (Vida de 50 a 1) ---
        else if (b->fase_atual == 2)
        {
            // ESTADO 0: Perseguição suave e Alinhamento com o Jogador (Mira)
            if (estado_ia == 0)
            {
                if (boss_frame_count >= 2)
                {
                    boss_frame_count = 0;
                    if (b->y < p->y && b->y < ALTURA_TELA - 3) b->y++;
                    else if (b->y > p->y && b->y > 2) b->y--;
                }

                laser_timer++;
                if (som_ativo && laser_timer % 15 == 0)
                {
                    (void)system("play -q -n synth 0.04 sine 2000 vol 0.25 > /dev/null 2>&1 &");
                }

                if (laser_timer >= 60) // 2 segundos de mira
                {
                    laser_timer = 0;
                    alvo_fixado_y = b->y; // Tranca o Y do laser
                    estado_ia = 1;
                    if (som_ativo) (void)system("play -q -n synth 0.60 square 150 vol 0.35 > /dev/null 2>&1 &");
                }
            }
            // ESTADO 1: Ativação do Mega Laser
            else if (estado_ia == 1)
            {
                laser_timer++;
                
                // Desenha o laser de forma segura na matriz física temporária para colisão imediata
                for (int x_laser = offset + 1; x_laser < b->x + offset; x_laser++)
                {
                    if (x_laser < LARGURA_MAPA) mapa[alvo_fixado_y][x_laser] = '=';
                }

                if (laser_timer >= 35) // Dura 1 segundo ativo
                {
                    laser_timer = 0;
                    // Limpa do mapa para libertar a investida física
                    for (int x_laser = offset + 1; x_laser < b->x + offset; x_laser++)
                    {
                        if (x_laser < LARGURA_MAPA && mapa[alvo_fixado_y][x_laser] == '=') mapa[alvo_fixado_y][x_laser] = ' ';
                    }
                    alvo_fixado_x = p->x; // Guarda posição horizontal do jogador para o Touro
                    estado_ia = 2;
                    if (som_ativo) (void)system("play -q -n synth 0.20 sine 400:1200 vol 0.35 > /dev/null 2>&1 &");
                }
            }
            // ESTADO 2: Investida Rápida (Modo Touro) à esquerda
            else if (estado_ia == 2)
            {
                if (boss_frame_count >= 1)
                {
                    boss_frame_count = 0;
                    b->x--; 
                    if (b->x <= alvo_fixado_x || b->x <= 2) estado_ia = 3;
                }
            }
            // ESTADO 3: Recuo Lento e Vulnerável para a direita
            else if (estado_ia == 3)
            {
                if (boss_frame_count >= 4)
                {
                    boss_frame_count = 0;
                    b->x++;
                    if (b->x >= LARGURA_TELA - 10)
                    {
                        b->x = LARGURA_TELA - 10;
                        estado_ia = 0; // Reinicia o ciclo para perseguição
                    }
                }
            }
        }

        if (b->vida <= 0) b->ativo = 2; // Estado Derrotado
    }
}


void desenhar_boss(Boss *b, int offset, int nivel_atual) {
    // Para evitar avisos de variável não usada se não precisares do offset aqui
    (void)offset; 

    switch (nivel_atual) {
        case 0:
        case 1: // --- NÍVEL 1 E 2 ---
            mvprintw(b->y - 1, b->x, "[X]");
            mvprintw(b->y,     b->x, "<O>");
            mvprintw(b->y + 1, b->x, "[X]");
            break;

        case 2: // --- NÍVEL 3 (150 HP UNIFICADOS) ---
        default: {
            // 1. LINHA SUPERIOR: Destrói-se quando a vida cai abaixo de 100
            if (b->vida > 100) {
                mvprintw(b->y - 1, b->x, "[X]");
            } else {
                // Se destruída, mostra faíscas retro intermitentes baseadas no offset global
                mvprintw(b->y - 1, b->x, (offset % 2 == 0) ? " * " : " ; ");
            }

            // 2. LINHA CENTRAL: Núcleo reativo
            if (b->fase_atual == 1) {
                mvprintw(b->y, b->x, "<O>");
            } else {
                // Na Fase 2, o núcleo reage ao estado do Mega Laser
                if (estado_ia == 1) {
                    mvprintw(b->y, b->x, "<█>"); // Laser ativo
                } else if (estado_ia == 0 && offset % 2 == 0) {
                    mvprintw(b->y, b->x, "<- "); // Carregando/Mira
                } else {
                    mvprintw(b->y, b->x, "<O>");
                }
            }

            // 3. LINHA INFERIOR: Destrói-se quando a vida cai abaixo de 50
            if (b->vida > 50) {
                mvprintw(b->y + 1, b->x, "[X]");
            } else {
                mvprintw(b->y + 1, b->x, (offset % 2 == 0) ? " ; " : " * ");
            }
            break;
        }
    }
}


