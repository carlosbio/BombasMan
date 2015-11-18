#include "utils.h"

int _tmain(int argc,TCHAR *argv[])
{
	HANDLE hThread[MAX_CLIENTE];
	int nClientes = 0;
    BOOL ret=FALSE;
	int i=0,j=0;

	#ifdef UNICODE
		_setmode(_fileno(stdin),_O_WTEXT);
		_setmode(_fileno(stdout),_O_WTEXT);
		_setmode(_fileno(stderr),_O_WTEXT);
	#endif

	totalUtilizadoresReg = carregaUtilizadoresReg(utilizadoresReg);	// Carrega os users registados (a partir do ficheiro binário)
	criarSeguranca();	// Regras de segurança


	while (nClientes < MAX_CLIENTE){
		_tprintf(TEXT("\n[SERVER]: Vou criar um NPIPE: %s\n"), PIPE_NAME);
		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAX_CLIENTE, MAX_BUFFER*sizeof(TCHAR), MAX_BUFFER*sizeof(TCHAR), 1000, &sa);
		if (hPipe == INVALID_HANDLE_VALUE){
			perror("Erro na criação do pipe!");
			exit(-1);
		}

		_tprintf(TEXT("[SERVIDOR] Esperar ligacao de um cliente... (ConnectNamedPipe)\n"));
		if (!ConnectNamedPipe(hPipe, NULL)){
			perror("Erro na ligação ao cliente!");
			exit(-1);
		}

		hThread[nClientes] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)processarPedidos, (LPVOID)hPipe, 0, NULL);	
		nClientes++;
	}

	WaitForMultipleObjects(MAX_CLIENTE, hThread, FALSE, INFINITE);
	//WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hPipe);
	Sleep(2000);
	exit(0);
}

int carregaUtilizadoresReg(Utilizadores *uti)	// Função que carrega os users registados
{
	FILE *f;
	int total,i;

	f = _tfopen(TEXT("regUtilizadores.dat"),TEXT("rb"));	// Abre o ficheiro binário em modo de leitura
	
	if(f==NULL)	// Verifica erros de abertura do ficheiro
	{
		_tprintf(TEXT("ERRO ao abrir Utilizadores.dat\n"));
		return 0;
	}

	fread(&total,sizeof(total),1,f);			// Lê do ficheiro e guarda o valor total de utilizadores como inteiro
	fread(uti,sizeof(Utilizadores),total,f);	// Lê do ficheiro e guarda os dados dos utilizadores como estrutura

	for(i=0;i<total;i++)
		uti[i].loginStatus = FALSE; // Flag de admin não logado
	
	fclose(f);	// Fecha o ficheiro

	return total;	// Devolve o valor total de users registados
}

void carregaTabuleiro()	// Função que carrega o tabuleiro
{
	FILE *f;
	int i,j,valor=-1;

	f=_tfopen(TEXT("matriz.txt"),TEXT("r"));
	
	if(f!=NULL)
	{
		for(i=0;i<LINHAS;i++)

		for (j = 0; j < COLUNAS; j++)
			_ftscanf(f, TEXT("%d"), &tabuleiro[i][j]);

		fclose(f);
	}

	colocaChavesTabuleiro();	// Coloca as chaves no tabuleiro
	colocaCaixasTabuleiro();	// Coloca as caixas no tabuleiro
	criaInimigos();				// Coloca os inimigos no tabuleiro
}

void recarregaTabuleiro()	// Função que recarrega o tabuleiro
{
	FILE *f;
	int i,j,valor=-1;

	f=_tfopen(TEXT("matriz.txt"),TEXT("r"));
	
	if(f!=NULL)
	{
		for(i=0;i<LINHAS;i++)

		for (j = 0; j < COLUNAS; j++)
			_ftscanf(f, TEXT("%d"), &tabuleiro[i][j]);

		fclose(f);
	}

	colocaChavesTabuleiro();	// Coloca as chaves no tabuleiro
	colocaCaixasTabuleiro();	// Coloca as caixas no tabuleiro
	regeneraInimigos();			// Coloca os inimigos no tabuleiro

	for(int i=0;i<MAX_CAIXAS;i++)
		Caixa[i].energia++;
}

void criarSeguranca()	// Função que atribui definições de segurança (MODELO)
{	
	TCHAR str[256];
	PSECURITY_DESCRIPTOR pSD;
	PACL pAcl;
	EXPLICIT_ACCESS ea;
	PSID pEveryoneSID=NULL,pAdminSID=NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld=SECURITY_WORLD_SID_AUTHORITY;

	pSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);

	if(pSD==NULL) 
		return;
	
	if(!InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION))
		return;

	if(!AllocateAndInitializeSid(&SIDAuthWorld,1,SECURITY_WORLD_RID,0,0,0,0,0,0,0,&pEveryoneSID))
		_stprintf_s(str,256,TEXT("AllocateAndInitializeSid() error %u"),GetLastError());

	ZeroMemory(&ea,sizeof(EXPLICIT_ACCESS));

	ea.grfAccessPermissions=/*GENERIC_ALL*/GENERIC_WRITE|GENERIC_READ;
	ea.grfAccessMode=SET_ACCESS;
	ea.grfInheritance=SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm=TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType=TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName=(LPTSTR)pEveryoneSID;

	if (SetEntriesInAcl(1,&ea,NULL,&pAcl)!=ERROR_SUCCESS)
		return;

	if (!SetSecurityDescriptorDacl(pSD,TRUE,pAcl,FALSE))
		return;

	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=pSD;
	sa.bInheritHandle=TRUE;
}

int verificaUserExiste(Login *utilizador)	// Função que verifica se o user já efectuou o login
{ 
	int i=0;

	for(i=0;i<totalUtilizadoresReg;i++)
		if (_tcsncmp(utilizador->user, utilizadoresReg[i].user, 150) == 0 && _tcsncmp(utilizador->pass, utilizadoresReg[i].pass, 150) == 0)
			if(utilizadoresReg[i].loginStatus==FALSE)
			{
				utilizadoresReg[i].loginStatus=TRUE;
				return 1;			
			}
	return 0;
}

void guardaUserReg(Utilizadores *uti,int totalutilizadoresReg)	// Função responsável por guardar um registo de users
{ 
	FILE *f; 

	f=_tfopen(TEXT("regUtilizadores.dat"),TEXT("wb")); 

	if(f==NULL)
	{ 
		_tprintf(TEXT("Erro no acesso ao ficheiro\n")); 
		return; 
	} 

	fwrite(&totalutilizadoresReg, sizeof(totalutilizadoresReg), 1, f);
	fwrite(uti, sizeof(Utilizadores), totalutilizadoresReg, f);
	fclose(f); 
}

int verificaExiste(Login *utilizador)	// Verifica se o utilizador já existe no sistema
{ 
	int i=0;

	for(i=0;i<totalUtilizadoresReg;i++)

	if (_tcsncmp(utilizador->user, utilizadoresReg[i].user, 150) == 0)
		return 1;			

	return 0;
}

void colocaPlayerTabuleiro(Players *Jogador, int numC) //Funcao que coloca o player numa posição aleatoria do tabuleiro
{
	int lin = 0, col = 0, i;
	BOOL colocado = FALSE;

	srand((unsigned)time(NULL));

	do
	{
		lin = rand() % LINHAS;
		col = rand() % LINHAS;

		if (tabuleiro[lin][col] == VAZIO) // Se a posicao estiver vazia
		{
			tabuleiro[lin][col] = (numC+1)*(-1); // Coloca o jogador X na posicao vazia
			Jogador->posX = lin;
			Jogador->posY = col;
			Jogador->Power = NIVEL1;
			Jogador->vidas = VIDAS;
			Jogador->tbombas = 1;	// Nro de bombas que o player possui na mochila
			Jogador->countBombas = 0;
			Jogador->countChaves = 0;
			Jogador->pontos = 0;


			for (i = 0; i < MAX_BOMBAS; i++)
				Jogador->bombasPlayer[i].status = NAO_ACTIVA;

			colocado = TRUE;
		}
	} 
	while (!colocado);
}

int verificaPos(int lin, int col) //Funcao devolve o valor do que está na posição x,y do tabuleiro
{
	return tabuleiro[lin][col];
}

void colocaCaixasTabuleiro() //Funcao que coloca as caixas numa posição aleatoria do tabuleiro
{
	int lin = 0, col = 0;
	int countCaixas = 0;

	srand((unsigned)time(NULL));

	do
	{
		lin = rand() % LINHAS;
		col = rand() % COLUNAS;

		if (tabuleiro[lin][col] == VAZIO) // Se a posicao estiver vazia
		{
			tabuleiro[lin][col] = CAIXA; // Coloca uma caixa na posicao vazia
			Caixa[countCaixas].posX = lin;
			Caixa[countCaixas].posY = col;
			Caixa[countCaixas].energia = NIVEL1;
			Caixa[countCaixas].status = TRUE;
			countCaixas++;
		}
	} 
	while (countCaixas<MAX_CAIXAS);
}

void colocaChavesTabuleiro() //Funcao que coloca as chaves numa posição aleatoria do tabuleiro
{
	int lin = 0, col = 0;
	int countChaves = 0;

	srand((unsigned)time(NULL));

	do
	{
		lin = rand() % LINHAS;
		col = rand() % COLUNAS;

		if (tabuleiro[lin][col] == VAZIO) // Se a posicao estiver vazia
		{
			tabuleiro[lin][col] = CHAVE; // Coloca uma chave na posicao vazia
			Chaves[countChaves].linha = lin;
			Chaves[countChaves].coluna = col;
			Chaves[countChaves].status = TRUE;
			countChaves++;
		}
	} 
	while (countChaves<MAX_CHAVES);
}

void colocaBomba(Players *Jogador, int numC)
{
	HANDLE hTimer5s;
	DATA dados;

	dados = malloc(sizeof(DATA));

	if(Jogador->tbombas>0)	// Caso o player ainda possua bombas na mochila
	{
		tabuleiro[Jogador->posX][Jogador->posY] = BOMBA;
		Jogador->bombasPlayer[Jogador->countBombas].posX = Jogador->posX;
		Jogador->bombasPlayer[Jogador->countBombas].posY = Jogador->posY;
		Jogador->bombasPlayer[Jogador->countBombas].status = TRUE;

		dados->idBomba = Jogador->countBombas;
		dados->numC = numC;
		
		hTimer5s = CreateThread(NULL, 0, timerBombas, dados, 0, NULL);

		Jogador->countBombas++;
		Jogador->tbombas--;		// Retira uma bomba da mochila
	}
}

void movePlayer(int direccao, Players *Jogador, int numC)	// Funcao que move o jogador na direccao indicada
{
	int i;

	switch (direccao)
	{
		case ESQUERDA:
			if (Jogador->posY > 1) //So verifica se pode andar para a esquerda se não estiver encostado a parede
			{
				if (verificaPos(Jogador->posX, Jogador->posY - 1) == VAZIO)
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == BONUSBOMBAS)	// Se a posição de destino tiver um bónus de bombas extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
					Jogador->tbombas++;	// Coloca uma bomba extra na mochila
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == BONUSVIDA)	// Se a posição de destino tiver um bónus de vida extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
					Jogador->vidas++;	// Incrementa uma vida
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == CHAVE)	// Se a posição de destino tiver uma chave
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
					Jogador->countChaves++;	// Incrementa uma chave
					Jogador->pontos += 5;

					for (i = 0; i < MAX_CHAVES; i++)
					{
						if (Chaves[i].linha == Jogador->posX && Chaves[i].coluna == Jogador->posY)
							Chaves[i].status = FALSE;
					}		
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == BONUSNUKE)	// Se a posição de destino tiver um bónus de bomba nuclear
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
					bombaNuclear();
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == CHAVE)	// Se a posição de destino tiver um bónus de bomba nuclear
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
					Jogador->pontos += 5; // Incrementa uma chave
					for (i = 0; i < MAX_CHAVES; i++)
					{
						if (Chaves[i].linha == Jogador->posX && Chaves[i].coluna == Jogador->posY)
							Chaves[i].status = FALSE;
					}
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == PORTAL)	// Se a posição de destino tiver um portal
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY - 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY--;
				}
				else if (verificaPos(Jogador->posX, Jogador->posY - 1) == INIMIGO1||verificaPos(Jogador->posX, Jogador->posY - 1) == INIMIGO2)	// Se a posição de destino tiver um inimigo
					Jogador->vidas--;
			}

			break;

		case DIREITA:
			if (Jogador->posY < COLUNAS - 2) //So verifica se pode andar para a direita se não estiver encostado a parede
			{
				if (verificaPos(Jogador->posX, Jogador->posY + 1) == VAZIO)
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == BONUSBOMBAS)	// Se a posição de destino tiver um bónus de bombas extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
					Jogador->tbombas++;	// Coloca uma bomba extra na mochila
				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == BONUSVIDA)	// Se a posição de destino tiver um bónus de vida extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
					Jogador->vidas++;	// Incrementa uma vida
				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == CHAVE)	// Se a posição de destino tiver uma chave
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
					Jogador->countChaves++;
					Jogador->pontos += 5; // Incrementa uma chave
					for (i = 0; i < MAX_CHAVES; i++)
					{
						if (Chaves[i].linha == Jogador->posX && Chaves[i].coluna == Jogador->posY)
							Chaves[i].status = FALSE;
					}

				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == BONUSNUKE)	// Se a posição de destino tiver um bónus de bomba nuclear
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
					bombaNuclear();
				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == PORTAL)	// Se a posição de destino tiver um portal
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX][Jogador->posY + 1] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posY++;
				}
				else if (verificaPos(Jogador->posX, Jogador->posY + 1) == INIMIGO1||verificaPos(Jogador->posX, Jogador->posY + 1) == INIMIGO2)	// Se a posição de destino tiver um inimigo
					Jogador->vidas--;
			}

			break;

		case CIMA:
			if (Jogador->posX > 1) //So verifica se pode andar para cima se não estiver encostado a parede
			{
				if (verificaPos(Jogador->posX - 1, Jogador->posY) == VAZIO)
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == BONUSBOMBAS)	// Se a posição de destino tiver um bónus de bombas extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
					Jogador->tbombas++;	// Coloca uma bomba extra na mochila
				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == BONUSVIDA)	// Se a posição de destino tiver um bónus de vida extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
					Jogador->vidas++;	// Incrementa uma vida
				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == CHAVE)	// Se a posição de destino tiver uma chave
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
					Jogador->countChaves++;// Incrementa uma chave
					Jogador->pontos += 5;
					for (i = 0; i < MAX_CHAVES; i++)
					{
						if (Chaves[i].linha == Jogador->posX && Chaves[i].coluna == Jogador->posY)
							Chaves[i].status = FALSE;
					}

				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == BONUSNUKE)	// Se a posição de destino tiver um bónus de bomba nuclear
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
					bombaNuclear();
				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == PORTAL)	// Se a posição de destino tiver um portal
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX - 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX--;
				}
				else if (verificaPos(Jogador->posX - 1, Jogador->posY) == INIMIGO1||verificaPos(Jogador->posX - 1, Jogador->posY) == INIMIGO2)	// Se a posição de destino tiver um inimigo
					Jogador->vidas--;
			}

			break;

		case BAIXO:
			if (Jogador->posX < LINHAS - 2) //So verifica se pode andar para baixo se não estiver encostado a parede
			{
				if (verificaPos(Jogador->posX + 1, Jogador->posY) == VAZIO)
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == BONUSBOMBAS)	// Se a posição de destino tiver um bónus de bombas extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
					Jogador->tbombas++;	// Coloca uma bomba extra na mochila
				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == BONUSVIDA)	// Se a posição de destino tiver um bónus de vida extra
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
					Jogador->vidas++;	// Incrementa uma vida
				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == CHAVE)	// Se a posição de destino tiver uma chave
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC + 1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
					Jogador->countChaves++;// Incrementa uma chave
					Jogador->pontos += 5;
					for (i = 0; i < MAX_CHAVES; i++)
					{
						if (Chaves[i].linha == Jogador->posX && Chaves[i].coluna == Jogador->posY)
							Chaves[i].status = FALSE;
					}

				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == BONUSNUKE)	// Se a posição de destino tiver um bónus de bomba nuclear
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
					bombaNuclear();
				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == PORTAL)	// Se a posição de destino tiver um portal
				{
					tabuleiro[Jogador->posX][Jogador->posY] = VAZIO; // Posicao antiga fica vazia
					tabuleiro[Jogador->posX + 1][Jogador->posY] = (numC+1) * (-1); // Posicao actual fica com Player
					Jogador->posX++;
				}
				else if (verificaPos(Jogador->posX + 1, Jogador->posY) == INIMIGO1||verificaPos(Jogador->posX + 1, Jogador->posY) == INIMIGO2)	// Se a posição de destino tiver um inimigo
					Jogador->vidas--;
			}

			break;

		default:
			break;
	}
}

void bombaNuclear()
{
	int i, j, k;

	for (i = 0; i < LINHAS; i++)
		for (j = 0; j < COLUNAS; j++)
		{
			for (k = 0; k < MAX_CAIXAS; k++)	// Rebenta com todas as caixas
				if (Caixa[k].status==TRUE && Caixa[k].posX == i && Caixa[k].posY == j)
				{
					Caixa[k].energia=0;
					Caixa[k].status=FALSE;
					tabuleiro[i][j] = VAZIO;
				}

			for (k = 0; k < MAX_INIMIGOS; k++)	// Rebenta com todos os inimigos
				if (listInim[k]->activo==1&&listInim[k]->linha==i&&listInim[k]->coluna==j)
				{
					listInim[k]->activo=0;
					tabuleiro[i][j] = VAZIO;
				}
		}
}

void actualizaTabuleiro()	// Função que actualiza o tabuleiro em cada acção
{
	int i,j,k,l,z;
	DWORD nBytesWrite;

	for (i=0;i<LINHAS;i++)
	{
		for (j = 0; j < COLUNAS; j++)
		{			
			for (z = 0; z < numCliente; z++)	// Posição das Bombas
			{
				for (k = 0; k < Player[z].countBombas; k++)
				{
					if (Player[z].bombasPlayer[k].status == ACTIVA && Player[z].bombasPlayer[k].posX == i && Player[z].bombasPlayer[k].posY == j)
						tabuleiro[i][j] = BOMBA;
					//else if (Player[z].bombasPlayer[k].status == BOMBA_OFF && Player[z].bombasPlayer[k].posX == i && Player[z].bombasPlayer[k].posY == j)
					//	tabuleiro[i][j] = VAZIO;
				}
			}
			
			for (k = 0; k < MAX_CAIXAS; k++)	// Posição das Caixas
				if (Caixa[k].status==TRUE && Caixa[k].energia>0&& Caixa[k].posX == i && Caixa[k].posY == j)
					tabuleiro[i][j] = CAIXA;

			for (k = 0; k < MAX_CHAVES; k++)	// Posição das Chaves
				if (Chaves[k].status==TRUE && Chaves[k].linha == i && Chaves[k].coluna == j)
					tabuleiro[i][j] = CHAVE;
			
			for (z = 0; z < numCliente; z++)	// Posição dos Players
				if (Player[z].posX == i && Player[z].posY == j)
					tabuleiro[i][j] = (z + 1) * (-1);
			
			for (l = 0; l < MAX_INIMIGOS; l++)	// Posição dos inimigos
				if (listInim[l]->activo==1&&listInim[l]->linha == i && listInim[l]->coluna == j)
				{
					if(nivel==1)
						tabuleiro[i][j] = INIMIGO1;

					if(nivel==2)
					{
						if(l<3)
							tabuleiro[i][j]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
						else
							tabuleiro[i][j]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
					}

					if(nivel==3)
					{
						if(l<2)
							tabuleiro[i][j]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
						else
							tabuleiro[i][j]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
					}

					if(nivel==4)
					{
						if(l<1)
							tabuleiro[i][j]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
						else
							tabuleiro[i][j]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
					}

					if(nivel==5)					
						tabuleiro[i][j]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia

				}
		}
	}
}

void rebentarBomba(int idBomba, int numC) // Função que vai efectuar o "rebentamento" das bombas
{
	int i,random;

	//Coloca o status da bomba a OFF
	Player[numC].bombasPlayer[idBomba].status = BOMBA_OFF;
	
	//Coloca o lugar da bomba vazio
	tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY] = BLAST_CENTRO;
	Sleep(150);
	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] != PAREDE
		&& tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] != CHAVE)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = BLAST_ESQ;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] != PAREDE
		&& tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] != CHAVE)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = BLAST_DIR;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] != PAREDE
		&& tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] != CHAVE)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = BLAST_CIMA;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] != PAREDE
		&& tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] != CHAVE)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = BLAST_BAIXO;
	Sleep(400);

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] == BLAST_ESQ)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = VAZIO;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] == BLAST_DIR)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = VAZIO;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] == BLAST_CIMA)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] == BLAST_BAIXO)
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;

	tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;

	srand(time(NULL));	// Inicializa seed

	// PARA AS CAIXAS

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] == CAIXA) // Posicao à esquerda da bomba
	{
		for (i = 0; i < MAX_CAIXAS; i++)
		{
			if (Caixa[i].posX == Player[numC].bombasPlayer[idBomba].posX && Caixa[i].posY == (Player[numC].bombasPlayer[idBomba].posY - 1))
			{
				Caixa[i].energia--;
				Player[numC].pontos += 10;

				if (Caixa[i].energia == 0)
				{
					random = rand() % 100 + 1;	// Gera nro entre 1 e 100

					if(random>=1&&random<=2)	// Com uma probabilidade de 2%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = BONUSNUKE;
						Caixa[i].status = FALSE;
					}
					else if(random>=3&&random<=7)	// Com uma probabilidade de 5%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = BONUSVIDA;
						Caixa[i].status = FALSE;
					}
					else if(random>=8&&random<=17)	// Com uma probabilidade de 10%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = BONUSBOMBAS;
						Caixa[i].status = FALSE;
					}
					else
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = VAZIO;
						Caixa[i].status = FALSE;
					}
				}		
			}
		}
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] == CAIXA) // Posicao à direita da bomba
	{
		for (i = 0; i < MAX_CAIXAS; i++)
		{
			if (Caixa[i].posX == Player[numC].bombasPlayer[idBomba].posX && Caixa[i].posY == (Player[numC].bombasPlayer[idBomba].posY + 1))
			{
				Caixa[i].energia--;
				Player[numC].pontos += 10;


				if (Caixa[i].energia == 0)
				{
					random = rand() % 100 + 1;	// Gera nro entre 1 e 100

					if(random>=1&&random<=2)	// Com uma probabilidade de 2%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = BONUSNUKE;
						Caixa[i].status = FALSE;
					}
					else if(random>=3&&random<=7)	// Com uma probabilidade de 5%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = BONUSVIDA;
						Caixa[i].status = FALSE;
					}
					else if(random>=8&&random<=17)	// Com uma probabilidade de 10%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = BONUSBOMBAS;
						Caixa[i].status = FALSE;
					}
					else
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = VAZIO;
						Caixa[i].status = FALSE;
					}
				}
			}
		}
	}


	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] == CAIXA) // Posicao baixo da bomba
	{
		for (i = 0; i < MAX_CAIXAS; i++)
		{
			if (Caixa[i].posX == (Player[numC].bombasPlayer[idBomba].posX + 1) && Caixa[i].posY == Player[numC].bombasPlayer[idBomba].posY)
			{
				Caixa[i].energia--;
				Player[numC].pontos += 10;


				if (Caixa[i].energia == 0)
				{
					random = rand() % 100 + 1;	// Gera nro entre 1 e 100

					if(random>=1&&random<=2)	// Com uma probabilidade de 2%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSNUKE;
						Caixa[i].status = FALSE;
					}
					else if(random>=3&&random<=7)	// Com uma probabilidade de 5%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSVIDA;
						Caixa[i].status = FALSE;
					}
					else if(random>=8&&random<=17)	// Com uma probabilidade de 10%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSBOMBAS;
						Caixa[i].status = FALSE;
					}
					else
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;
						Caixa[i].status = FALSE;
					}
				}
			}
		}
	}


	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] == CAIXA) // Posicao cima da bomba
	{
		for (i = 0; i < MAX_CAIXAS; i++)
		{
			if (Caixa[i].posX == (Player[numC].bombasPlayer[idBomba].posX - 1) && Caixa[i].posY == Player[numC].bombasPlayer[idBomba].posY)
			{
				Caixa[i].energia--;
				Player[numC].pontos += 10;

				if (Caixa[i].energia == 0)
				{
					random = rand() % 100 + 1;	// Gera nro entre 1 e 100

					if(random>=1&&random<=2)	// Com uma probabilidade de 2%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSNUKE;
						Caixa[i].status = FALSE;
					}
					else if(random>=3&&random<=7)	// Com uma probabilidade de 5%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSVIDA;
						Caixa[i].status = FALSE;
					}
					else if(random>=8&&random<=17)	// Com uma probabilidade de 10%
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = BONUSBOMBAS;
						Caixa[i].status = FALSE;
					}
					else
					{
						tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;
						Caixa[i].status = FALSE;
					}
				}
			}
		}
	}


	// PARA OS INIMIGOS

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] == INIMIGO1||tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] == INIMIGO2) // Posicao à esquerda da bomba
	{
		for (i = 0; i < MAX_INIMIGOS; i++)
		{
			if (listInim[i]->linha == Player[numC].bombasPlayer[idBomba].posX && listInim[i]->coluna == (Player[numC].bombasPlayer[idBomba].posY - 1))
			{
				listInim[i]->activo=0;
				tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = VAZIO;	
				Player[numC].pontos += 20;
			}
		}
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] == INIMIGO1||tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] == INIMIGO2) // Posicao à esquerda da bomba
	{
		for (i = 0; i < MAX_INIMIGOS; i++)
		{
			if (listInim[i]->linha == Player[numC].bombasPlayer[idBomba].posX && listInim[i]->coluna == (Player[numC].bombasPlayer[idBomba].posY + 1))
			{
				listInim[i]->activo=0;
				Player[numC].pontos += 20;
				tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = VAZIO;	
			}
		}
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] == INIMIGO1||tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] == INIMIGO2) // Posicao à esquerda da bomba
	{
		for (i = 0; i < MAX_INIMIGOS; i++)
		{
			if (listInim[i]->linha == Player[numC].bombasPlayer[idBomba].posX + 1 && listInim[i]->coluna == (Player[numC].bombasPlayer[idBomba].posY))
			{
				listInim[i]->activo=0;
				Player[numC].pontos += 20;
				tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;	
			}
		}
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] == INIMIGO1||tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] == INIMIGO2) // Posicao à esquerda da bomba
	{
		for (i = 0; i < MAX_INIMIGOS; i++)
		{
			if (listInim[i]->linha == Player[numC].bombasPlayer[idBomba].posX - 1 && listInim[i]->coluna == (Player[numC].bombasPlayer[idBomba].posY))
			{
				listInim[i]->activo=0;
				Player[numC].pontos += 20;
				tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;	
			}
		}
	}

	// PARA OS PLAYERS

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] < 0) // < 0 --> São Jogadores
	{
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = VAZIO;
		Player[numC].vidas--;
		recolocaPlayer(&Player[numC], numC);
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] < 0) // < 0 --> São Jogadores
	{
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = VAZIO;
		Player[numC].vidas--;
		recolocaPlayer(&Player[numC], numC);

	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] < 0) // < 0 --> São Jogadores
	{
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;
		Player[numC].vidas--;
		recolocaPlayer(&Player[numC], numC);
	}

	if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] < 0) // < 0 --> São Jogadores
	{
		tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;
		Player[numC].vidas--;
		recolocaPlayer(&Player[numC], numC);
	}
	// PARA AS CHAVES
	//AS BOMBAS NÃO DEVE REBENTAR AS CHAVES SENÃO É IMPOSSIVEL ACABAR O JOGO! SE ASSIM FOR É GAMEOVER!!

	//if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] == CHAVE) // Posicao à esquerda da bomba
	//{
	//	for (i = 0; i < MAX_CHAVES; i++)
	//	{
	//		if (Chaves[i].linha == Player[numC].bombasPlayer[idBomba].posX && Chaves[i].coluna == (Player[numC].bombasPlayer[idBomba].posY - 1))
	//		{
	//			Chaves[i].status=FALSE;
	//			tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY - 1] = VAZIO;	
	//		}
	//	}
	//}

	//if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] == CHAVE) // Posicao à esquerda da bomba
	//{
	//	for (i = 0; i < MAX_CHAVES; i++)
	//	{
	//		if (Chaves[i].linha == Player[numC].bombasPlayer[idBomba].posX && Chaves[i].coluna == (Player[numC].bombasPlayer[idBomba].posY + 1))
	//		{
	//			Chaves[i].status=FALSE;
	//			tabuleiro[Player[numC].bombasPlayer[idBomba].posX][Player[numC].bombasPlayer[idBomba].posY + 1] = VAZIO;	
	//		}
	//	}
	//}

	//if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] == CHAVE) // Posicao à esquerda da bomba
	//{
	//	for (i = 0; i < MAX_CHAVES; i++)
	//	{
	//		if (Chaves[i].linha == Player[numC].bombasPlayer[idBomba].posX + 1&& Chaves[i].coluna == (Player[numC].bombasPlayer[idBomba].posY))
	//		{
	//			Chaves[i].status=FALSE;
	//			tabuleiro[Player[numC].bombasPlayer[idBomba].posX + 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;	
	//		}
	//	}
	//}

	//if (tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] == CHAVE) // Posicao à esquerda da bomba
	//{
	//	for (i = 0; i < MAX_CHAVES; i++)
	//	{
	//		if (Chaves[i].linha == Player[numC].bombasPlayer[idBomba].posX - 1&& Chaves[i].coluna == (Player[numC].bombasPlayer[idBomba].posY))
	//		{
	//			Chaves[i].status=FALSE;
	//			tabuleiro[Player[numC].bombasPlayer[idBomba].posX - 1][Player[numC].bombasPlayer[idBomba].posY] = VAZIO;	
	//		}
	//	}
	//}

	actualizaTabuleiro();
}

DWORD WINAPI processarRegisto(LPVOID param)	// Função responsável por registar um novo user
{
	int fSucesso;
	DWORD nBytesRead;
	DWORD nBytesWrite;
	HANDLE hPipe = (HANDLE)param;
	TCHAR opcao[MAX_BUFFER];

	fSucesso = ReadFile(hPipe, &utilizador, sizeof(utilizador), &nBytesRead, NULL);

	if (verificaExiste(&utilizador) == 1)
	{
		_tcscpy(opcao, TEXT("NAO"));
		WriteFile(hPipe, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
	}

	else
	{
		_tcscpy(opcao, TEXT("SIM"));
		WriteFile(hPipe, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);

		_tcscpy(utilizadoresReg[totalUtilizadoresReg].user, utilizador.user);
		_tcscpy(utilizadoresReg[totalUtilizadoresReg].pass, utilizador.pass);

		utilizadoresReg[totalUtilizadoresReg].loginStatus = FALSE;
		utilizadoresReg[totalUtilizadoresReg].maxPontuacao = 0;
		utilizadoresReg[totalUtilizadoresReg].numJogos = 0;

		totalUtilizadoresReg++;

		guardaUserReg(utilizadoresReg, totalUtilizadoresReg);

	}

	return 0;
}

DWORD WINAPI processarPedidos(LPVOID param)  // Função associada à primeira thread (MENU de JOGO)
{
	HANDLE hPipe = (HANDLE)param;
	TCHAR opcao[MAX_BUFFER];
	DWORD nBytesRead, nBytesWrite;
	BOOL fSucesso = FALSE;
	BOOL JOGO = FALSE;
	BOOL TERMINAR = FALSE;
	int Highscore = 0;
	int pedido = 0;

	do{
		_tprintf(TEXT("[SERVER]: Estou a espera da opcao do CLIENTE: \n"));
		if (!ReadFile(hPipe, &pedido, sizeof(pedido), &nBytesRead, NULL))
		{
			_tperror(TEXT("[SERVER-processarPedidos]: Erro a ler opcao do CLIENTE!\n"));
			exit(-1);
		}

		switch (pedido)
		{
		case LOGIN:				// O Cliente pediu para fazer LOGIN
			_tcscpy(opcao, TEXT("SIM"));
			WriteFile(hPipe, opcao, sizeof(opcao), &nBytesWrite, NULL);
			CreateThread(NULL, 0, processarLogin, (LPVOID)hPipe, 0, NULL);
			break;

		case REGISTO:				// O Cliente pediu para fazer REGISTO
			_tcscpy(opcao, TEXT("SIM"));
			WriteFile(hPipe, opcao, sizeof(opcao), &nBytesWrite, NULL);
			CreateThread(NULL, 0, processarRegisto, (LPVOID)hPipe, 0, NULL);
			break;

		case INICIARJOGO:				// O Cliente pediu para CRIAR JOGO
			//_tcscpy(opcao, TEXT("SIM"));
			// Vou enviar on Nº de JOGADOR
			WriteFile(hPipe, &numCliente, sizeof(numCliente), &nBytesWrite, NULL);
			JOGO=TRUE;
			if(NOVO==TRUE)
			{
				NOVO=FALSE;
				carregaTabuleiro();
			}
			_tcscpy(Player[numCliente].user, utilizador.user);  // Associar o User logado ao Player
			CreateThread(NULL, 0, iniciaJogo, (LPVOID)numCliente, 0, NULL);
			break;

		case HIGHSCORES:				// O Cliente pediu para enviar HIGHSCORES
			//_tcscpy(opcao, TEXT("SIM"));
			//WriteFile(hPipe, opcao, sizeof(opcao), &nBytesWrite, NULL);
			CreateThread(NULL, 0, enviarHighscores, (LPVOID)hPipe, 0, NULL);
			break;
		case SAIR:
			TERMINAR = TRUE;
			DisconnectNamedPipe(hPipe);
			CloseHandle(hPipe);
		default:
			WriteFile(hPipe, TEXT("NAO"), _tcslen(TEXT("NAO")), &nBytesWrite, NULL);
			break;
		}
	} while (!TERMINAR);

	return 0;
}
DWORD WINAPI iniciaJogo(LPVOID param)
{
	int numC = (int)param;
	TCHAR opcao[MAX_BUFFER];
	DWORD nBytesRead;
	DWORD nBytesWrite;
	BOOL JOGO = TRUE;
	TCHAR buf[10];
	int flag,vidasantes=Player[numC].vidas,vidasdepois;
	int flagvida = 0;
	int flagi = 0, flagc = 0, flagfim = 0;
	int flagportalcolocado=0;
	int i, j;

	//hEventoChegou = CreateEvent(NULL, FALSE, FALSE, NULL);
	//OVERLAPPED ovl;
	//ovl.hEvent = hEventoChegou;

	hPipeJogo[numC] = CreateNamedPipe(PIPE_NAME_JOGO, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, MAX_BUFFER*sizeof(TCHAR), MAX_BUFFER*sizeof(TCHAR), 1000, &sa);
	
	if (hPipeJogo[numC] == INVALID_HANDLE_VALUE)
	{
		_tperror(TEXT("Erro na criação do pipe!"));
		exit(-1);
	}

	if (!ConnectNamedPipe(hPipeJogo[numC], NULL))
	{
		_tperror(TEXT("Erro na ligação ao cliente!"));
		exit(-1);
	}

	//WaitForSingleObject(ovl.hEvent, INFINITE);

	JOGO=TRUE;

	while (JOGO)
	{
		flag=0;	// Flag que valida a actualização do tabuleiro
		vidasdepois=Player[numC].vidas;	// Actualiza o nro de vidas do player a cada iteração

		if (!ReadFile(hPipeJogo[numC], opcao, sizeof(opcao), &nBytesRead, NULL))
		{
			_tperror(TEXT("[SERVER-processarPedidos]: Erro a ler opcao do CLIENTE!\n"));
			exit(-1);
		}

		opcao[nBytesRead / sizeof(TCHAR)] = '\0';
		
		if (_tcsncmp(opcao, TEXT("COLOCA_PLAYER"), 13) == 0)	// O cliente pedir para ser colocado no Tabuleiro
		{
			colocaPlayerTabuleiro(&Player[numC], numC);	// Coloca o jogador no tabuleiro

			vidasantes=Player[numC].vidas;	// Inicializa o nro de vidas do player
			numCliente++;

		}			
		else if (_tcsncmp(opcao, TEXT("PLAYER_ESQ"), 10) == 0)	// O Player pediu para ir para a esquerda
			movePlayer(ESQUERDA,&Player[numC],numC);		
		else if (_tcsncmp(opcao, TEXT("PLAYER_DIR"), 10) == 0)	// O player pediu para ir para a direita
			movePlayer(DIREITA,&Player[numC],numC);		
		else if (_tcsncmp(opcao, TEXT("PLAYER_CIM"), 10) == 0)	// O player pediu para ir para cima
			movePlayer(CIMA,&Player[numC],numC);		
		else if (_tcsncmp(opcao, TEXT("PLAYER_BAI"), 10) == 0)	// O player pediu para ir para baixo
			movePlayer(BAIXO,&Player[numC], numC);		
		else if (_tcsncmp(opcao, TEXT("BOMBA"), 10) == 0)		// O player pediu para ir para colocar bomba
			colocaBomba(&Player[numC], numC);		
		else if (_tcsncmp(opcao, TEXT("SAIR"), 10) == 0)		// O player pediu para sair
		{
			JOGO = FALSE;
			tabuleiro[Player[numC].posX][Player[numC].posY] = VAZIO;
			numCliente--;
			if (numCliente == 0) // Quer dizer que é o ultimo player em jogo o nivel volta para o inicio
				nivel = 1;
		}			
		else if (_tcsncmp(opcao, TEXT("ACTUALIZA"), 10) == 0)	// O player pediu o mapa actualizado
			flag=0;
		else if (_tcsncmp(opcao, TEXT("NVIDAS"), 10) == 0)		// O player pediu o nro de vidas disponíveis
		{
			flag=1;			// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			_itow_s(Player[numC].vidas,buf,10,10);				// Converte inteiro para TCHAR
			WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
		}
		else if (_tcsncmp(opcao, TEXT("PONTOS"), 10) == 0)		// O player pediu o nro de vidas disponíveis
		{
			flag = 1;			// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			_itow_s(Player[numC].pontos, buf, 10, 10);				// Converte inteiro para TCHAR
			WriteFile(hPipeJogo[numC], buf, sizeof(buf), &nBytesWrite, NULL);
		}

		else if (_tcsncmp(opcao, TEXT("CHAVES"), 10) == 0)		// O player pediu o nro de chaves
		{
			flag = 1;			// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			_itow_s(Player[numC].countChaves, buf, 10, 10);				// Converte inteiro para TCHAR
			WriteFile(hPipeJogo[numC], buf, sizeof(buf), &nBytesWrite, NULL);
		}
		else if (_tcsncmp(opcao, TEXT("NIVEL"), 10) == 0)		// O player pediu o nro de chaves
		{
			flag = 1;			// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			_itow_s(nivel, buf, 10, 10);				// Converte inteiro para TCHAR
			WriteFile(hPipeJogo[numC], buf, sizeof(buf), &nBytesWrite, NULL);
		}
		else if (_tcsncmp(opcao, TEXT("NBOMBAS"), 10) == 0)		// O player pediu o nro de bombas disponíveis
		{
			flag=1;			// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			_itow_s (Player[numC].tbombas,buf,10,10);			// Converte inteiro para TCHAR
			WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
		}
		else if (_tcsncmp(opcao, TEXT("RECURSOS"), 10) == 0)		// O player pediu para verificar se perdeu uma vida ou se perdeu o jogo
		{
			if(vidasdepois==0)		// Acabaram-se as vidas
			{
				flag=1;				// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
				flagvida=0;		// Flag de fim de jogo
				_itow_s (flagvida,buf,10,10);	// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
				JOGO=FALSE;		// Sai do jogo

				tabuleiro[Player[numC].posX][Player[numC].posY] = VAZIO;
				numCliente--;
			}
			else if(vidasdepois<vidasantes)	// Caso tenha sido perdida uma vida
			{
				flag=1;				// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração	
				recolocaPlayer(&Player[numC],numC);	// Recoloca o player numa posição aleatória e válida
				vidasantes=vidasdepois;				// Actualiza o contador de vidas
				flagvida=1;						// Flag de vida descontada
				_itow_s (flagvida,buf,10,10);		// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
			}
			else
			{
				flag=1;				// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
				flagvida=2;		// Flag de validação OK
				_itow_s (flagvida,buf,10,10);	// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
			}
		}

		else if (_tcsncmp(opcao, TEXT("JUMP"), 10) == 0)
		{
			// A Cada passagem de nível actualiza a pontuação máxima se a actual for maior
			for (i = 0; i < numCliente; i++)
			{
				for (j = 0; j < totalUtilizadoresReg; j++)
				{
					if (_tcscmp(Player[i].user, utilizadoresReg[j].user, 150) == 0)
					{
						if (Player[i].pontos > utilizadoresReg[j].maxPontuacao)
							utilizadoresReg[j].maxPontuacao = Player[i].pontos;
						utilizadoresReg[j].numJogos++;
					}
				}
			}
			guardaUserReg(utilizadoresReg, totalUtilizadoresReg);
			nivel++;
			flagportalcolocado = 0;
			portal.status = FALSE;
			//tabuleiro[Player[numC].posX][Player[numC].posY] = VAZIO;
			recarregaTabuleiro();
			recolocaPlayer(&Player[numC], numC);	// Coloca o jogador no tabuleiro
			vidasantes = Player[numC].vidas;	// Inicializa o nro de vidas do player
		}
		else if (_tcsncmp(opcao, TEXT("VALIDAFIM"), 10) == 0)		// O player pediu para verificar se ganhou o jogo
		{
			flag = 1;	// Marca a flag a 1 para que não actualize o tabuleiro nesta iteração
			flagi = 0;
			flagc = 0;
			flagfim = 0;

			for(i=0;i<MAX_INIMIGOS;i++)	// Verifica se os inimigos estão todos mortos
				if(listInim[i]->activo==1)
					flagi=1;

			for(i=0;i<MAX_CHAVES;i++)	// Verifica se as chaves foram apanhadas
				if(Chaves[i].status==TRUE)
					flagc=1;

			if(flagi == 0 && flagc == 0 && flagportalcolocado == 0)	// Se os inimigos estiverem todos mortos e as chaves apanhadas
			{
				colocaPortal();			// Abre o portal
				flagportalcolocado=1;	// Marca o portal como colocado
				flagfim = 1;				// Valida a colocação do portal 
				_itow_s (flagfim,buf,10,10);		// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
			}
			else if(flagportalcolocado==1&&Player[numC].posX==portal.linha&&Player[numC].posY==portal.coluna)	// Se o player estiver no portal
			{				
				flagfim=2;				// Valida o fim do nível
				_itow_s (flagfim,buf,10,10);		// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
			}
			else
			{
				flagfim=0;			// Não valida o fim do jogo
				_itow_s (flagfim,buf,10,10);		// Converte inteiro para TCHAR
				WriteFile(hPipeJogo[numC],buf,sizeof(buf),&nBytesWrite,NULL);
			}	
		}

		if (JOGO&&flag == 0)	// Caso o jogo ainda decorra e a flag de validação esteja a zero
		{
			//if (flagfim != 2)
			actualizaTabuleiro();	// Actualiza o tabuleiro
			WriteFile(hPipeJogo[numC], tabuleiro, sizeof(tabuleiro), &nBytesWrite, NULL);
		}
	}

	//tabuleiro[Player[numC].posY][Player[numC].posX] = VAZIO;	// Se o jogador sair, coloca a sua posição vazia
	actualizaTabuleiro();
	WriteFile(hPipeJogo[numC], tabuleiro, sizeof(tabuleiro), &nBytesWrite, NULL);

	//SetEvent(hEventoChegou);

	if (!DisconnectNamedPipe(hPipeJogo[numC]))
	{
		_tperror(TEXT("Erro ao desligar o pipe!"));
		exit(-1);
	}

	CloseHandle(hPipeJogo[numC]);

	return 0;
}

void colocaPortal()
{
	int lin = 0, col = 0;
	int countPortal = 0;

	srand((unsigned)time(NULL));

	do
	{
		lin = rand() % LINHAS;
		col = rand() % COLUNAS;

		if (tabuleiro[lin][col] == VAZIO) // Se a posicao estiver vazia
		{
			tabuleiro[lin][col] = PORTAL; // Coloca um portal na posicao vazia
			portal.linha = lin;
			portal.coluna = col;
			portal.status = TRUE;
			countPortal++;
		}
	} 
	while (countPortal<1);
}

void recolocaPlayer(Players *Jogador, int numC)	// Após perder uma vida, esta função recoloca o player numa nova posição aleatória e válida
{
	int lin = 0, col = 0, i;
	BOOL colocado = FALSE;

	srand((unsigned)time(NULL));

	tabuleiro[Jogador->posX][Jogador->posY]=VAZIO;	// A posição actual fica vazia

	do
	{
		lin = rand() % LINHAS;
		col = rand() % LINHAS;

		if (tabuleiro[lin][col] == VAZIO) // Se a posicao estiver vazia
		{
			Jogador->posX = lin;
			Jogador->posY = col;
			Jogador->tbombas = 1;	// Nro de bombas que o player possui na mochila
			Jogador->countBombas = 0;

			for (i = 0; i < MAX_BOMBAS; i++)
				Jogador->bombasPlayer[i].status = NAO_ACTIVA;

			colocado = TRUE;
		}
	} 
	while (!colocado);

	if(numC==1)		// Para cada player, a posição actual fica ocupada por este
		tabuleiro[Jogador->posX][Jogador->posY]=PLAYER1;
	else if(numC==2)
		tabuleiro[Jogador->posX][Jogador->posY]=PLAYER2;
	else if(numC==3)
		tabuleiro[Jogador->posX][Jogador->posY]=PLAYER3;
	else if(numC==4)
		tabuleiro[Jogador->posX][Jogador->posY]=PLAYER4;
}


DWORD WINAPI processarLogin(LPVOID param)	// Função responsável pelo processamento de autenticação do login
{
	int ret;
	DWORD nBytesWrite;
	DWORD nBytesRead;
	HANDLE hPipe = (HANDLE)param;
	TCHAR opcao[MAX_BUFFER];

	ret = ReadFile(hPipe, &utilizador, sizeof(utilizador), &nBytesRead, NULL);

	if (verificaUserExiste(&utilizador) == 1)
	{
		_tcscpy(opcao, TEXT("SIM"));
		WriteFile(hPipe, opcao,sizeof(opcao), &nBytesWrite, NULL);
	}
	else
	{
		_tcscpy(opcao, TEXT("NAO"));
		WriteFile(hPipe, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
	}
	return 0;
}
DWORD WINAPI enviarHighscores(LPVOID param){
	int ret;
	DWORD nBytesWrite;
	DWORD nBytesRead;
	HANDLE hPipe = (HANDLE)param;
	TCHAR opcao[MAX_BUFFER];
	int i;

	for (i = 0; i < totalUtilizadoresReg; i++)
	{
		_tcscpy(utilizadoresHighscores[i].user, utilizadoresReg[i].user);
		_itow_s(utilizadoresReg[i].numJogos, utilizadoresHighscores[i].numJogos, 10, 10);				// Converte inteiro para TCHAR
		_itow_s(utilizadoresReg[i].maxPontuacao, utilizadoresHighscores[i].maxPontuacao, 10, 10);				// Converte inteiro para TCHAR
	}
	
	WriteFile(hPipe, &utilizadoresHighscores, sizeof(utilizadoresHighscores), &nBytesWrite, NULL);
	WriteFile(hPipe, &totalUtilizadoresReg, sizeof(totalUtilizadoresReg), &nBytesWrite, NULL);

	return 0;
}
DWORD WINAPI timerBombas(LPVOID param)
{
	DATA dados;
	int seg = 0;
	LARGE_INTEGER li;

	dados = (DATA)param;
	HANDLE hWTimerLocal = CreateWaitableTimer(NULL, 0, NULL); //criar timer que conta o tempo decorrido

	li.QuadPart = -50000000; // Intervalo de tempo relativo ao instante actual (5s)
	SetWaitableTimer(hWTimerLocal, &li, 0, NULL, NULL, 0);

	WaitForSingleObject(hWTimerLocal, INFINITE);

	rebentarBomba(dados->idBomba, dados->numC);

	CloseHandle(hWTimerLocal);

	if(Player[dados->numC].tbombas==0)
		Player[dados->numC].tbombas++;		// Repõe uma bomba na mochila após a anterior rebentar

	return 0;
}

void criaInimigos()		// Função que coloca os inimigos em posições aleatórias e válidas no tabuleiro
{
	int lin=0,col=0;
	int countInim=0;

	hMemInim1=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,sizeof(Inimigos),Inim1);
	hMemInim2=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,sizeof(Inimigos),Inim2);
	hMemInim3=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,sizeof(Inimigos),Inim3);
	hMemInim4=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,sizeof(Inimigos),Inim4);

	if (hMemInim1==NULL||hMemInim2==NULL||hMemInim3==NULL||hMemInim4==NULL)
	{
		_tprintf(TEXT("[Erro]Criação de objectos do Windows(%d)\n"),GetLastError());
		return -1;
	}

	listInim[0]=PtrInim1=(Inimigos(*))MapViewOfFile(hMemInim1,FILE_MAP_WRITE,0,0,sizeof(Inimigos));
	listInim[1]=PtrInim2=(Inimigos(*))MapViewOfFile(hMemInim2,FILE_MAP_WRITE,0,0,sizeof(Inimigos));
	listInim[2]=PtrInim3=(Inimigos(*))MapViewOfFile(hMemInim3,FILE_MAP_WRITE,0,0,sizeof(Inimigos));
	listInim[3]=PtrInim4=(Inimigos(*))MapViewOfFile(hMemInim4,FILE_MAP_WRITE,0,0,sizeof(Inimigos));

	if (PtrInim1==NULL||PtrInim2==NULL||PtrInim3==NULL||PtrInim4==NULL)
	{
		_tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"), GetLastError());
		return -1;
	}

	srand(time(NULL));	// Inicializa seed

	do
	{
		lin=rand()%LINHAS;
		col=rand()%COLUNAS;

		if (tabuleiro[lin][col]==VAZIO)			// Se a posicao estiver vazia
		{
			if(nivel==1)
				tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia

			if(nivel==2)
			{
				if(countInim<3)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==3)
			{
				if(countInim<2)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==4)
			{
				if(countInim<1)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==5)
				tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia

			listInim[countInim]->linha=lin;
			listInim[countInim]->coluna=col;
			listInim[countInim]->activo=1;
			listInim[countInim]->id=countInim;

			countInim++;
		}
	} 
	while (countInim<MAX_INIMIGOS);

	CreateThread(NULL,0,iaInimigos,(LPVOID)countInim,0,NULL);	// Cria a thread de comportamento dos inimigos
}

DWORD WINAPI iaInimigos(LPVOID param)	// Thread de comportamento dos inimigos (para já com movimento aleatório e não agressivo)
{
	int countInim=(int)param,n,i;

	JOGO=TRUE;

	srand((unsigned)time(NULL));

	while(JOGO==TRUE)
	{
		for(i = 0; i < countInim; i++)
		{
			n=rand() % 4 + 1;

			switch(n)
			{
				case 1: // Esquerda
					if (listInim[i]->coluna > 1&&listInim[i]->activo==1) // So verifica se pode andar para a esquerda se não estiver encostado a parede
					{
						if (verificaPos(listInim[i]->linha, listInim[i]->coluna - 1) == VAZIO)
						{
							tabuleiro[listInim[i]->linha][listInim[i]->coluna] = VAZIO;		// Posicao antiga fica vazia

							if(nivel==1)
								tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO1; // Posicao actual fica com Inimigo 1

							if(nivel==2)
							{
								if(i<3)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==3)
							{
								if(i<2)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==4)
							{
								if(i<1)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==5)
								tabuleiro[listInim[i]->linha][listInim[i]->coluna - 1] = INIMIGO2; // Posicao actual fica com Inimigo 2

							listInim[i]->coluna--;
						}						
					}
					break;

				case 2: // Direita
					if (listInim[i]->coluna < COLUNAS - 2&&listInim[i]->activo==1) //So verifica se pode andar para a direita se não estiver encostado a parede
					{
						if (verificaPos(listInim[i]->linha, listInim[i]->coluna + 1) == VAZIO)
						{
							tabuleiro[listInim[i]->linha][listInim[i]->coluna] = VAZIO;		// Posicao antiga fica vazia
							
							if(nivel==1)
								tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO1; // Posicao actual fica com Inimigo 1

							if(nivel==2)
							{
								if(i<3)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==3)
							{
								if(i<2)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==4)
							{
								if(i<1)
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==5)
								tabuleiro[listInim[i]->linha][listInim[i]->coluna + 1] = INIMIGO2; // Posicao actual fica com Inimigo 2

							listInim[i]->coluna++;
						}						
					}
					break;

				case 3: // Cima
					if (listInim[i]->linha > 1&&listInim[i]->activo==1) //So verifica se pode andar para cima se não estiver encostado a parede
					{
						if (verificaPos(listInim[i]->linha - 1, listInim[i]->coluna) == VAZIO)
						{
							tabuleiro[listInim[i]->linha][listInim[i]->coluna] = VAZIO;		// Posicao antiga fica vazia
							
							if(nivel==1)
								tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1

							if(nivel==2)
							{
								if(i<3)
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==3)
							{
								if(i<2)
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==4)
							{
								if(i<1)
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==5)
								tabuleiro[listInim[i]->linha - 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2

							listInim[i]->linha--;
						}						
					}
					break;

				case 4: // Baixo
					if (listInim[i]->linha < LINHAS - 2&&listInim[i]->activo==1) //So verifica se pode andar para baixo se não estiver encostado a parede
					{
						if (verificaPos(listInim[i]->linha + 1, listInim[i]->coluna) == VAZIO)
						{
							tabuleiro[listInim[i]->linha][listInim[i]->coluna] = VAZIO;		// Posicao antiga fica vazia
							
							if(nivel==1)
								tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1

							if(nivel==2)
							{
								if(i<3)
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==3)
							{
								if(i<2)
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==4)
							{
								if(i<1)
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO1; // Posicao actual fica com Inimigo 1
								else
									tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2
							}

							if(nivel==5)
								tabuleiro[listInim[i]->linha + 1][listInim[i]->coluna] = INIMIGO2; // Posicao actual fica com Inimigo 2

							listInim[i]->linha++;
						}						
					}
					break;

				default:
					break;
			}
		}

		Sleep(1000);
	}
	
	return 0;
}

void regeneraInimigos()
{
	int lin=0,col=0;
	int countInim=0;

	srand(time(NULL));	// Inicializa seed

	do
	{
		lin=rand()%LINHAS;
		col=rand()%COLUNAS;

		if (tabuleiro[lin][col]==VAZIO)			// Se a posicao estiver vazia
		{
			if(nivel==1)
				tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia

			if(nivel==2)
			{
				if(countInim<3)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==3)
			{
				if(countInim<2)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==4)
			{
				if(countInim<1)
					tabuleiro[lin][col]=INIMIGO1;		// Coloca um inimigo 1 na posicao vazia
				else
					tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia
			}

			if(nivel==5)
				tabuleiro[lin][col]=INIMIGO2;		// Coloca um inimigo 2 na posicao vazia

			listInim[countInim]->linha=lin;
			listInim[countInim]->coluna=col;
			listInim[countInim]->activo=1;
			listInim[countInim]->id=countInim;

			countInim++;
		}
	} 
	while (countInim<MAX_INIMIGOS);

	CreateThread(NULL,0,iaInimigos,(LPVOID)countInim,0,NULL);	// Cria a thread de comportamento dos inimigos
}