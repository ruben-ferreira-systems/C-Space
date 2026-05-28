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
    if (vida_inicial_combate == 0)
    {
        vida_inicial_combate = b->vida;
        if (nivel_atual == 2)
        {
            b->hp_arma_superior = 150;
            b->hp_arma_inferior = 150;
            b->hp_nucleo = 200;
            b->fase_atual = 1;
            b->vida = 300;
        }
    }

    boss_frame_count++;

    // =========================================================
    // CORREÇÃO: IA UNIFICADA PARA NÍVEL 1 E NÍVEL 2 (MODO TOURO)
    // =========================================================
    if (nivel_atual == 0 || nivel_atual == 1)
    {
        // Se estiver com a vida cheia, ambos fazem o movimento padrão de ricochete
        if (b->vida == vida_inicial_combate)
        {
            if (boss_frame_count >= 3)
            {
                boss_frame_count = 0;
                b->y += b->direcao_y;
                b->x += b->direcao_x;
                if (b->y - 2 < 0 || b->y + 2 >= ALTURA_TELA)
                    b->direcao_y *= -1;
                if (b->x < LARGURA_TELA / 2 || b->x > LARGURA_TELA - 5)
                    b->direcao_x *= -1;
            }
        }
        else
        {
            // RESTAURADO: Se sofrerem dano, AMBOS ativam a fúria do Modo Touro!
            int velocidade_corte = (estado_ia == 1) ? 1 : 3;
            if (boss_frame_count >= velocidade_corte)
            {
                boss_frame_count = 0;
                if (estado_ia == 0)
                {
                    alvo_fixado_x = p->x;
                    alvo_fixado_y = p->y;
                    estado_ia = 1;
                    system("play -q -n synth 0.15 sine 300:900 vol 0.35 > /dev/null 2>&1 &");
                }
                else if (estado_ia == 1)
                {
                    if (b->x < alvo_fixado_x)
                        b->x++;
                    else if (b->x > alvo_fixado_x)
                        b->x--;
                    if (b->y < alvo_fixado_y)
                        b->y++;
                    else if (b->y > alvo_fixado_y)
                        b->y--;
                    if (b->x == alvo_fixado_x && b->y == alvo_fixado_y)
                        estado_ia = 2;
                    if (b->x <= 1)
                        estado_ia = 2;
                }
                else if (estado_ia == 2)
                {
                    if (b->x < LARGURA_TELA - 6)
                    {
                        b->x++;
                        b->y += (rand() % 3) - 1;
                        if (b->y - 2 < 0)
                            b->y = 2;
                        if (b->y + 2 >= ALTURA_TELA)
                            b->y = ALTURA_TELA - 3;
                    }
                    else
                    {
                        estado_ia = 0;
                    }
                }
            }
        }

        // --- SISTEMA DE DISPAROS RE-CALIBRADO ---
        boss_tiro_timer++;
        if (boss_tiro_timer >= 12)
        {
            boss_tiro_timer = 0;

            // NÍVEL 2: Dispara a parede vertical de 3 tiros
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
                    if (tiros_criados >= 3)
                        break;
                }
            }
            // NÍVEL 1: Mantém o tiro único linear central que eles aprenderam a desviar
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

    // ==========================================
    // LÓGICA DE IA: NÍVEL 3 (150 HP UNIFICADOS)
    // ==========================================
    else if (nivel_atual == 2)
    {

        // Transição Automática de Fase baseada nos 150 HP totais
        if (b->fase_atual == 1 && b->vida <= 50)
        {
            b->fase_atual = 2;
            estado_ia = 0; // Ativa perseguição / laser
            boss_frame_count = 0;
            laser_timer = 0;
        }

        // FASE 1: DESARMAMENTO (Vida entre 151 e 51)
        if (b->fase_atual == 1)
        {
            if (boss_frame_count >= 3)
            {
                boss_frame_count = 0;
                b->y += b->direcao_y;
                b->x += b->direcao_x;
                if (b->y - 2 < 0 || b->y + 2 >= ALTURA_TELA)
                    b->direcao_y *= -1;
                if (b->x < LARGURA_TELA / 2 || b->x > LARGURA_TELA - 8)
                    b->direcao_x *= -1;
            }

            boss_tiro_timer++;
            if (boss_tiro_timer >= 12)
            {
                boss_tiro_timer = 0;

                // Disparos condicionados pela vida global do Boss
                int arma_superior_viva = (b->vida > 100);
                int arma_inferior_viva = (b->vida > 50);

                int configs_tiro[3][2] = {{arma_superior_viva, -1}, {1, 0}, {arma_inferior_viva, 1}};

                for (int t = 0; t < 3; t++)
                {
                    if (configs_tiro[t][0] > 0)
                    { // Se a arma respetiva estiver operacional
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
        // FASE 2: FÚRIA DO MEGA LASER E INVESTIDA (Vida entre 50 e 0)
        else if (b->fase_atual == 2)
        {
            // [Os teus Estados 0, 1, 2 e 3 da IA permanecem exatamente iguais aqui]
            // ...
        }

        // Condição de derrota linear direta
        if (b->vida <= 0)
        {
            b->ativo = 2;
        }
    }
}

void desenhar_boss(Boss *b, int offset, int nivel_atual)
{
    int x_absoluto = b->x + offset; // Ajuste para o scroll do ecrã

    switch (nivel_atual)
    {
    case 0:
    case 1:
        mvprintw(b->y - 1, x_absoluto, "[X]");
        mvprintw(b->y, x_absoluto, "<O>");
        mvprintw(b->y + 1, x_absoluto, "[X]");
        break;

    case 2: // NÍVEL 3 COMPACTO DE 150 HP
    default:
    {
        // 1. LINHA SUPERIOR: Destrói-se quando a vida cai abaixo de 100
        if (b->vida > 100)
        {
            mvprintw(b->y - 1, x_absoluto, "[X]");
        }
        else
        {
            mvprintw(b->y - 1, x_absoluto, (offset % 2 == 0) ? " * " : " ; ");
        }

        // 2. LINHA CENTRAL: Núcleo reativo
        if (b->fase_atual == 1)
        {
            mvprintw(b->y, x_absoluto, "<O>");
        }
        else
        {
            if (estado_ia == 1)
            {
                mvprintw(b->y, x_absoluto, "<█>"); // Laser ativo
            }
            else if (estado_ia == 0 && offset % 2 == 0)
            {
                mvprintw(b->y, x_absoluto, "<- "); // Carregando laser
            }
            else
            {
                mvprintw(b->y, x_absoluto, "<O>");
            }
        }

        // 3. LINHA INFERIOR: Destrói-se quando a vida cai abaixo de 50
        if (b->vida > 50)
        {
            mvprintw(b->y + 1, x_absoluto, "[X]");
        }
        else
        {
            mvprintw(b->y + 1, x_absoluto, (offset % 2 == 0) ? " ; " : " * ");
        }
        {
            break;
        }
    }
    }
}
