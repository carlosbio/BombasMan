#ifndef _UTILS_H_
#define _UTILS_H_

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aclapi.h>
#include <time.h>
#include <tchar.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>

// DEFINICOES DE PIPES
#define PIPE_NAME TEXT("\\\\.\\pipe\\BombasMan")	// Nome do pipe
#define PIPE_NAME_LOGIN TEXT("\\\\.\\pipe\\BombasManLogin")	// Nome do pipe de Login
#define PIPE_NAME_JOGO TEXT("\\\\.\\pipe\\BombasManJogo")	// Nome do pipe de Login

// DEFINICOES DE ELEMENTOS DO JOGO
#define LINHAS 13			// Linhas da matriz que representa o tabuleiro
#define COLUNAS 15			// Colunas da matriz que representa o tabuleiro
#define MAX_CLIENTES 100	// Numero máximo de clientes
#define MAX_PLAYERS 4	// Numero máximo de clientes
#define MAX_BUFFER 256		// Tamanho maximo do buffer de troca de mensagens
#define VAZIO 0				// Celula Vazia
#define BONUSVIDA 3			// Vida extra
#define BONUSBOMBAS 4		// Bombas extra
#define BONUSNUKE 5			// Bomba nuclear

#define INIMIGO1 6			// Inimigo Burro
#define INIMIGO2 60			// Inimigo Esperto

#define CHAVE 7			// Chave objectivo
#define PORTAL 8		// Portal de fim de nível
#define PAREDE 1			// Parede
#define	CAIXA 2				// Caixa
#define	BOMBA 10			// Bomba
#define BOMBA_BUM 11		// Bomba Rebenta
#define BLAST_CENTRO 12		// Rasto de Explosao
#define BLAST_DIR 13		// Rasto de Explosao 
#define BLAST_ESQ 14		// Rasto de Explosao 
#define BLAST_CIMA 15		// Rasto de Explosao 
#define BLAST_BAIXO 16		// Rasto de Explosao 
#define	PLAYER1 -1
#define PLAYER2	-2
#define PLAYER3	-3
#define PLAYER4	-4
#define LARGURA 50
#define ALTURA 62

// DEFINICOES PARA DIRECCAO DO PLAYER
#define BAIXO 0
#define ESQUERDA 1
#define DIREITA 2
#define CIMA 3

// DEFINICOES PARA ESTADO DO JOGO
#define HOME 0				// Está no ecrã inicial
#define LOGIN 1				// Está no sign in
#define REGISTAR 2			// Está no registo
#define JOGAR 3				// Está a jogar
#define HIGH 4				// Está nos HIGHSCORES
#define GAMEOVER 5			// Mensagem GameOver
#define ERROMAXPLAYERS -1   // Mensagem Max Jogodares
#define CREDITOS 6			// Ecrã de Créditos
#define TABELASCORES 7		// Ecra de Highscores
#define FINNISH	8			// Ecrã de Fim de Jogo

// DEFINICOES DOS PEDIDOS
#define LOGIN 1
#define REGISTO 2
#define INICIARJOGO 3
#define LISTASERVIDORES 4
#define LIGARJOGO 5
#define HIGHSCORES 6
#define SAIR 0

TCHAR nomePipeIni[MAX_BUFFER];
TCHAR nomePipeJogo[MAX_BUFFER];

int dirPlayer = BAIXO;
int dirPlayer2 = BAIXO;
int dirPLayer3 = BAIXO;
int dirPlayer4 = BAIXO;
BOOL perdeuVida = FALSE;
TCHAR pontos[10];
TCHAR auxpontos[10];
TCHAR vidas[10];
TCHAR bombas[10];
TCHAR morte[10];
TCHAR nivel[10];
TCHAR countChaves[10];
TCHAR fim[10];
TCHAR user[50];
TCHAR pass[50];

int tabuleiro[LINHAS][COLUNAS];		// Carregar a matriz do ficheiro

int PlayerX, PlayerY;
int gameSTATUS = 0, menuOP = 0, loginOP = 0;

int x = 0;
TCHAR opcao[MAX_BUFFER];	// Buffer de comunicacao
int tecla;
int pedido;
HANDLE hPipe, hPipeJogo;	// Handle do pipe
BOOL fSucesso = FALSE;		// Guarda o return da função ReadFile (true=sucesso)
DWORD nBytesRead;		// Guarda o nº de bytes recebidos
DWORD nBytesWrite;		// Guarda o nº de bytes enviados
BOOL joga = FALSE;		// Flag para sinalizar se o servidor autoriza ou não o cliente a jogar 
BOOL TERMINAR = FALSE;		// Flag para sinalizar se o servidor autoriza ou não o cliente a jogar 
int Highscore = 0;

int totUserReg=0;

typedef struct
{
	TCHAR username[150];
	TCHAR password[150];
	int numJogos;
	int	maxPontuacao;
	int loginStatus;
}
Utilizadores;	// Utilizadores da aplicação a guardar no ficheiro

Utilizadores userReg[100];

typedef struct
{
	TCHAR user[150];	// Nome de utilizador
	TCHAR numJogos[10];		// Número de jogos ja jogados
	TCHAR maxPontuacao[10];	// Pontuação Máxima
}
estatistica;

estatistica utilizadoresHighscores[100];

typedef struct
{
	TCHAR user[150];
	TCHAR pass[150];
}
Login;	// Receber logins de utilizadores

Login utilizador;

typedef struct
{
	TCHAR Nome[150];
}
Servers;					

SECURITY_ATTRIBUTES sa;

void escreveNum(TCHAR num[10], int x, int y);
DWORD WINAPI actualizaJogo(LPVOID param);
#endif