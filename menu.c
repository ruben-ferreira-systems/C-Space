#include "menu.h"
#include "space.h"
#include "draw.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Grava o progresso atual num ficheiro binário local
void guardar_jogo(int nivel, int vidas, long pontuacao)
{
    FILE *f = fopen("save.dat", "wb");
    if (f == NULL)
        return;

    SaveData data = {nivel, vidas, pontuacao};
    fwrite(&data, sizeof(SaveData), 1, f);
    fclose(f);
}

// Tenta ler o progresso do ficheiro binário local
int carregar_jogo(Player *p, int *nivel_alvo)
{
    FILE *f = fopen("save.dat", "rb");
    if (f == NULL)
        return 0; // Não existe gravação

    SaveData data;
    fread(&data, sizeof(SaveData), 1, f);
    fclose(f);

    // Injeta os dados lidos diretamente nas referências do motor
    *nivel_alvo = data.nivel;
    p->vidas = data.vidas;
    p->pontuacao = data.pontuacao;
    p->proximo_patamar_pontos = ((p->pontuacao / 10000) + 1) * 10000;
    return 1; // Sucesso
}

int executar_menu_principal(Player *p, int *nivel_alvo)
{
    int selecao = OPCAO_PLAY;
    int ch;

    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);

    while (1)
    {
        clear();
        int centro_x = LARGURA_TELA / 2;
        int centro_y = ALTURA_TELA / 2;

        mvprintw(centro_y - 5, centro_x - 7, "SPACE GAME C");
        mvprintw(centro_y - 4, centro_x - 10, "====================");

        // Opções traduzidas para Inglês
        mvprintw(centro_y - 2, centro_x - 8, "%s PLAY", (selecao == OPCAO_PLAY) ? "-> " : "   ");
        mvprintw(centro_y - 1, centro_x - 8, "%s LOAD GAME", (selecao == OPCAO_LOAD) ? "-> " : "   ");
        mvprintw(centro_y, centro_x - 8, "%s CONTROLS", (selecao == OPCAO_COMANDOS) ? "-> " : "   ");
        mvprintw(centro_y + 1, centro_x - 8, "%s CREDITS", (selecao == OPCAO_CREDITOS) ? "-> " : "   ");
        mvprintw(centro_y + 2, centro_x - 8, "%s HIGH SCORES", (selecao == OPCAO_MAX_SCORES) ? "-> " : "   ");
        mvprintw(centro_y + 3, centro_x - 8, "%s EXIT", (selecao == OPCAO_SAIR) ? "-> " : "   ");

        refresh();
        ch = getch();

        // Navegação adaptada também para W e S no menu, além das Setas
        if ((ch == KEY_UP || ch == 'w' || ch == 'W') && selecao > OPCAO_PLAY)
            selecao--;
        if ((ch == KEY_DOWN || ch == 's' || ch == 'S') && selecao < OPCAO_SAIR)
            selecao++;

        if (ch == 10 || ch == '\n')
        {
            if (selecao == OPCAO_SAIR)
                return 0;

            if (selecao == OPCAO_PLAY)
            {
                *nivel_alvo = 0;
                p->vidas = 3;
                p->pontuacao = 0;
                p->proximo_patamar_pontos = 10000;

                animacao_intro_binaria(); 

                return 1;
            }

            if (selecao == OPCAO_LOAD)
            {
                if (carregar_jogo(p, nivel_alvo))
                {
                    return 1;
                }
                else
                {
                    mvprintw(centro_y + 5, centro_x - 10, "No Save Found!");
                    refresh();
                    usleep(1000000);
                }
            }

            if (selecao == OPCAO_COMANDOS) {
                int config_selecao = 0;
                
                // Força o teclado a esperar pela tecla do utilizador de forma estrita
                nodelay(stdscr, FALSE);
                keypad(stdscr, TRUE);
                clear(); // Limpa o menu principal antes de desenhar o submenu

                while (1) {
                    // Limpeza interna controlada para o desenho das teclas
                    clear();
                    int cx = LARGURA_TELA / 2;
                    int cy = ALTURA_TELA / 2;

                    mvprintw(cy - 5, cx - 9, "SETTINGS & CONTROLS");
                    mvprintw(cy - 4, cx - 11, "======================");

                    // Opções Interativas Textuais
                    mvprintw(cy - 2, cx - 12, "%s SOUND: [%s]",    (config_selecao == 0) ? "-> " : "   ", som_ativo ? "ON " : "OFF");
                    mvprintw(cy - 1, cx - 12, "%s MOVE UP: [%c]",   (config_selecao == 1) ? "-> " : "   ", tecla_up);
                    mvprintw(cy,     cx - 12, "%s MOVE DOWN: [%c]", (config_selecao == 2) ? "-> " : "   ", tecla_down);
                    mvprintw(cy + 1, cx - 12, "%s MOVE LEFT: [%c]", (config_selecao == 3) ? "-> " : "   ", tecla_left);
                    mvprintw(cy + 2, cx - 12, "%s MOVE RIGHT: [%c]",(config_selecao == 4) ? "-> " : "   ", tecla_right);
                    mvprintw(cy + 3, cx - 12, "%s FIRE: [%c]",      (config_selecao == 5) ? "-> " : "   ", (tecla_fire == ' ') ? 'S' : tecla_fire); // Se for espaço mostra S de Space
                    mvprintw(cy + 5, cx - 10, "Press ENTER to Modify / Q to Exit");

                    refresh();
                    int c_ch = getch();

                    // Navegação estável pelas opções com W/S e Setas
                    if (c_ch == KEY_UP || c_ch == 'w' || c_ch == 'W') { 
                        if (config_selecao > 0) config_selecao--; 
                    }
                    if (c_ch == KEY_DOWN || c_ch == 's' || c_ch == 'S') { 
                        if (config_selecao < 5) config_selecao++; 
                    }
                    
                    // Se carregar em Q, limpa o ecrã por completo e sai
                    if (c_ch == 'q' || c_ch == 'Q') {
                        clear();
                        refresh();
                        break; 
                    }

                    if (c_ch == 10 || c_ch == '\n') {
                        if (config_selecao == 0) {
                            som_ativo = !som_ativo; 
                        } 
                        else if (config_selecao >= 1 && config_selecao <= 4) {
                            clear();
                            mvprintw(cy, cx - 12, "PRESS THE NEW KEY...");
                            refresh();
                            int nova_tecla = getch();
                            
                            if (nova_tecla >= 32 && nova_tecla <= 126) {
                                if (config_selecao == 1) tecla_up    = nova_tecla;
                                if (config_selecao == 2) tecla_down  = nova_tecla;
                                if (config_selecao == 3) tecla_left  = nova_tecla;
                                if (config_selecao == 4) tecla_right = nova_tecla;
                            }
                        }
                        else if (config_selecao == 5) {
                            clear();
                            mvprintw(cy, cx - 12, "PRESS NEW FIRE KEY...");
                            refresh();
                            int nova_tecla = getch();
                            if (nova_tecla >= 32 && nova_tecla <= 126) {
                                tecla_fire = nova_tecla;
                            }
                        }
                    }
                }
            }


            if (selecao == OPCAO_CREDITOS)
            {
                clear();
                mvprintw(centro_y - 1, centro_x - 12, "Developed in IEFP Course");
                mvprintw(centro_y, centro_x - 6, "Final Lab C");
                mvprintw(centro_y + 2, centro_x - 14, "Press any key to return...");
                refresh();
                getch();
            }

            if (selecao == OPCAO_MAX_SCORES)
            {
                carregar_e_desenhar_highscores();
            }
        }
    }
}
// Função para desenhar a tabela de recordes no Menu Principal
void carregar_e_desenhar_highscores()
{
    int centro_x = LARGURA_TELA / 2;
    int centro_y = ALTURA_TELA / 2;
    clear();

    FILE *f = fopen("highscores.txt", "r");

    // Cria a lista dinâmica preenchida por defeito
    HighScore ranking[MAX_LIST];
    for (int i = 0; i < MAX_LIST; i++)
    {
        strcpy(ranking[i].nome, "EMPTY");
        ranking[i].score = 0;
    }

    // Se o ficheiro de texto existir, lê até ao limite da lista
    if (f != NULL)
    {
        for (int i = 0; i < MAX_LIST; i++)
        {
            // O espaço inicial " %s %ld" limpa automaticamente quebras de linha vazias no ficheiro
            if (fscanf(f, " %7s %ld", ranking[i].nome, &ranking[i].score) != 2)
            {
                break;
            }
        }
        fclose(f);
    }

    mvprintw(centro_y - 5, centro_x - 8, "GALACTIC RANKING");
    mvprintw(centro_y - 4, centro_x - 11, "======================");

    // Desenha o ranking adaptando-se ao tamanho definido na macro
    for (int i = 0; i < MAX_LIST; i++)
    {
        mvprintw(centro_y - 2 + i, centro_x - 12, "%d. %-7s : %07ld", i + 1, ranking[i].nome, ranking[i].score);
    }

    mvprintw(centro_y + MAX_LIST + 1, centro_x - 14, "Press any key to return...");
    refresh();
    getch();
}
void verificar_e_gravar_highscore(long pontuacao_final)
{
    if (pontuacao_final <= 0)
        return;

    FILE *f = fopen("highscores.txt", "r");

    // Inicializa a tabela em memória
    HighScore ranking[MAX_LIST];
    for (int i = 0; i < MAX_LIST; i++)
    {
        strcpy(ranking[i].nome, "EMPTY");
        ranking[i].score = 0;
    }

    if (f != NULL)
    {
        for (int i = 0; i < MAX_LIST; i++)
        {
            if (fscanf(f, "%7s %ld", ranking[i].nome, &ranking[i].score) == EOF)
                break;
        }
        fclose(f);
    }

    // Encontra a posição correta do jogador no ranking
    int posicao_inserir = -1;
    for (int i = 0; i < MAX_LIST; i++)
    {
        if (pontuacao_final > ranking[i].score)
        {
            posicao_inserir = i;
            break;
        }
    }

    // --- SE CONQUISTOU UM LUGAR NO TOP 3: ATIVA A CELEBRAÇÃO ---
    if (posicao_inserir != -1)
    {
        nodelay(stdscr, FALSE);

        int centro_x = LARGURA_TELA / 2;
        int centro_y = ALTURA_TELA / 2;

        system("play -q -n synth 0.08 sine 523 synth 0.08 sine 659 synth 0.08 sine 784 synth 0.18 sine 1046 vol 0.35 > /dev/null 2>&1 &");

        // --- NOVO: ANIMAÇÃO DE PISCAR INTERMITENTE (ARCADE CELEBRATION) ---
        // Pisca o ecrã 4 vezes alternando entre mensagens em destaque e molduras
        for (int pisca = 0; pisca < 4; pisca++)
        {
            clear();
            // Frame A: Desenha a moldura de estrelas festiva
            for (int i = 0; i < LARGURA_TELA; i++)
            {
                mvaddch(centro_y - 3, i, '*');
                mvaddch(centro_y + 3, i, '*');
            }
            mvprintw(centro_y - 1, centro_x - 10, "!!! NEW HIGH SCORE !!!");
            mvprintw(centro_y + 1, centro_x - 11, "RANK %d SPOT CONQUERED!", posicao_inserir + 1);
            refresh();
            usleep(250000); // Fica aceso por 250ms

            clear();
            refresh();
            usleep(150000); // Fica apagado por 150ms (Gera o efeito estroboscópico)
        }

        // --- CAPTURA DO NOME DO PILOTO ---
        echo();
        curs_set(1); // Mostra o cursor para digitação

        clear();
        // Desenha uma moldura fixa minimalista para o input
        for (int i = 0; i < LARGURA_TELA; i++)
        {
            mvaddch(centro_y - 2, i, '=');
            mvaddch(centro_y + 2, i, '=');
        }
        mvprintw(centro_y - 1, centro_x - 10, " ENTER YOUR INITIALS ");
        mvprintw(centro_y, centro_x - 13, "NAME (MAX 7 CHARS): ");
        refresh();

        char novo_nome[8] = "";
        getnstr(novo_nome, 7);

        if (strlen(novo_nome) == 0)
            strcpy(novo_nome, "PILOT");

        noecho();
        curs_set(0); // Esconde o cursor novamente

        // Empurra os recordes inferiores para baixo com segurança
        for (int i = MAX_LIST - 1; i > posicao_inserir; i--)
        {
            ranking[i] = ranking[i - 1];
        }

        // Grava os novos dados na posição conquistada
        strcpy(ranking[posicao_inserir].nome, novo_nome);
        ranking[posicao_inserir].score = pontuacao_final;

        // Guarda a lista atualizada de volta no disco
        f = fopen("highscores.txt", "w");
        if (f != NULL)
        {
            for (int i = 0; i < MAX_LIST; i++)
            {
                fprintf(f, "%s %ld\n", ranking[i].nome, ranking[i].score);
            }
            fclose(f);
        }
    }
}
