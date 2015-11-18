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

#define PIPE_NAME TEXT("\\\\.\\pipe\\BombasMan")	// Nome do pipe
#define PIPE_NAME_LOGIN TEXT("\\\\.\\pipe\\BombasManLogin")	// Nome do pipe de Login
#define PIPE_NAME_JOGO TEXT("\\\\.\\pipe\\BombasManJogo")	// Nome do pipe de Jogo
#define PIPE_NAME_INIM TEXT("\\\\.\\pipe\\BombasManInim")	// Nome do pipe de Inimigos

#define LINHAS 13			// Linhas da matriz que representa o tabuleiro
#define COLUNAS 15			// Colunas da matriz que representa o tabuleiro
#define MAX_CLIENTE 100		// Numero máximo de clientes
#define MAX_PLAYERS 4		// Numero máximo de jogadores
#define MAX_BUFFER 256		// Tamanho do buffer para troca de mensagens
#define MAX_BOMBAS 2		// Numero máximo de bombas por player
#define MAX_POWER 5			// Power máximo que uma bomba pode ter
#define MAX_CHAVES 2		// Nro máximo de chaves por nível
#define MAX_CAIXAS 30
#define MAX_INIMIGOS 4
#define VIDAS	3			// Numero de vidas iniciais

#define ACTIVA 1			// Bomba está activa
#define NAO_ACTIVA 0		// Bomba não activa (nao estás colocada)
#define REBENTA 2			// Bomba vai rebentar

#define NIVEL1	1			// Nivel 1
#define NIVEL2	2			// Nivel 2
#define NIVEL3	3			// Nivel 3
#define NIVEL4	4			// Nivel 4
#define NIVEL5	5			// Nivel 5

// DEFINICOES DE ELEMENTOS DO JOGO
#define VAZIO 0
#define BONUSVIDA 3		// Vida extra
#define BONUSBOMBAS 4	// Bombas extra
#define BONUSNUKE 5		// Bomba nuclear
#define INIMIGO1 6		// Inimigo
#define INIMIGO2 60		// Inimigo 2
#define CHAVE 7			// Chave objectivo
#define PORTAL 8		// Portal de fim de nível
#define PAREDE 1
#define	CAIXA 2
#define	BOMBA 10
#define BOMBA_BUM 11
#define BOMBA_OFF 14
#define BLAST_CENTRO 12		// Rasto de Explosao
#define BLAST_DIR 13		// Rasto de Explosao 
#define BLAST_ESQ 14		// Rasto de Explosao 
#define BLAST_CIMA 15		// Rasto de Explosao 
#define BLAST_BAIXO 16		// Rasto de Explosao 
#define	PLAYER1 -1
#define PLAYER2	-2
#define PLAYER3	-3
#define PLAYER4	-4

// DEFINICOES DOS PEDIDOS
#define LOGIN 1
#define REGISTO 2
#define INICIARJOGO 3
#define LISTASERVIDORES 4
#define LIGARJOGO 5
#define HIGHSCORES 6
#define SAIR 0

// DEFINICOES DOS MOVIMENTOS
#define ESQUERDA 0
#define DIREITA 1
#define CIMA 2
#define BAIXO 3

HANDLE hPipe;						// Variavel global para guardar os Pipes de todos os Clientes
HANDLE hEventoChegou;				// Handle para sinalizar a ocorrencia de evento
HANDLE hThread = INVALID_HANDLE_VALUE;

int totalUtilizadoresReg = 0;		// Variavel global para numero total de utilizadores já registados
int tabuleiro[LINHAS][COLUNAS];		// Carregar a matriz do ficheiro
int numCliente = 0; // Para identificar cada Cliente
int nivel=1;
BOOL JOGO = TRUE;
BOOL NOVO = TRUE;

typedef struct
{
	int posX;	// Posição X das bombas no labirinto	
	int posY;	// Posição Y das bombas no labirinto
	int energia;// Define quantas vezes temos de rebentar a caixa para ela desaparecer
	BOOL status;// Define estado da caixa: TRUE=colocada; FALSE=não colocada
}
Caixas;

typedef struct
{
	int linha,coluna;
	BOOL status;
}
Chave;

typedef struct
{
	int posX;	// Posição X das bombas no labirinto	
	int posY;	// Posição Y das bombas no labirinto
	int status;// Define estado da bomba: TRUE=colocada; FALSE=não colocada
}
Bombas;

typedef struct
{
	int linha,coluna;
	BOOL status;
}
Portal;

typedef struct
{
	int posX;		// Posição X dos Players no labirinto
	int posY;		// Posição X dos Players no labirinto
	int vidas;		// Numero de Vidas restantes
	int countBombas;	// Numero de Bombas que o Player já colocou
	int tbombas;	// Nro de bombas que o player possui na mochila
	int Power;		// Nivel de Power que o player possui
	int countChaves; // Conta as chaves que apanhou
	Bombas bombasPlayer[MAX_BOMBAS]; // Bombas dos Players
	int pontos;
	TCHAR user[150];
}
Players;

typedef struct
{
	int linha,coluna;
	int id;
	int activo;
	int totInim;
}
Inimigos;

Inimigos *listInim[4];	// Lista de inimigos

typedef struct {
	int idBomba;
	int numC;
}*DATA;

typedef struct
{
	TCHAR user[150];
	TCHAR pass[150];
	BOOL loginStatus;	// TRUE: está logado; FALSE: não está logado
	int numJogos;		// Número de jogos ja jogados
	int maxPontuacao;	// Pontuação Máxima
}
Utilizadores;	// Utilizadores da aplicação a guardar no ficheiro

Utilizadores utilizadoresReg[100];	// Utilizadores registados

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
	TCHAR Nome[150];
}
Servidores;		// Estrutura para os servidores disponíveis

typedef struct
{
	TCHAR user[150];
	TCHAR pass[150];
}
Login;	// Receber logins de utilizadores

Login utilizador;

Players Player[MAX_PLAYERS];
Caixas Caixa[MAX_CAIXAS];
Chave Chaves[MAX_CHAVES];
Portal portal;

SECURITY_ATTRIBUTES sa;

int carregaUtilizadoresReg(Utilizadores *uti);
void criarSeguranca();
int verificaUserExiste(Login *user);
void guardaUserReg(Utilizadores *uti,int totUserReg);
int verificaExiste(Login *user);

void carregaTabuleiro();
void escreveTabuleiro();
void colocaPlayerTabuleiro(Players *Jogador, int numC);
int verificaPos(int lin, int col);
void colocaCaixasTabuleiro();
void movePlayer(int direccao, Players *Jogador, int numC);
void colocaBomba(Players *Jogador, int numC);
void actualizaTabuleiro(); //Funcao que actualiza o tabuleiro em cada acção
void rebentarBomba(int idBomba, int numC);
void bombaNuclear();
void criaInimigos();
void recolocaPlayer(Players *Jogador, int numC);
void colocaChavesTabuleiro();
void colocaPortal();
void regeneraInimigos();
void recarregaTabuleiro();

DWORD WINAPI timerBombas(LPVOID param);
DWORD WINAPI enviarHighscores(LPVOID param);
DWORD WINAPI processarPedidos(LPVOID param);
DWORD WINAPI processaPedido(LPVOID param);
DWORD WINAPI processarLogin(LPVOID param);
DWORD WINAPI processarRegisto(LPVOID param);
DWORD WINAPI processarCriarJogo(LPVOID param);
DWORD WINAPI iniciaJogo(LPVOID param);
DWORD WINAPI iaInimigos(LPVOID param);
DWORD WINAPI gereInimigos(LPVOID param);

HANDLE hPipeJogo[MAX_PLAYERS];
HANDLE hPipeInim;
HANDLE hEventoChegou;

TCHAR Inim1[]=TEXT("Inim1");
TCHAR Inim2[]=TEXT("Inim2");
TCHAR Inim3[]=TEXT("Inim3");
TCHAR Inim4[]=TEXT("Inim4");

Inimigos(*PtrInim1);
Inimigos(*PtrInim2);
Inimigos(*PtrInim3);
Inimigos(*PtrInim4);

HANDLE hMemInim1;
HANDLE hMemInim2;
HANDLE hMemInim3;
HANDLE hMemInim4;

#endif