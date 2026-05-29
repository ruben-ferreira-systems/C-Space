#include "space.h"
#include "boss.h"
#include "levels.h"
#include "draw.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int som_ativo = 1;
int tecla_up = 'w';
int tecla_down = 's';
int tecla_left = 'a';
int tecla_right = 'd';
int tecla_fire = ' ';

void init_game()
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE); // Suporte estável para setas e controlos

    if (has_colors())
    {
        start_color();
        // Par 1: Texto Verde sobre Fundo Preto (O visual clássico do terminal)
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        bkgd(COLOR_PAIR(1));
    }
}

void gerar_mapa(char mapa[ALTURA_TELA][LARGURA_MAPA], unsigned int seed, int nivel_atual)
{
    srand(seed);
    // Vai buscar o comprimento real configurado para este nível específico
    int comp_real = configuracao_niveis[nivel_atual].comprimento_mapa;

    for (int y = 0; y < ALTURA_TELA; y++)
    {
        for (int x = 0; x < LARGURA_MAPA; x++)
        {
            // 1. Paredes laterais absolutas do fim do mundo (Ajustado ao tamanho real do nível)
            if (x == 0 || x == comp_real - 1)
            {
                mapa[y][x] = '#';
                continue;
            }

            // 2. ARENA DO BOSS DINÂMICA (Últimos 40 blocos do nível atual)
            // Ecrã totalmente limpo e retangular, sem as ondas do túnel
            if (x >= comp_real - 40)
            {
                mapa[y][x] = (y == 0 || y == ALTURA_TELA - 1) ? '#' : ' ';
                continue;
            }

            // Se a coordenada X ultrapassar o fim do nível, limpa por segurança (Previne lixo na memória)
            if (x >= comp_real)
            {
                mapa[y][x] = ' ';
                continue;
            }

            // 3. BUFFER DE INÍCIO DO NÍVEL (Primeiros 15 blocos)
            if (x < 15)
            {
                mapa[y][x] = (y == 0 || y == ALTURA_TELA - 1) ? '#' : ' ';
                continue;
            }

            // 4. CALCULAR AS ONDAS DO TÚNEL (Válido de x = 15 até ao início da transição)
            double freq1 = (double)x / 8.0 + (double)seed;
            double freq2 = (double)x / 12.0 - (double)seed;
            int onda_topo = 2 + (int)(abs((int)(sin(freq1) * 3.5)));
            int onda_fundo = 2 + (int)(abs((int)(cos(freq2) * 3.5)));

            if (onda_topo + onda_fundo >= ALTURA_TELA - 2)
            {
                onda_topo = 4;
                onda_fundo = 4;
            }

            // Aplica as paredes de pedra do túnel
            if (y < onda_topo || y >= (ALTURA_TELA - onda_fundo))
            {
                mapa[y][x] = '#';
            }
            else
            {
                // 5. ZONA DE TRANSIÇÃO DINÂMICA (80 blocos antes da Arena do Boss)
                // O túnel continua a existir aqui, mas NÃO nascem obstáculos nem inimigos
                if (x >= comp_real - 80)
                {
                    mapa[y][x] = ' '; // Espaço interior do túnel fica vazio e seguro

                    // Injeta a Vida Extra garantida no meio da zona de transição deste nível
                    if (y == 7 && x == (comp_real - 60))
                    {
                        mapa[y][x] = '+';
                    }
                }
                // 6. ÁREA DE JOGO NORMAL DO TÚNEL
                else
                {
                    int chance_item = rand() % 100;
                    if (chance_item < 5)
                    {
                        mapa[y][x] = (nivel_atual == 0) ? 'O' : 'x';
                    }
                    else
                    {
                        mapa[y][x] = ' ';
                    }
                }
            }
        }
    }
}

void process_input(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, int upgrade_x)
{
    int ch;
    int last_move_ch = -1; // Guarda apenas a última tecla de movimento válida

    // Loop que esvazia o buffer de input do ncurses no frame atual
    while ((ch = getch()) != ERR)
    {
        // 1. Processamento imediato do disparo
        if (ch == tecla_fire)
        {
            for (int i = 0; i < MAX_TIROS; i++)
            {
                if (p->tiro[i].ativo == 0)
                {
                    p->tiro[i].x = p->x + offset + 1;
                    p->tiro[i].y = p->y;
                    p->tiro[i].ativo = 1;
                    break;
                }
            }
        }

        // 2. Atalhos de Debug / Sair
        if (ch == 'q' || ch == 'Q')
            p->x = -99;
        if (ch == 'n' || ch == 'N')
            p->x = -88;

        // 3. Filtro: Se a tecla for uma das tuas configurações de movimento, guarda-a
        if (ch == tecla_up || ch == tecla_down || ch == tecla_left || ch == tecla_right)
        {
            last_move_ch = ch;
        }
    }

    // 4. Armazena a posição pretendida para testar a colisão
    int prox_x = p->x;
    int prox_y = p->y;

    if ((last_move_ch == tecla_up) && p->y > 0)
        prox_y--;
    if ((last_move_ch == tecla_down) && p->y < ALTURA_TELA - 1)
        prox_y++;

    if (upgrade_x == 1)
    {
        // Direções corrigidas para corresponderem ao comportamento intuitivo
        if ((last_move_ch == tecla_left) && p->x > 1)
            prox_x--; // Mover para a esquerda diminui o X
        if ((last_move_ch == tecla_right) && p->x < LARGURA_TELA / 2)
            prox_x++; // Mover para a direita aumenta o X
    }

    // 5. Calcula as coordenadas globais da posição pretendida
    int mundo_prox_x = prox_x + offset;

    if (mundo_prox_x >= 0 && mundo_prox_x < LARGURA_MAPA)
    {
        char destino = mapa[prox_y][mundo_prox_x];

        // Se estiver imune, ignora as restrições de movimento das paredes
        if (p->imunidade > 0)
        {
            if (destino == '+')
            {
                if (p->vidas < 5)
                    p->vidas++;
                p->pontuacao += 500;
                mapa[prox_y][mundo_prox_x] = ' ';
            }
            p->x = prox_x;
            p->y = prox_y;
        }
        // Se NÃO estiver imune, valida o tipo de obstáculo
        else
        {
            if (destino == ' ' || destino == '+')
            {
                if (destino == '+')
                {
                    if (p->vidas < 5)
                        p->vidas++;
                    p->pontuacao += 500;
                    mapa[prox_y][mundo_prox_x] = ' ';
                    if (som_ativo)
                    {
                        system("play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &");
                    }
                }
                p->x = prox_x;
                p->y = prox_y;
            }
            else if (destino == '#' || destino == 'O' || destino == 'x')
            {
                p->x = -99; // Regista o impacto fatal
            }
        }
    }
}

void update_game(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, Boss *b, Projétil tiros_ini[MAX_TIROS_INIMIGO], int nivel_atual)
{
    if (p->imunidade > 0)
        p->imunidade--;

    // --- 1. INTELIGENCIA DOS INIMIGOS 'x' NO TUNEL ---
    if (nivel_atual >= 1 && b->ativo == 0)
    {
        static int inimigo_timer = 0;
        inimigo_timer++;

        if (inimigo_timer >= 3)
        {
            inimigo_timer = 0;

            for (int y = 1; y < ALTURA_TELA - 1; y++)
            {
                // CORREÇÃO CRÍTICA: Varrimento invertido impede o teletransporte e loops infinitos de IA
                for (int x = offset + LARGURA_TELA - 3; x >= offset + 2; x--)
                {
                    if (mapa[y][x] == 'x')
                    {
                        // Movimento seguro para a esquerda
                        if ((rand() % 100) < 20 && mapa[y][x - 1] == ' ')
                        {
                            mapa[y][x - 1] = 'x';
                            mapa[y][x] = ' ';
                        }

                        // O inimigo SÓ dispara se a casa à esquerda estiver VAZIA (' ')
                        if ((rand() % 100) < 3 && mapa[y][x - 1] == ' ')
                        {
                            for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                            {
                                if (tiros_ini[i].ativo == 0)
                                {
                                    tiros_ini[i].x = x - 1;
                                    tiros_ini[i].y = y;
                                    tiros_ini[i].ativo = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- 2. MOVIMENTO DOS TIROS DOS INIMIGOS ---
    for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
    {
        if (tiros_ini[i].ativo == 1) // Projétil de dano normal
        {
            tiros_ini[i].x--; // Viajam para a esquerda

            int tx = tiros_ini[i].x;
            int ty = tiros_ini[i].y;
            int ecra_tiro_x = tx - offset;

            if (ecra_tiro_x <= 0 || (tx < LARGURA_MAPA && mapa[ty][tx] == '#'))
            {
                tiros_ini[i].ativo = 0;
                continue;
            }

            if (p->imunidade <= 0 && ecra_tiro_x == p->x && ty == p->y)
            {
                tiros_ini[i].ativo = 0;
                p->x = -99;
            }
        }
        else if (tiros_ini[i].ativo == 2) // --- NOVO: ITEM DE CURA FLUTUANTE ---
        {
            int tx = tiros_ini[i].x;
            int ty = tiros_ini[i].y;
            int ecra_tiro_x = tx - offset;

            // Se por acaso o ecrã ultrapassar o item e ele sair pela esquerda
            if (ecra_tiro_x <= 0)
            {
                tiros_ini[i].ativo = 0;
                continue;
            }

            // Colisão: Se a nave do jogador passar por cima do item flutuante
            if (ecra_tiro_x == p->x && ty == p->y)
            {
                tiros_ini[i].ativo = 0; // Remove o item da arena
                if (p->vidas < MAX_VIDAS_JOGADOR)
                {
                    p->vidas++;
                }
                p->pontuacao += 500; // Recompensa extra por recolha arriscada
                if (som_ativo)
                {
                    system("play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &");
                }
            }
        }
    }

    // --- 3. DELEGAÇÃO DA IA DO BOSS (CÓDIGO TRANSFERIDO PARA O BOSS.C) ---
    if (b->ativo == 1)
    {
        IA_gerir_boss(b, p, offset, tiros_ini, nivel_atual, mapa);
    }

    // --- 4. ATUALIZACAO DOS TIROS DO JOGADOR + ANULAMENTO DE BALAS ---
    for (int i = 0; i < MAX_TIROS; i++)
    {
        if (p->tiro[i].ativo == 1)
        {
            p->tiro[i].x++;
            int tx = p->tiro[i].x;
            int ty = p->tiro[i].y;
            int ecra_tiro_x = tx - offset;

            if (ecra_tiro_x >= LARGURA_TELA - 1 || tx >= LARGURA_MAPA - 1)
            {
                p->tiro[i].ativo = 0;
                continue;
            }

            // ANULAMENTO DE PROJETEIS (Laser contra Laser)
            for (int j = 0; j < MAX_TIROS_INIMIGO; j++)
            {
                if (tiros_ini[j].ativo == 1 && tiros_ini[j].y == ty)
                {
                    if (tiros_ini[j].x == tx || tiros_ini[j].x == tx - 1)
                    {
                        p->tiro[i].ativo = 0;
                        tiros_ini[j].ativo = 0;
                        p->pontuacao += 5;
                        break;
                    }
                }
            }

            if (p->tiro[i].ativo == 0)
                continue;

            // --- HITBOX EVOLUÍDA DO BOSS (ADAPTÁVEL POR NÍVEL) ---
            if (b->ativo == 1)
            {
                int atingiu_boss = 0;

                // Todos os níveis partilham agora a mesma largura física justa (3 blocos)
                if (ecra_tiro_x >= b->x && ecra_tiro_x <= b->x + 2)
                {
                    // NÍVEL 1 E NÍVEL 2: Hitbox padrão global
                    if (nivel_atual == 0 || nivel_atual == 1)
                    {
                        if (ty >= b->y - 1 && ty <= b->y + 1)
                        {
                            atingiu_boss = 1;
                            b->vida--;
                            if (som_ativo)
                            {
                                (void)system("play -q -n synth 0.03 sine 1500 vol 0.20 > /dev/null 2>&1 &");
                            }
                        }
                    }
                    // NÍVEL 3: Mesma área 3x3, mas fracionada por componentes baseados na vida global (150 HP)
                    else if (nivel_atual == 2)
                    {
                        // FASE 1: DESARMAMENTO (Vida global entre 150 e 51)
                        if (b->fase_atual == 1)
                        {
                            // Braço Superior: Só recebe dano se a vida global for maior que 100
                            if (ty == b->y - 1 && b->vida > 100)
                            {
                                b->vida--;
                                p->tiro[i].ativo = 0;
                                p->pontuacao += 20;
                                if (som_ativo)
                                {
                                    (void)system("play -q -n synth 0.03 sine 1500 vol 0.20 > /dev/null 2>&1 &");
                                }
                                continue;
                            }
                            // Braço Inferior: Só recebe dano se a vida global for maior que 50
                            else if (ty == b->y + 1 && b->vida > 50)
                            {
                                b->vida--;
                                p->tiro[i].ativo = 0;
                                p->pontuacao += 20;
                                if (som_ativo)
                                {
                                    (void)system("play -q -n synth 0.03 sine 1500 vol 0.20 > /dev/null 2>&1 &");
                                }
                                continue;
                            }
                            // O núcleo central absorve o tiro sem sofrer dano enquanto os braços existirem
                            else if (ty == b->y)
                            {
                                p->tiro[i].ativo = 0;
                                continue;
                            }
                        }
                        // FASE 2: FÚRIA DO NÚCLEO CENTRAL (Vida global <= 50)
                        else if (b->fase_atual == 2)
                        {
                            // Na Fase 2, o ponto vulnerável é estritamente a linha central Y
                            if (ty == b->y && b->vida > 0)
                            {
                                b->vida--;
                                p->tiro[i].ativo = 0;
                                p->pontuacao += 50;
                                if (som_ativo)
                                {
                                    (void)system("play -q -n synth 0.03 sine 1800 vol 0.25 > /dev/null 2>&1 &");
                                }

                                if (b->vida <= 0)
                                {
                                    b->ativo = 2; // Vitória absoluta!
                                    p->pontuacao += 500000;
                                    if (som_ativo)
                                    {
                                        (void)system("play -q -n synth 0.80 brownnoise synth 0.80 sine 300:40 vol 0.45 > /dev/null 2>&1 &");
                                    }
                                }
                                continue;
                            }
                        }
                    }
                }

                // Finalização de dano exclusiva para os Níveis 1 e 2
                if (atingiu_boss && (nivel_atual == 0 || nivel_atual == 1))
                {
                    p->tiro[i].ativo = 0;
                    p->pontuacao += 10;
                    if (b->vida <= 0)
                    {
                        b->ativo = 2;
                        p->pontuacao += 100000;
                        if (som_ativo)
                        {
                            (void)system("play -q -n synth 0.80 brownnoise synth 0.80 sine 300:40 vol 0.45 > /dev/null 2>&1 &");
                        }
                    }
                    continue;
                }
            }

            // Colisões padrão com o mapa (Rochas 'O' ou Inimigos 'x')
            // --- COLISÕES PADRÃO COM O MAPA (ROCHAS 'O' OU INIMIGOS 'x') ---
            if (ty >= 0 && ty < ALTURA_TELA && tx >= 0 && tx < configuracao_niveis[nivel_atual].comprimento_mapa)
            {
                // CASO A: Destruição do Asteroide 'O'
                if (mapa[ty][tx] == 'O')
                {
                    mapa[ty][tx] = ' ';   // Limpa o asteroide do mapa
                    p->tiro[i].ativo = 0; // Apaga o laser do jogador
                    p->pontuacao += 50;   // Recompensa padrão

                    // SOM RETRO DE PEDRA A QUEBRAR (Curto e bastante grave)
                    if (som_ativo)
                    {
                        system("play -q -n synth 0.08 sine 180:60 vol 0.35 > /dev/null 2>&1 &");
                    }
                }
                // CASO B: Destruição da Nave Inimiga 'x'
                else if (mapa[ty][tx] == 'x')
                {
                    mapa[ty][tx] = ' ';   // Limpa o inimigo do mapa
                    p->tiro[i].ativo = 0; // Apaga o laser do jogador
                    p->pontuacao += 150;  // Maior recompensa (ameaça ativa)

                    // SOM RETRO DE EXPLOSÃO METÁLICA (Mais longo e descendente)
                    if (som_ativo)
                    {
                        system("play -q -n synth 0.15 sine 350:50 vol 0.40 > /dev/null 2>&1 &");
                    }
                }
                else if (mapa[ty][tx] == '#')
                {
                    if (b->ativo == 0 || ecra_tiro_x < b->x)
                    {
                        p->tiro[i].ativo = 0;
                    }
                }
            }
        }
    }
}
