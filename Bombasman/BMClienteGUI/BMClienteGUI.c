
#include "resource.h"
#include "utils.h"

// Pré-declaração da função WndProc (a que executa os procedimentos da janela por
// "callback") 
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

HWND hPass;
HWND hLogin;
HWND hBotaoOK;
HWND hBotaoCancel;

HINSTANCE hInstGlobal;

HDC memDC, auxdc;

TCHAR *szProgName = TEXT("BombasMan");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, 
				   LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;			// hWnd é o handler da janela, gerado mais abaixo 
						// por CreateWindow()
	MSG lpMsg;			// MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX é uma estrutura cujos membros servem para 
						// definir as características da classe da janela
	hInstGlobal = hInst;

	LPWSTR *szArgList;
	int argCount;
// ============================================================================
// 1. Definição das características da janela "wcApp" 
//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
// ============================================================================

	wcApp.cbSize = sizeof(WNDCLASSEX);	// Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;			// Instância da janela actualmente exibida 
										// ("hInst" é parâmetro de WinMain e vem 
										// inicializada daí)
	wcApp.lpszClassName = szProgName;	// Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;		// Endereço da função de processamento da janela 
										// ("WndProc" foi declarada no início e encontra-se
										// mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;			// Estilo da janela: Fazer o redraw
													// se for modificada horizontal ou
													// verticalmente
	/*OUTROS ESTILOS POSSíVEIS: CS_DBLCLKS			permite a captura de duplo cliques do rato
								CS_NOCLOSE			retira o 'X' na barra para fechar a aplicação*/

	// Definir o meu icone (hInst - instância passada no WinMain)
	wcApp.hIcon = LoadIcon(hInst,IDI_ICON1);	// "hIcon" = handler do ícon normal
													// "NULL" = Icon definido no Windows
													// "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(hInst, IDI_ICON1);	// "hIcon" = handler do ícon pequeno
													// "NULL" = Icon definido no Windows
													// "IDI_WIN..." Ícon "Wind.NT logo"
	/*OUTROS TIPOS DE ICONES:	IDI_ASTERISK		Same as IDI_INFORMATION.
								IDI_ERROR			Hand-shaped icon.
								IDI_EXCLAMATION		Same as IDI_WARNING.
								IDI_HAND			Same as IDI_ERROR. 
								IDI_INFORMATION		Asterisk icon.
								IDI_QUESTION		Question mark icon.
								IDI_WARNING			Exclamation point icon*/

	wcApp.hCursor = NULL;	// "hCursor" = handler do cursor (rato)
													// "NULL" = Forma definida no Windows
													// "IDC_ARROW" Aspecto "seta" 
	/*OUTROS TIPOS DE CURSORES:	IDC_CROSS IDC_HAND IDC_HELP IDC_UPARROW IDC_WAIT */


	// Quando não dá, fazer o cast para (TCHAR *) IDR_MENU1
	wcApp.lpszMenuName = NULL;						// Classe do menu que a janela pode ter
													// (NULL = não tem menu)
	wcApp.cbClsExtra = 0;							// Livre, para uso particular
	wcApp.cbWndExtra = 0;							// Livre, para uso particular

	wcApp.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); // "hbrBackground" = handler para 
														// "brush" de pintura do fundo da
														// janela. Devolvido por
														// "GetStockObject". Neste caso o
														// fundo vai ser branco
	/*OUTRAS CORES DE BRUSH:	BLACK_BRUSH  DKGRAY_BRUSH GRAY_BRUSH LTGRAY_BRUSH  */

// ============================================================================
// 2. Registar a classe "wcApp" no Windows
// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);

	
// ============================================================================
// 3. Criar a janela
// ============================================================================
	hWnd = CreateWindow(
		szProgName,				// Nome da janela (programa) definido acima
		TEXT("BombasMAN"),	// Texto que figura na barra da janela
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
								// Outros valores: WS_HSCROLL, WS_VSCROLL
								// (Fazer o OR "|" do que se pretender)
		CW_USEDEFAULT,			// Posição x pixels (default=à direita da última)
		CW_USEDEFAULT,			// Posição y pixels (default=abaixo da última)
		1020,			// Largura da janela (em pixels)
		720,			// Altura da janela (em pixels)
		(HWND) HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir 
								// de outra) ou HWND_DESKTOP se a janela for
								// a primeira, criada a partir do "desktop"
		(HMENU) NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE) hInst,			// handle da instância do programa actual
								// ("hInst" é declarado num dos parâmetros
								// de WinMain(), valor atribuído pelo Windows)
		0);			// Não há parâmetros adicionais para a janela

// ============================================================================
// 4. Mostrar a janela
// ============================================================================

	//DialogBox(hInst, IDD_LOGIN, hWnd, TrataLogin);

	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido 
								// por "CreateWindow"; "nCmdShow"= modo de
								// exibição (p.e. normal, modal); é passado
								// como parâmetro de WinMain()

	UpdateWindow(hWnd);			// Refrescar a janela (Windows envia à janela
								// uma mensagem para pintar, mostrar dados,
								// (refrescar), etc)
	

	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
	if (szArgList == NULL)
	{
		MessageBox(NULL, TEXT("Erro a ler opcao na linha de comandos"), TEXT("Error"), MB_OK);
	}

	if (argCount > 1)
	{
		_sntprintf(nomePipeIni, MAX_BUFFER, TEXT("\\\\%s\\pipe\\BombasMan"), szArgList[1]);
		_sntprintf(nomePipeJogo, MAX_BUFFER, TEXT("\\\\%s\\pipe\\BombasManJogo"), szArgList[1]);
	}
	else
	{
		_tcscpy(nomePipeIni, TEXT("\\\\.\\pipe\\BombasMan"));
		_tcscpy(nomePipeJogo, TEXT("\\\\.\\pipe\\BombasManJogo"));
	}

	LocalFree(szArgList);


	//Esperar pelo PIPE do Servidor
	if (!WaitNamedPipe(nomePipeIni, NMPWAIT_WAIT_FOREVER))
		return -1;

	// Criar um PIPE de comunicação com o server
	hPipe = CreateFile(nomePipeIni, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hPipe != NULL)
	{
		while (GetMessage(&lpMsg, NULL, 0, 0)) {
			TranslateMessage(&lpMsg);			// Pré-processamento da mensagem
			// p.e. obter código ASCII da tecla
			// premida
			DispatchMessage(&lpMsg);			// Enviar a mensagem traduzida de volta
			// ao Windows, que aguarda até que a 
			// possa reenviar à função de tratamento
			// da janela, CALLBACK TrataEventos (mais 
			// abaixo)
		}
	}

// ============================================================================
// 6. Fim do programa
// ============================================================================
	return((int)lpMsg.wParam);		// Retorna-se sempre o parâmetro "wParam" da
								// estrutura "lpMsg"
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) 
	{	
	static HBITMAP bombasman, bombasman2, bombasman3, bombasman4, perdeVida;
	static HBITMAP bombasmanfundo, parede, caixa, hbit, home_bkg, game_bkg, game_over, gameLogin, gameRegisto, gameCreditos;
	static HBITMAP inimigo, inimigo2, bomba, numero, botaoOK, botaoCancel, gameHighscores;
	static HBITMAP loginON, registarON, jogarON, highscoresON, sairON, gameFim;
	static HBITMAP loginOFF, registarOFF, jogarOFF, highscoresOFF, sairOFF;
	static HBITMAP loginOKON, loginOKOFF, loginCancelON, loginCancelOFF, jogarUN, highscoresUN;
	static HBITMAP expCima, expBaixo, expDir, expEsq, expCentro, ErroMaxPlayer;
	static HBITMAP vidaBonus, bombaBonus, bombaNuke, chave, portal, creditosOFF, creditosON;
	static HFONT hFont;
	static int maxX, maxY;
	static int numPlayer;
	static BOOL loginSTATUS = FALSE;
	static BOOL registoSTATUS = FALSE;
	LOGFONT lf;
	HDC hdc;
	PAINTSTRUCT area;

	int cursorPosX, cursorPosY;


	int i, j;

	switch (messg) {

	case WM_COMMAND:
		break;

	case WM_LBUTTONUP:
		switch (gameSTATUS)
		{
		case HOME:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);

			// LOGIN
			if (cursorPosX > 805 && cursorPosX < 930 && cursorPosY > 165 && cursorPosY < 210)
			{
				pedido = LOGIN;
				WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
				ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);

				if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
				{
					gameSTATUS = LOGIN;
					ShowWindow(hLogin, SW_SHOW); // Mostra a textbox;
					ShowWindow(hPass, SW_SHOW); // Mostra a textbox;
					InvalidateRect(hWnd, NULL, 0);
				}
			}

			// REGISTAR
			if (cursorPosX > 780 && cursorPosX < 975 && cursorPosY > 240 && cursorPosY < 290)
			{
				pedido = REGISTO;
				WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
				ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);

				if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
				{
					gameSTATUS = REGISTO;
					ShowWindow(hLogin, SW_SHOW); // Mostra a textbox;
					ShowWindow(hPass, SW_SHOW); // Mostra a textbox;
					InvalidateRect(hWnd, NULL, 0);
				}
			}

			// INICIAR JOGO
			if (cursorPosX > 780 && cursorPosX < 970 && cursorPosY > 315 && cursorPosY < 375 && loginSTATUS == TRUE)
			{
				if (numPlayer == -4)
				{
					gameSTATUS = ERROMAXPLAYERS;
				}
				else
				{
					pedido = INICIARJOGO;
					WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
					ReadFile(hPipe, &numPlayer, sizeof(numPlayer), &nBytesRead, NULL);
					numPlayer = (numPlayer + 1)*(-1);
					Sleep(1000);

					if (!WaitNamedPipe(nomePipeJogo, NMPWAIT_WAIT_FOREVER))
						exit(1);

					hPipeJogo = CreateFile(nomePipeJogo, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hPipeJogo == INVALID_HANDLE_VALUE)
						exit(1);

					// Vou Pedir ao Servidor para colocar Player no field e devolver o tabuleiro
					_tcscpy(opcao, TEXT("COLOCA_PLAYER"));
					WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
					ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);

					gameSTATUS = JOGAR;
					_tcscpy(vidas, TEXT("3"));		// Inicialização
					_tcscpy(bombas, TEXT("1"));		// Inicialização
					_tcscpy(nivel, TEXT("1"));		// Inicialização
					_tcscpy(pontos, TEXT("0"));		// Inicialização
					_tcscpy(countChaves, TEXT("0"));

					CreateThread(NULL, 0, actualizaJogo, (LPVOID)hWnd, 0, NULL);
				}
			}

			//SAIR
			if (cursorPosX > 830 && cursorPosX < 910 && cursorPosY > 480 && cursorPosY < 520)
			{
				pedido = SAIR;
				WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
				Sleep(1000);
				PostQuitMessage(0);
			}

			// CREDITOS
			if (cursorPosX > 149 && cursorPosX < 322 && cursorPosY > 559 && cursorPosY < 620)
			{
				gameSTATUS = CREDITOS;
				InvalidateRect(hWnd, NULL, 0);
			}
				
			// HIGHSCORES
			if (cursorPosX > 770 && cursorPosX < 985 && cursorPosY > 400 && cursorPosY < 450)
			{
				gameSTATUS = TABELASCORES;
				pedido = HIGHSCORES;
				WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
				Sleep(1000);
				ReadFile(hPipe, &utilizadoresHighscores, sizeof(utilizadoresHighscores), &nBytesRead, NULL);
				Sleep(500);
				ReadFile(hPipe, &totUserReg, sizeof(totUserReg), &nBytesRead, NULL);
				InvalidateRect(hWnd, NULL, 0);
			}


			break;
		case LOGIN:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);
			// OK
			if (cursorPosX > 585 && cursorPosX < 630 && cursorPosY > 365 && cursorPosY < 400)
			{
				SendMessage(hLogin, WM_GETTEXT, sizeof(utilizador.user), (LPARAM)utilizador.user);
				SendMessage(hPass, WM_GETTEXT, sizeof(utilizador.pass), (LPARAM)utilizador.pass);
							
				Sleep(500);

				WriteFile(hPipe, &utilizador, sizeof(utilizador), &nBytesWrite, NULL);
				
				ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);
				
				if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
				{
					loginSTATUS = TRUE;
					gameSTATUS = HOME;
					ShowWindow(hLogin, SW_HIDE); // Esconde a textbox;
					ShowWindow(hPass, SW_HIDE); // Esconde a textbox;
				}
					
				else
					loginSTATUS = FALSE;

				InvalidateRect(hWnd, NULL, 0);
			}

			// CANCEL
			if (cursorPosX > 460 && cursorPosX < 575 && cursorPosY > 365 && cursorPosY < 400)
			{
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL
				ShowWindow(hLogin, SW_HIDE); // Mostra a textbox;
				ShowWindow(hPass, SW_HIDE); // Mostra a textbox;
			}
			break;


		case REGISTO:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);
			
			// OK
			if (cursorPosX > 585 && cursorPosX < 630 && cursorPosY > 365 && cursorPosY < 400)
			{
				SendMessage(hLogin, WM_GETTEXT, sizeof(utilizador.user), (LPARAM)utilizador.user);
				SendMessage(hPass, WM_GETTEXT, sizeof(utilizador.pass), (LPARAM)utilizador.pass);

				Sleep(500);

				WriteFile(hPipe, &utilizador, sizeof(utilizador), &nBytesWrite, NULL);

				Sleep(200);

				ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);

				if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
				{
					gameSTATUS = HOME;
					ShowWindow(hLogin, SW_HIDE); // Esconde a textbox;
					ShowWindow(hPass, SW_HIDE); // Esconde a textbox;
				}

				InvalidateRect(hWnd, NULL, 0);
			}

			// CANCEL
			if (cursorPosX > 460 && cursorPosX < 575 && cursorPosY > 365 && cursorPosY < 400)
			{
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL
				ShowWindow(hLogin, SW_HIDE); // Mostra a textbox;
				ShowWindow(hPass, SW_HIDE); // Mostra a textbox;
			}
			break;
		}
		break;

	case WM_MOUSEMOVE:

		switch (gameSTATUS)
		{
		case HOME:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);
			// LOGIN
			if (cursorPosX > 805 && cursorPosX < 930 && cursorPosY > 165 && cursorPosY < 210)
				menuOP = 0;

			// REGISTAR
			if (cursorPosX > 780 && cursorPosX < 975 && cursorPosY > 240 && cursorPosY < 290)
				menuOP = 1;

			// JOGAR
			if (cursorPosX > 780 && cursorPosX < 970 && cursorPosY > 315 && cursorPosY < 375)
				menuOP = 2;

			// HIGHSCORES
			if (cursorPosX > 770 && cursorPosX < 985 && cursorPosY > 400 && cursorPosY < 450)
				menuOP = 3;

			// SAIR
			if (cursorPosX > 830 && cursorPosX < 910 && cursorPosY > 480 && cursorPosY < 520)
				menuOP = 4;

			// CREDITOS
			if (cursorPosX > 149 && cursorPosX < 322 && cursorPosY > 559 && cursorPosY < 620)
				menuOP = 5;

			InvalidateRect(hWnd, NULL, 0);
			break;

		case LOGIN:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);
			// OK
			if (cursorPosX > 585 && cursorPosX < 630 && cursorPosY > 365 && cursorPosY < 400)
			{
				loginOP = 0;
				InvalidateRect(hWnd, NULL, 0);
			}

			// CANCEL
			if (cursorPosX > 460 && cursorPosX < 575 && cursorPosY > 365 && cursorPosY < 400)
			{
				loginOP = 1;
				InvalidateRect(hWnd, NULL, 0);
			}

			break;
		case REGISTO:
			cursorPosX = LOWORD(lParam);
			cursorPosY = HIWORD(lParam);
			// OK
			if (cursorPosX > 585 && cursorPosX < 630 && cursorPosY > 365 && cursorPosY < 400)
			{
				loginOP = 0;
				InvalidateRect(hWnd, NULL, 0);
			}

			// CANCEL
			if (cursorPosX > 460 && cursorPosX < 575 && cursorPosY > 365 && cursorPosY < 400)
			{
				loginOP = 1;
				InvalidateRect(hWnd, NULL, 0);
			}

			break;

		case JOGAR:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (gameSTATUS)
		{
		case HOME:
			if (wParam == VK_UP && menuOP > 0)
				menuOP--;
			if (wParam == VK_DOWN && menuOP < 5)
				menuOP++;
			if (wParam == VK_RETURN)
			{
				switch (menuOP)
				{

				case 0: // LOGIN
					pedido = LOGIN;
					WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
					ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);

					if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
					{
						gameSTATUS = LOGIN;
						ShowWindow(hLogin, SW_SHOW); // Esconde a textbox;
						ShowWindow(hPass, SW_SHOW); // Esconde a textbox;
						InvalidateRect(hWnd, NULL, 0);
					}
					break;

				case 1: // REGISTO
					pedido = REGISTO;
					WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
					ReadFile(hPipe, opcao, sizeof(opcao), &nBytesRead, NULL);

					if (_tcsncmp(opcao, TEXT("SIM"), 3) == 0)
					{
						gameSTATUS = REGISTO;
						ShowWindow(hLogin, SW_SHOW); // Mostra a textbox;
						ShowWindow(hPass, SW_SHOW); // Mostra a textbox;
						InvalidateRect(hWnd, NULL, 0);
					}
					break;

				case 2: // JOGAR
					
					if (loginSTATUS)
					{
						if (numPlayer == -4)
						{
							gameSTATUS = ERROMAXPLAYERS;
						}
						else
						{
							pedido = INICIARJOGO;
							WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
							ReadFile(hPipe, &numPlayer, sizeof(numPlayer), &nBytesRead, NULL);
							numPlayer = (numPlayer + 1)*(-1);
							Sleep(1000);

							if (!WaitNamedPipe(nomePipeJogo, NMPWAIT_WAIT_FOREVER))
								exit(1);

							hPipeJogo = CreateFile(nomePipeJogo, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
							if (hPipeJogo == INVALID_HANDLE_VALUE)
								exit(1);

							// Vou Pedir ao Servidor para colocar Player no field e devolver o tabuleiro
							_tcscpy(opcao, TEXT("COLOCA_PLAYER"));
							WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
							ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);

							gameSTATUS = JOGAR;
							_tcscpy(vidas, TEXT("3"));		// Inicialização
							_tcscpy(bombas, TEXT("1"));		// Inicialização
							_tcscpy(nivel, TEXT("1"));		// Inicialização
							_tcscpy(pontos, TEXT("0"));		// Inicialização
							_tcscpy(countChaves, TEXT("0"));

							CreateThread(NULL, 0, actualizaJogo, (LPVOID)hWnd, 0, NULL);
						}
					}
					//====
					break;

				case  3: // HIGHSCORES
					gameSTATUS = TABELASCORES;
					pedido = HIGHSCORES;
					WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
					Sleep(1000);
					ReadFile(hPipe, &utilizadoresHighscores, sizeof(utilizadoresHighscores), &nBytesRead, NULL);
					Sleep(500);
					ReadFile(hPipe, &totUserReg, sizeof(totUserReg), &nBytesRead, NULL);
					break;

				case  4: //SAIR
					pedido = SAIR;
					WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
					Sleep(1000);
					PostQuitMessage(0);
					break;
				
				case  5: //CREDITOS
					gameSTATUS = CREDITOS;
					break;

				default:
					break;
				}
			}
			break;

		case JOGAR:
			if (wParam == VK_UP) // TECLA PARA CIMA
			{
				_tcscpy(opcao, TEXT("PLAYER_CIM"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
				dirPlayer = CIMA;
			}
				
			if (wParam == VK_DOWN) // TECLA PARA BAIXO
			{
				_tcscpy(opcao, TEXT("PLAYER_BAI"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
				dirPlayer = BAIXO;
			}
			if (wParam == VK_LEFT) // TECLA PARA A ESQUERDA
			{
				_tcscpy(opcao, TEXT("PLAYER_ESQ"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
				dirPlayer = ESQUERDA;
			}
			if (wParam == VK_RIGHT) // TECLA PARA A DIREITA
			{
				_tcscpy(opcao, TEXT("PLAYER_DIR"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
				dirPlayer = DIREITA;
			}
			if (wParam == VK_ESCAPE) // TECLA ESCAPE
			{
				_tcscpy(opcao, TEXT("SAIR"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL
			}
			if (wParam == VK_SHIFT) // TECLA SHIFT
			{
				_tcscpy(opcao, TEXT("BOMBA"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
			}

			if (wParam == 'j' || wParam == 'J')
			{
				_tcscpy(opcao, TEXT("JUMP"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				fSucesso = ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);

				// No final do Nivel 5 - Fim de Jogo
				if (_tcscmp(nivel, TEXT("5"), 10) == 0)
					gameSTATUS = FINNISH;
			}

			_tcscpy(opcao, TEXT("PONTOS"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, pontos, sizeof(pontos), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("NVIDAS"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, vidas, sizeof(vidas), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("NBOMBAS"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, bombas, sizeof(bombas), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("RECURSOS"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, morte, sizeof(morte), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("NIVEL"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, nivel, sizeof(nivel), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("CHAVES"));
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, countChaves, sizeof(countChaves), &nBytesRead, NULL);

			_tcscpy(opcao, TEXT("VALIDAFIM"));		// Valida a condição de vitória
			WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
			fSucesso = ReadFile(hPipeJogo, fim, sizeof(countChaves), &nBytesRead, NULL);

			if (_tcsncmp(fim, TEXT("2"), 1) == 0) // Termina o nível
			{
				_tcscpy(nivel, TEXT("2"));		// Inicialização
				_tcscpy(countChaves, TEXT("0"));
				gameSTATUS = JOGAR;
				_tcscpy(opcao, TEXT("JUMP"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);

				// No final do Nivel 5 - Fim de Jogo
				if (_tcscmp(nivel, TEXT("5"), 10) == 0)
					gameSTATUS = FINNISH;
			}

			if (_tcsncmp(morte, TEXT("0"), 2) == 0)
				gameSTATUS = GAMEOVER;
			else if (_tcsncmp(morte, TEXT("1"), 1) == 0) // Perdeu uma Vida
				perdeuVida = TRUE;

			break;

		case LOGIN:
			//ACCOES DO ECRA DE LOGIN
			if (wParam == VK_ESCAPE) // TECLA ESCAPE
			{
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL
				ShowWindow(hLogin, SW_HIDE); // Esconde a textbox;
				ShowWindow(hPass, SW_HIDE); // Esconde a textbox;
			}
			break;

		case REGISTO:
			//ACCOES DO ECRA DE LOGIN
			if (wParam == VK_ESCAPE) // TECLA ESCAPE
			{
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL
				ShowWindow(hLogin, SW_HIDE); // Esconde a textbox;
				ShowWindow(hPass, SW_HIDE); // Esconde a textbox;
			}
			break;

		case GAMEOVER:
			// Mostra Mensagem de GameOver
			// 
			if (wParam == VK_RETURN) // TECLA ENTER
				gameSTATUS = HOME;

			break;

		case CREDITOS:
			if (wParam == VK_RETURN) // TECLA ENTER
				gameSTATUS = HOME;

			break;

		case TABELASCORES:
			if (wParam == VK_RETURN) // TECLA ENTER
				gameSTATUS = HOME;

			break;

		case FINNISH:
			if (wParam == VK_RETURN) // TECLA ENTER
			{
				_tcscpy(opcao, TEXT("SAIR"));
				WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
				gameSTATUS = HOME; // VAI PARA O MENU INICIAL

			}

			break;

		case ERROMAXPLAYERS:
			// Mostra Mensagem Erro
			// 
			if (wParam == VK_RETURN) // TECLA ENTER
				gameSTATUS = HOME;
			break;

		}
		InvalidateRect(hWnd, NULL, 0);  // 0 - não pinta o fundo | 1 - pinta o fundo
		break;

	case WM_CREATE:

		// UTILIZAR TIPO DE LETRA EM DISCO
		AddFontResourceEx(TEXT("./fonts/KraashBlack.ttf"), FR_PRIVATE, NULL);
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight = 20;
		lf.lfWeight = SW_NORMAL;
		lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		_tcscpy(lf.lfFaceName, TEXT("Kraash Black"));
		hFont = CreateFontIndirect(&lf);


		// Caixa de Texto de Login
		hLogin = CreateWindow(TEXT("EDIT"),
			TEXT(""),
			WS_CHILD | WS_VISIBLE,
			390, 270,
			240, 30,
			hWnd,
			NULL,
			NULL,
			NULL);

		ShowWindow(hLogin, SW_HIDE);
		SendMessage(hLogin, WM_SETFONT, (WPARAM)hFont, (LPARAM)NULL);

		// Caixa de Texto de Password
		hPass = CreateWindow(TEXT("EDIT"),
			TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_PASSWORD,
			390, 325,
			240, 30,
			hWnd,
			NULL,
			NULL,
			NULL);
		ShowWindow(hPass, SW_HIDE);
		SendMessage(hPass, WM_SETFONT, (WPARAM)hFont, (LPARAM)NULL);

		
		// Ler todos os BITMAPS do JOGO
		home_bkg = (HBITMAP)LoadImage(NULL, TEXT("./images/home_bkg.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		game_over = (HBITMAP)LoadImage(NULL, TEXT("./images/game_over.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		game_bkg = (HBITMAP)LoadImage(NULL, TEXT("./images/game_bkg.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		gameCreditos = (HBITMAP)LoadImage(NULL, TEXT("./images/gameCreditos.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		gameFim = (HBITMAP)LoadImage(NULL, TEXT("./images/gameFim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		gameHighscores = (HBITMAP)LoadImage(NULL, TEXT("./images/gameHighScores.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		ErroMaxPlayer = (HBITMAP)LoadImage(NULL, TEXT("./images/ErroMaxPlayer.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		gameLogin = (HBITMAP)LoadImage(NULL, TEXT("./images/game_login.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		gameRegisto = (HBITMAP)LoadImage(NULL, TEXT("./images/game_registo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginOKON = (HBITMAP)LoadImage(NULL, TEXT("./images/botao_OKON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginOKOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/botao_OKOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginCancelON = (HBITMAP)LoadImage(NULL, TEXT("./images/botao_CancelON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginCancelOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/botao_CancelOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/loginOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		registarOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/registarOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		jogarOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/jogarOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		jogarUN = (HBITMAP)LoadImage(NULL, TEXT("./images/jogarUN.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		highscoresOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/highscoresOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		highscoresUN = (HBITMAP)LoadImage(NULL, TEXT("./images/highscoresUN.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		sairOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/sairOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		loginON = (HBITMAP)LoadImage(NULL, TEXT("./images/loginON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		registarON = (HBITMAP)LoadImage(NULL, TEXT("./images/registarON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		jogarON = (HBITMAP)LoadImage(NULL, TEXT("./images/jogarON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		highscoresON = (HBITMAP)LoadImage(NULL, TEXT("./images/highscoresON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		sairON = (HBITMAP)LoadImage(NULL, TEXT("./images/sairON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		creditosON = (HBITMAP)LoadImage(NULL, TEXT("./images/creditosON.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		creditosOFF = (HBITMAP)LoadImage(NULL, TEXT("./images/creditosOFF.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		home_bkg = (HBITMAP)LoadImage(NULL, TEXT("./images/home_bkg.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		perdeVida = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado_morte.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombasman = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombasman2 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombasman3 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado3.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombasman4 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado4.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		inimigo = (HBITMAP)LoadImage(NULL, TEXT("./images/inimigo_01.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		inimigo2 = (HBITMAP)LoadImage(NULL, TEXT("./images/inimigo_02.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bomba = (HBITMAP)LoadImage(NULL, TEXT("./images/bomba.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombasmanfundo = (HBITMAP)LoadImage(NULL, TEXT("./images/chao.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		parede = (HBITMAP)LoadImage(NULL, TEXT("./images/parede.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		vidaBonus = (HBITMAP)LoadImage(NULL, TEXT("./images/vida_bonus.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombaBonus = (HBITMAP)LoadImage(NULL, TEXT("./images/bomba_bonus.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bombaNuke = (HBITMAP)LoadImage(NULL, TEXT("./images/bomba_nuke.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		chave = (HBITMAP)LoadImage(NULL, TEXT("./images/chave.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		portal = (HBITMAP)LoadImage(NULL, TEXT("./images/portal.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		expCima = (HBITMAP)LoadImage(NULL, TEXT("./images/exp_cima.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		expBaixo = (HBITMAP)LoadImage(NULL, TEXT("./images/exp_baixo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		expEsq = (HBITMAP)LoadImage(NULL, TEXT("./images/exp_esq.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		expDir = (HBITMAP)LoadImage(NULL, TEXT("./images/exp_dir.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		expCentro = (HBITMAP)LoadImage(NULL, TEXT("./images/exp_centro.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		caixa = (HBITMAP)LoadImage(NULL, TEXT("./images/caixa.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		// To get max width and height of window
		maxX = GetSystemMetrics(SM_CXSCREEN); // Max X
		maxY = GetSystemMetrics(SM_CYSCREEN); // Max y
		
		hdc = GetDC(hWnd);
		memDC = CreateCompatibleDC(hdc);
		hbit = CreateCompatibleBitmap(hdc, maxX, maxY); // Faz um printscreen de todo o ecrã e cria um nitmap
		SelectObject(memDC, hbit);
		ReleaseDC(hWnd, hdc);
	break;

	case WM_PAINT:
		auxdc = CreateCompatibleDC(memDC);

		switch (gameSTATUS)
		{

		// Carrega e Desenha BITMAPS do Menu inicial
		case HOME:
			SelectObject(auxdc, home_bkg);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);
			if (menuOP == 0)
				SelectObject(auxdc, loginON);
			else
				SelectObject(auxdc, loginOFF);
			BitBlt(memDC, 760, 145, 235, 80, auxdc, 0, 0, SRCCOPY);

			if (menuOP == 1)
				SelectObject(auxdc, registarON);
			else
				SelectObject(auxdc, registarOFF);
			BitBlt(memDC, 760, 225, 235, 80, auxdc, 0, 0, SRCCOPY);

			if (loginSTATUS)
			{
				if (menuOP == 2)
					SelectObject(auxdc, jogarON);
				else
					SelectObject(auxdc, jogarOFF);
				BitBlt(memDC, 760, 305, 235, 80, auxdc, 0, 0, SRCCOPY);
			}
			else
			{
				SelectObject(auxdc, jogarUN);
				BitBlt(memDC, 760, 305, 235, 80, auxdc, 0, 0, SRCCOPY);
			}

			if (menuOP == 3)
				SelectObject(auxdc, highscoresON);
			else
				SelectObject(auxdc, highscoresOFF);
			BitBlt(memDC, 760, 385, 235, 80, auxdc, 0, 0, SRCCOPY);

			if (menuOP == 4)
				SelectObject(auxdc, sairON);
			else
				SelectObject(auxdc, sairOFF);
			BitBlt(memDC, 760, 465, 235, 80, auxdc, 0, 0, SRCCOPY);

			if (menuOP == 5)
				SelectObject(auxdc, creditosON);
			else
				SelectObject(auxdc, creditosOFF);
			BitBlt(memDC, 149, 559, 173, 61, auxdc, 0, 0, SRCCOPY);

			break;

		// Carrega e desenha BITMAPS do JOGO
		case JOGAR:
			SelectObject(auxdc, game_bkg);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);
			// Desenha o Chao
			for (i = 0; i < 13; i++)
				for (j = 0; j < 15; j++)
				{
					SelectObject(auxdc, bombasmanfundo);
					BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
				}

			switch (numPlayer)
			{
			case PLAYER1:
				switch (dirPlayer)
				{
				case BAIXO:
					bombasman = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case ESQUERDA:
					bombasman = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado_esq.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case DIREITA:
					bombasman = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado_dir.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case CIMA:
					bombasman = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado_cim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				}
				break;
			case PLAYER2:
				switch (dirPlayer)
				{
				case BAIXO:
					bombasman2 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case ESQUERDA:
					bombasman2 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado2_esq.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case DIREITA:
					bombasman2 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado2_dir.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case CIMA:
					bombasman2 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado2_cim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				}
				break;
			case PLAYER3:
				switch (dirPlayer)
				{
				case BAIXO:
					bombasman3 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado3.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case ESQUERDA:
					bombasman3 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado3_esq.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case DIREITA:
					bombasman3 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado3_dir.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case CIMA:
					bombasman3 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado3_cim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				}
				break;
			case PLAYER4:
				switch (dirPlayer)
				{
				case BAIXO:
					bombasman4 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado4.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case ESQUERDA:
					bombasman4 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado4_esq.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case DIREITA:
					bombasman4 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado4_dir.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				case CIMA:
					bombasman4 = (HBITMAP)LoadImage(NULL, TEXT("./images/soldado4_cim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					break;
				}
				break;
			}

			// Desenha Matriz & Inimigos
			for (i = 0; i < 13; i++)
				for (j = 0; j < 15; j++)
				{
					switch (tabuleiro[i][j])
					{
					case PLAYER1:
						SelectObject(auxdc, bombasman);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case PLAYER2:
						SelectObject(auxdc, bombasman2);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case PLAYER3:
						SelectObject(auxdc, bombasman3);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
					case PLAYER4:
						SelectObject(auxdc, bombasman4);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case INIMIGO1:
						SelectObject(auxdc, inimigo);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case INIMIGO2:
						SelectObject(auxdc, inimigo2);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BOMBA:
						SelectObject(auxdc, bomba);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BONUSVIDA:
						SelectObject(auxdc, vidaBonus);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BONUSNUKE:
						SelectObject(auxdc, bombaNuke);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BONUSBOMBAS:
						SelectObject(auxdc, bombaBonus);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case PORTAL:
						SelectObject(auxdc, portal);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case CHAVE:
						SelectObject(auxdc, chave);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case CAIXA:
						SelectObject(auxdc, caixa);
						BitBlt(memDC, j*LARGURA, i*LARGURA - 12, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case PAREDE:
						SelectObject(auxdc, parede);
						BitBlt(memDC, j*LARGURA, i*LARGURA-12, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BLAST_CENTRO:
						SelectObject(auxdc, expCentro);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BLAST_DIR:
						SelectObject(auxdc, expDir);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BLAST_ESQ:
						SelectObject(auxdc, expEsq);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BLAST_CIMA:
						SelectObject(auxdc, expCima);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					case BLAST_BAIXO:
						SelectObject(auxdc, expBaixo);
						BitBlt(memDC, j*LARGURA, i*LARGURA, LARGURA, ALTURA, auxdc, 0, 0, SRCCOPY);
						break;
					default:
						break;
					}

				}
				// Escrever PONTUAÇAO
				escreveNum(pontos, 840, 245);


				//Escrever NIVEL | VIDAS | BOMBAS -> SCOREBOARD
				escreveNum(nivel, 860, 140);
				escreveNum(vidas, 840, 325);
				escreveNum(bombas, 945, 325);
				escreveNum(countChaves, 840, 395);
				break;

		case LOGIN:
			SelectObject(auxdc, gameLogin);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);

			// CAIXAS DE TEXTO

			if (loginOP == 0)
				SelectObject(auxdc, loginOKON);
			else
				SelectObject(auxdc, loginOKOFF);
			BitBlt(memDC, 588, 368, 39, 30, auxdc, 0, 0, SRCCOPY);

			if (loginOP == 1)
				SelectObject(auxdc, loginCancelON);
			else
				SelectObject(auxdc, loginCancelOFF);
			BitBlt(memDC, 464, 368, 109, 30, auxdc, 0, 0, SRCCOPY);
			break;

		case REGISTO:
			SelectObject(auxdc, gameRegisto);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);

			// CAIXAS DE TEXTO

			if (loginOP == 0)
				SelectObject(auxdc, loginOKON);
			else
				SelectObject(auxdc, loginOKOFF);
			BitBlt(memDC, 588, 368, 39, 30, auxdc, 0, 0, SRCCOPY);

			if (loginOP == 1)
				SelectObject(auxdc, loginCancelON);
			else
				SelectObject(auxdc, loginCancelOFF);
			BitBlt(memDC, 464, 368, 109, 30, auxdc, 0, 0, SRCCOPY);
			break;

		case GAMEOVER:
			SelectObject(auxdc, game_over);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);
			break;

		case ERROMAXPLAYERS:
			SelectObject(auxdc, ErroMaxPlayer);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);
			break;

		case TABELASCORES:
			SelectObject(auxdc, gameHighscores);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);
			SelectObject(memDC, hFont);
			SetBkMode(memDC, TRANSPARENT);
			SetTextColor(memDC, RGB(255, 204, 0));

			for (i = 0; i < totUserReg; i++)
			{
				TextOut(memDC, 300, 240 + (i * 30), utilizadoresHighscores[i].user, _tcslen(utilizadoresHighscores[i].user));
				TextOut(memDC, 490, 240 + (i * 30), utilizadoresHighscores[i].numJogos, _tcslen(utilizadoresHighscores[i].numJogos));
				TextOut(memDC, 640, 240 + (i * 30), utilizadoresHighscores[i].maxPontuacao, _tcslen(utilizadoresHighscores[i].maxPontuacao));
			}
			break;

		case CREDITOS:
			SelectObject(auxdc, gameCreditos);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);

		case FINNISH:
			SelectObject(auxdc, gameFim);
			BitBlt(memDC, 0, 0, 1000, 700, auxdc, 0, 0, SRCCOPY);

		}

		DeleteDC(auxdc);
		hdc = BeginPaint(hWnd, &area);
		BitBlt(hdc, 0, 0, maxX, maxY, memDC, 0, 0, SRCCOPY); // Pinta todo o ecrã de jogo duma só vez
		EndPaint(hWnd, &area);

	break;

	case WM_DESTROY:	// Destruir a janela e terminar o programa
		pedido = SAIR;
		WriteFile(hPipe, &pedido, sizeof(pedido), &nBytesWrite, NULL);
		Sleep(1000);
		PostQuitMessage(0);
		break;
	default:
// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar", "maximizar",
// "restaurar") não é efectuado nenhum processamento, apenas se segue 
// o "default" do Windows DefWindowProc()
			return(DefWindowProc(hWnd,messg,wParam,lParam));
			break;
	}

	return(0);
}


void escreveNum(TCHAR* num, int x, int y)
{
	HBITMAP numero;
	int i = 0;


	while (*num != 0)
	{
		//if (_tcsncmp(num, TEXT("0"), 1) == 0)
		numero = (HBITMAP)LoadImage(NULL, TEXT("./images/0.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("1"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("2"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("3"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/3.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("4"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/4.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("5"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/5.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("6"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/6.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("7"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/7.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("8"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/8.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (_tcsncmp(num, TEXT("9"), 1) == 0)
			numero = (HBITMAP)LoadImage(NULL, TEXT("./images/9.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		SelectObject(auxdc, numero);
		BitBlt(memDC, x+(i*32), y, 31, 40, auxdc, 0, 0, SRCCOPY);

		i++;
		num++;
	}

}


DWORD WINAPI actualizaJogo(LPVOID param)	// Função responsável pelo processamento de autenticação do login
{
	DWORD nBytesWrite;
	DWORD nBytesRead;
	HWND hWnd = (HWND)param;
	TCHAR opcao[MAX_BUFFER];

	while (1)
	{
		_tcscpy(opcao, TEXT("ACTUALIZA"));
		WriteFile(hPipeJogo, opcao, _tcslen(opcao)*sizeof(TCHAR), &nBytesWrite, NULL);
		ReadFile(hPipeJogo, tabuleiro, sizeof(tabuleiro), &nBytesRead, NULL);
		InvalidateRect(hWnd, NULL, 0);
		Sleep(500);
	}

	return 0;
}