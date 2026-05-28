#include "space.h"
#include "levels.h"
#include "boss.h"
#include "draw.h"
#include "menu.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main()
{

    char mapa[ALTURA_TELA][LARGURA_MAPA];
    Projétil tiros_inimigos[MAX_TIROS_INIMIGO];
    Player jogador;
    Boss boss;
    unsigned int sementes_niveis[MAX_NIVEIS] = {12345, 98765, 42424, 86420}; 
    int nivel_atual = 0;

    init_game();

    while (1)
    {
        int proximo_nivel_carregar = 0;

        // NOVO: Abre o Menu interativo. Se devolver 0, fecha o programa
        if (executar_menu_principal(&jogador, &proximo_nivel_carregar) == 0)
        {
            break;
        }

        // Configura as variáveis globais com base na escolha (Jogo Novo ou Load)
        nivel_atual = proximo_nivel_carregar;
        // Remova a atribuição fixa "jogador.vidas = 3;" e "jogador.pontuacao = 0;" daqui
        // para não esmagar os dados do Load!

        int resultado = 0;
        int upgrade_x = 0;

        while (resultado == 0)
        {

            if (nivel_atual >= 1)
                upgrade_x = 1;
            else
                upgrade_x = 0;

            jogador.x = 2;
            jogador.y = 7;
            int offset = 0;
            int frame_count = 0;
            int fim_do_nivel = 0;

            boss.x = LARGURA_TELA - 4;
            boss.y = 7;
            boss.vida = BOSS_VIDA_MAX;
            boss.ativo = 0;
            boss.direcao_y = 1;
            boss.direcao_x = -1;

            jogador.proximo_patamar_pontos = 10000; 


            // Chamar o reset global do Boss ao iniciar/reiniciar qualquer nível
            reset_boss_ia();

            for (int i = 0; i < MAX_TIROS; i++)
            {
                jogador.tiro[i].ativo = 0;
            }

            for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
            {
                tiros_inimigos[i].ativo = 0;
            }

            nodelay(stdscr, TRUE);
            gerar_mapa_dinamico(mapa, sementes_niveis[nivel_atual], nivel_atual);

            clear();
            refresh();
            flushinp();
            frame_count = 0;

            // Loop de jogabilidade do nível atual
            while (resultado == 0 && fim_do_nivel == 0)
            {
                process_input(&jogador, mapa, offset, upgrade_x);

                // Batota / Debug: Saltar nível dinamicamente
                if (jogador.x == -88)
                {
                    jogador.x = 2;
                    offset = configuracao_niveis[nivel_atual].comprimento_mapa - LARGURA_TELA;
                    flushinp();
                }

                // Sair do jogo voluntariamente
                if (jogador.x == -99 && jogador.vidas <= 0)
                {
                    endwin();
                    return 0;
                }



                // DETEÇÃO DE DANOS ATIVOS (Colisão ativa ou projétil inimigo)
                if (jogador.x == -99)
                {
                    if (jogador.imunidade <= 0)
                    {
                        system("play -q -n synth 0.06 sine 220:110 synth 0.06 sine 220:110 vol 0.4 > /dev/null 2>&1 &");
                        jogador.vidas--;
                        if (jogador.vidas <= 0)
                        {
                            resultado = 2; // Game Over definitivo
                        }
                        else
                        {
                            jogador.x = 2;
                            jogador.y = 7;
                            jogador.imunidade = 66; // 2 segundos de imunidade

                            for (int i = 0; i < MAX_TIROS; i++)
                                jogador.tiro[i].ativo = 0;
                            for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                                tiros_inimigos[i].ativo = 0;
                            flushinp();
                        }
                    }
                    else
                    {
                        jogador.x = 2; // Segurança se estava imune
                    }
                }

                //SISTEMA DE RECOMPENSA FÍSICA (FATOR RISCO)
                if (jogador.pontuacao >= jogador.proximo_patamar_pontos)
                {
                    // Faz o item surgir 18 blocos à frente do jogador na mesma linha Y
                    int spawn_mundo_x = offset + jogador.x + 18;

                    // Proteção rigorosa contra transbordo de memória da matriz do mapa
                    if (spawn_mundo_x < LARGURA_MAPA)
                    {
                        // Se o caminho estiver livre, injeta o '+' na linha atual da nave
                        if (mapa[jogador.y][spawn_mundo_x] == ' ')
                        {
                            mapa[jogador.y][spawn_mundo_x] = '+';
                            // Sinal sonoro rápido de alerta (o item apareceu no mapa!)
                            system("play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &");
                        }
                        else
                        {
                            // Se a rota direta tiver uma parede, tenta injetar nas linhas adjacentes
                            if (mapa[jogador.y - 1][spawn_mundo_x] == ' ') 
                            {
                                mapa[jogador.y - 1][spawn_mundo_x] = '+';
                                system("play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &");
                            }
                            else if (mapa[jogador.y + 1][spawn_mundo_x] == ' ') 
                            {
                                mapa[jogador.y + 1][spawn_mundo_x] = '+';
                                system("play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &");
                            }
                        }
                    }

                    // Define o próximo patamar (+10.000 pontos à frente)
                    jogador.proximo_patamar_pontos += 10000;
                }

                update_game(&jogador, mapa, offset, &boss, tiros_inimigos, nivel_atual);
                draw_game(&jogador, mapa, offset, nivel_atual, &boss, tiros_inimigos);

                // Arena do Boss (Gatilho corrigido com base no tamanho dinâmico do nível)
                if (offset >= configuracao_niveis[nivel_atual].comprimento_mapa - LARGURA_TELA)
                {
                    if (boss.ativo == 0)
                        {
                            system("play -q -n synth 0.12 square 440 : synth 0.12 square 440 : synth 0.12 square 440 : synth 0.35 square 280 vol 0.30 > /dev/null 2>&1 &");
                            boss.ativo = 1;
                        }


                    // Hitbox retangular padrão do Boss contra o corpo do jogador
                    if (boss.ativo == 1 && jogador.imunidade <= 0 && jogador.x >= boss.x && jogador.x <= boss.x + 2)
                    {
                        if (jogador.y >= boss.y - 1 && jogador.y <= boss.y + 1)
                        {
                            system("play -q -n synth 0.08 sine 200:150 synth 0.08 sine 200:150 vol 0.4 > /dev/null 2>&1 &");
                            jogador.vidas--;
                            if (jogador.vidas <= 0)
                            {
                                resultado = 2;
                            }
                            else
                            {
                                jogador.x = 2;
                                jogador.y = 7;
                                jogador.imunidade = 66;
                                flushinp();
                            }
                        }
                    }
                    if (boss.ativo == 2)
                        fim_do_nivel = 1;
                }

                // SISTEMA DE SCROLL E COLISÃO PASSIVA
                frame_count++;
                if (frame_count >= 6)
                {
                    frame_count = 0;
                    if (offset < configuracao_niveis[nivel_atual].comprimento_mapa - LARGURA_TELA)
                    {
                        offset++;
                        int mundo_x = jogador.x + offset;

                        if (mundo_x < LARGURA_MAPA)
                        {
                            // RECOLHA PASSIVA DE VIDA EXTRA COM O SCROLL
                            if (mapa[jogador.y][mundo_x] == '+')
                            {
                                if (jogador.vidas < MAX_VIDAS_JOGADOR)
                                {
                                    jogador.vidas++;
                                }
                                jogador.pontuacao += 500;
                                mapa[jogador.y][mundo_x] = ' ';
                                system("play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &");
                            }
                            // COLISÕES PREJUDICIAIS COM O CENÁRIO
                            else if (jogador.imunidade <= 0 && (mapa[jogador.y][mundo_x] == '#' || mapa[jogador.y][mundo_x] == 'O' || mapa[jogador.y][mundo_x] == 'x'))
                            {
                                system("play -q -n synth 0.06 sine 220:110 synth 0.06 sine 220:110 vol 0.4 > /dev/null 2>&1 &");
                                jogador.vidas--;
                                if (jogador.vidas <= 0)
                                {
                                    resultado = 2;
                                }
                                else
                                {
                                    jogador.x = 2;
                                    jogador.y = 7;
                                    jogador.imunidade = 66;

                                    for (int i = 0; i < MAX_TIROS; i++)
                                        jogador.tiro[i].ativo = 0;
                                    for (int i = 0; i < MAX_TIROS_INIMIGO; i++)
                                        tiros_inimigos[i].ativo = 0;
                                    flushinp();
                                }
                            }
                        }
                    }
                }

                usleep(DELAY - (nivel_atual * 3000));
            } // Fim do loop do nível atual

            if (fim_do_nivel == 1 && resultado == 0)
            {
                if (nivel_atual < MAX_NIVEIS - 1)
                {
                    draw_transition(nivel_atual);
                }
                guardar_jogo(nivel_atual + 1, jogador.vidas, jogador.pontuacao);
                nivel_atual++;
                if (nivel_atual >= MAX_NIVEIS)
                    resultado = 1;
            }
        } // Fim do loop "while (resultado == 0)"
        
        if (jogador.pontuacao > 0)
        {
           verificar_e_gravar_highscore(jogador.pontuacao);
        }

        // --- ECRÃ FINAL INTERATIVO ---
        nodelay(stdscr, FALSE);
        keypad(stdscr, TRUE);
        flushinp();
        clear();

        for (int i = 0; i < LARGURA_TELA; i++)
        {
            mvaddch(ALTURA_TELA / 2 - 2, i, '=');
            mvaddch(ALTURA_TELA / 2 + 2, i, '=');
        }
        if (resultado == 1)
        {
            mvprintw(ALTURA_TELA / 2 - 1, (LARGURA_TELA / 2) - 7, "TOTAL VICTORY!");
        }
        else if (resultado == 2)
        {
            mvprintw(ALTURA_TELA / 2 - 1, (LARGURA_TELA / 2) - 7, "YOU WERE CRUSHED!");
        }
        mvprintw(ALTURA_TELA / 2, (LARGURA_TELA / 2) - 9, "FINAL SCORE: %ld", jogador.pontuacao);

        mvprintw(ALTURA_TELA / 2 + 4, (LARGURA_TELA / 2) - 12, "Press J to Play Again");
        mvprintw(ALTURA_TELA / 2 + 5, (LARGURA_TELA / 2) - 8, "Press Q to Exit");
        refresh();

        int escolha = 0;
        int fechar_programa = 0;
        while (1)
        {
            escolha = getch();
            if (escolha == 'j' || escolha == 'J')
                break;
            if (escolha == 'q' || escolha == 'Q')
            {
                fechar_programa = 1;
                break;
            }
        }
        if (fechar_programa)
            break;
    }

    endwin();
    return 0;
}
