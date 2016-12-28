// TicTacToe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TicTacToe.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// Game global variables
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3];

BOOL GetGameBoardRect(HWND hWnd, RECT *pRect)				// user-defined function
{
	RECT rc;
	if (GetClientRect(hWnd, &rc))
	{
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;
		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;

		return TRUE;
	}
	SetRectEmpty(pRect);
	return FALSE;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)		// user-defined function
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND hWnd, int x, int y)			// user-defined function
{
	POINT pt = { x, y };
	RECT rc;

	if (GetGameBoardRect(hWnd, &rc))
	{
		if (PtInRect(&rc, pt))		// if user clicked inside game board
		{
			// normalise
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			// convert to index (0 to 8)
			return (column + row * 3);
		}
	}
	return -1;		// click outside game board or failure
}

BOOL GetCellRect(HWND hWnd, int index, RECT *pRect)			// user-defined function
{
	RECT rcBoard;

	SetRectEmpty(pRect);
	if (index < 0 || index>8)
	{
		return FALSE;
	}
	if (GetGameBoardRect(hWnd, &rcBoard))
	{
		// From index, convert to (x, y) pair
		int x = index % 3;		// column number
		int y = index / 3;		// row number

		pRect->left = rcBoard.left + (x * CELL_SIZE) + 1;
		pRect->top = rcBoard.top + (y * CELL_SIZE) + 1;
		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;

		return TRUE;
	}

	return FALSE;
}

/*
 Returns:
 0 - No winner
 1 - Player 1 wins
 2 - Player 2 wins
 3 - Game draw
*/
int GetWinner(int wins[3])
{
	int cells[] = { 0,1,2,  3,4,5,  6,7,8,  0,3,6,  1,4,7,  2,5,8,  0,4,8,  2,4,6 };
	
	// Check for winner
	for (int i = 0; i < ARRAYSIZE(cells); i += 3)
	{
		if ((gameBoard[cells[i]] != 0) && (gameBoard[cells[i]] == gameBoard[cells[i + 1]]) && (gameBoard[cells[i]] == gameBoard[cells[i + 2]]))
		{
			// We have a winner
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}

	// To check if we have empty cells left
	for (int i = 0; i < ARRAYSIZE(gameBoard); ++i)
	{
		if (0 == gameBoard[i])
		{
			return 0;	// continue playing
		}
	}

	return 3;	// draw
}

void ShowTurn(HWND hWnd, HDC hdc)
{
	RECT rc;

	static const WCHAR szTurn1[] = L"Turn: Player 1";
	static const WCHAR szTurn2[] = L"Turn: Player 2";
	const WCHAR *pszTurnText = NULL;

	switch (winner)
	{
	case 0:	// Continue playing
		pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
		break;
	case 1:	// Player 1 wins
		pszTurnText = L"Player 1 is the winner!";
		break;
	case 2:	// Player 2 wins
		pszTurnText = L"Player 2 is the winner!";
		break;
	case 3:	// Draw
		pszTurnText = L"It's a draw!";
		break;
	}

	if ((pszTurnText != NULL) && (GetClientRect(hWnd, &rc)))
	{
		rc.top = rc.bottom - 48;
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
	}
}

void DrawIconCentered(HDC hdc, RECT *pRect, HICON hIcon)
{
	const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
	const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);

	if (pRect != NULL)
	{
		int x = pRect->left + ((pRect->right - pRect->left) - ICON_WIDTH) / 2;
		int y = pRect->top + ((pRect->bottom - pRect->top) - ICON_HEIGHT) / 2;
		DrawIcon(hdc, x, y, hIcon);
	}
}

void HighlightWinner(HWND hWnd, HDC hdc)
{
	RECT rcWin;

	for (int i = 0; i < 3; ++i)
	{
		if (GetCellRect(hWnd, wins[i], &rcWin))
		{
			FillRect(hdc, &rcWin, ((winner == 2) ? hbr2 : hbr1));
			DrawIconCentered(hdc, &rcWin, ((winner == 1) ? hIcon1 : hIcon2));
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			hbr1 = CreateSolidBrush(RGB(255, 160, 0));		// RGB equivalent of orange
			hbr2 = CreateSolidBrush(RGB(255, 102, 178));	// RGB equivalent of a shade of pink

			// Load player icons
			hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_Player1));
			hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_Player2));
		}
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_NEWGAME:
				{
					int ret = MessageBox(hWnd, L"Are you sure?", L"New Game", MB_YESNOCANCEL | MB_ICONQUESTION);

					if (ret == IDYES)			// Reset game
					{
						// Reset and start new game
						playerTurn = 1;
						winner = 0;
						ZeroMemory(gameBoard, sizeof(gameBoard));
						// Force a paint
						InvalidateRect(hWnd, NULL, TRUE);	// Post WM_PAINT to WndProc, which gets queued in the message queue
						UpdateWindow(hWnd);					// Update window to reflect change immediately for WM_PAINT
					}
					else if (ret == IDNO) {}	// Do nothing
					else {}						// Do nothing
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_LBUTTONDOWN:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			// Only handle clicks if it's a player's turn (i.e. 1 or 2)
			if (playerTurn == 0)
			{
				break;
			}
			
			int index = GetCellNumberFromPoint(hWnd, xPos, yPos);

			HDC hdc = GetDC(hWnd);
			if (hdc != NULL)
			{
				//WCHAR temp[100];

				//wsprintf(temp, L"Index = %d", index);			// for debugging
				//TextOut(hdc, xPos, yPos, temp, lstrlen(temp));

				// Get cell dimension from its index
				if (index != -1)
				{
					RECT rcCell;
					if ((gameBoard[index] == 0) && GetCellRect(hWnd, index, &rcCell))
					{
						gameBoard[index] = playerTurn;
						//FillRect(hdc, &rcCell, (HBRUSH)GetStockObject(WHITE_BRUSH));
						/*
						SetDCBrushColor(hdc, RGB(255, 160, 0));		// RGB equivalent of orange
						FillRect(hdc, &rcCell, (HBRUSH)GetStockObject(DC_BRUSH));
						*/
						//FillRect(hdc, &rcCell, ((playerTurn==2) ? hbr2 : hbr1));
						//DrawIcon(hdc, rcCell.left, rcCell.top, ((playerTurn == 1) ? hIcon1 : hIcon2));
						DrawIconCentered(hdc, &rcCell, ((playerTurn == 1) ? hIcon1 : hIcon2));

						// Check if we have a winner
						winner = GetWinner(wins);
						if (winner == 1 || winner == 2)
						{
							HighlightWinner(hWnd, hdc);
							MessageBox(hWnd, 
									   (winner == 1) ? L"Player 1 wins!" : L"Player 2 wins!", 
									   L"You win!", 
									   MB_OK | MB_ICONINFORMATION);	// L for unicode
							playerTurn = 0;		// Don't want to keep on playing
						}
						else if (winner == 3)
						{
							// Draw
							MessageBox(hWnd, L"Draw!", L"Draw", MB_OK | MB_ICONEXCLAMATION);
							playerTurn = 0;		// Don't want to keep on playing
						}
						else
						{
							// Keep playing
							playerTurn = (playerTurn == 2) ? 1 : 2;
						}

						// Display player turn
						ShowTurn(hWnd, hdc);

					}
				}

				ReleaseDC(hWnd, hdc);
			}
		}
		break;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *pMinMax = (MINMAXINFO*)lParam;

			pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
			pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			RECT rc;

			// Player 1 & player 2 text


			if (GetGameBoardRect(hWnd, &rc))
			{
				RECT rcClient;

				// Display player text and turn
				if (GetClientRect(hWnd, &rcClient))
				{
					const WCHAR szPlayer1[] = L"Player 1";
					const WCHAR szPlayer2[] = L"Player 2";

					SetBkMode(hdc, TRANSPARENT);

					// Draw player 1 & player 2 text
					SetTextColor(hdc, RGB(255, 255, 0));	// RGB equivalent of yellow
					TextOut(hdc, 16, 16, szPlayer1, ARRAYSIZE(szPlayer1));
					DrawIcon(hdc, 24, 40, hIcon1);

					SetTextColor(hdc, RGB(128, 255, 0));	// RGB equivalent of neon green
					TextOut(hdc, rcClient.right - 72, 16, szPlayer2, ARRAYSIZE(szPlayer2));
					DrawIcon(hdc, rcClient.right - 64, 40, hIcon2);

					// Display player turn
					ShowTurn(hWnd, hdc);
				}
				
				//Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);		// white board w/ black border
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));	// white board w/ white border
			}

			for (int i = 0; i < 4; ++i)		// loop to draw grid - to be used with FillRect [for white board w/ white border]
			//for (int i = 1; i < 3; ++i)	// loop to draw grid - to be used with Rectangle [for white board w/ black border]
			{
				// Horizontal lines of grid
				DrawLine(hdc, rc.left, rc.top + (CELL_SIZE * i), rc.right, rc.top + (CELL_SIZE * i));

				// Vertical lines of grid
				DrawLine(hdc, rc.left + (CELL_SIZE * i), rc.top, rc.left + (CELL_SIZE * i), rc.bottom);
			}

			// Draw all occupied cells (even after resizing window)
			RECT rcCell;
			for (int i = 0; i < ARRAYSIZE(gameBoard); ++i)
			{
				if ((gameBoard[i] != 0) && GetCellRect(hWnd, i, &rcCell))
				{
					//FillRect(hdc, &rcCell, ((gameBoard[i] == 2) ? hbr2 : hbr1));
					DrawIconCentered(hdc, &rcCell, ((gameBoard[i] == 1) ? hIcon1 : hIcon2));
				}
			}

			// Highlight winner (even after resizing window)
			if (winner == 1 || winner == 2)
			{
				HighlightWinner(hWnd, hdc);
			}

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteObject(hbr1);
		DeleteObject(hbr2);
		DestroyIcon(hIcon1);
		DestroyIcon(hIcon2);
		PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
