/////////////////////////////////////////////////////////////////////////////
// ProgramList.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
/////////////////////////////////////////////////////////////////////////////

#include "ProgramList.h"
#include "other.h"

int CurSel;
unsigned short SelectButton;
int EditProgramm;
char KeyValue;
HWND ProgList;


BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char Text[128];
	int i, j, k, p;
	char zeile[140];
	struct TProgramm save;

	static BOOL NextBlock = FALSE;
	LOGFONT Mfont = { 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Times New Roman" };
	LPDRAWITEMSTRUCT lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	RECT ptrtoWndPos;
	static int currX, currY;
	int fwKeys = wParam;		// key flags 
	int xPos;					// horizontal position of cursor 
	int yPos;					// vertical position of cursor 
	static HCURSOR hSaveCursor = NULL;
	static HCURSOR hsizex;
	static int distance = -9999;
	static int sPos;
	static int EndPos;
	BOOL SetOK;
	static BOOL InitWin = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		hsizex = LoadCursor(hInst, "Csizex");

		ProgList = CreateWindow("LISTBOX", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE |
								// LBS_NOINTEGRALHEIGHT | 
								LBS_NOTIFY |
								// LBS_HASSTRINGS | 
								LBS_OWNERDRAWVARIABLE | WS_VSCROLL | WS_HSCROLL,
								//    WS_HSCROLL |
								//    LBS_SORT , 
								//       LBS_DISABLENOSCROLL ,
								0, 0, 0, 0, hDlg, NULL, hInst, NULL);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
		SetCapture(hDlg);

		CurSel = CurrentProgramm;
		if (pgstarty != -1)
		{
			MoveWindow(hDlg, pgstartx, pgstarty, pgsizex, pgsizey, FALSE);
		}

	case RESET_LIST:
		SendMessage(ProgList, LB_RESETCONTENT, 0, 0);

		i = 0;
		while (i < MAXPROGS)
		{
			if (Programm[i].freq != 0)
			{
				k = SendMessage(ProgList, LB_ADDSTRING, i, (LPARAM) i);
			}
			i++;
		}
		SendMessage(ProgList, LB_SETCURSEL, CurSel, (LPARAM) 0);

	case WM_SIZE:

		GetWindowRect(hDlg, &ptrtoWndPos);
		currY = ptrtoWndPos.bottom - ptrtoWndPos.top;
		currX = ptrtoWndPos.right - ptrtoWndPos.left;
		currY = currY - 90;
		currX = currX - 60;
		if (InitWin == FALSE)
		{
			pgstartx = ptrtoWndPos.left;
			pgstarty = ptrtoWndPos.top;
			pgsizex = ptrtoWndPos.right - ptrtoWndPos.left;
			pgsizey = ptrtoWndPos.bottom - ptrtoWndPos.top;
		}
		InitWin = FALSE;
		i = 0;
		j = 6;
		p = 0;
		while (i < 15)
		{
			ButtonList[i].s = 0;
			if (ButtonList[i].FeldId > -1)
			{

				if (j < currX - 12)
				{
					ShowWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), SW_SHOW);
					GetFeldName(ButtonList[i].FeldId, (char *) &zeile);
					SetWindowText(GetDlgItem(hDlg, IDC_MOVE1 + i), zeile);
					k = ButtonList[i].x;
					if (j + k > currX - 12)
						k = (currX - 12) - j;
					MoveWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), j, 2, k, 19, TRUE);
					ButtonList[i].s = j;
					ButtonList[i].r = k;
					j = j + k;
				}
				else
				{
					ShowWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), SW_HIDE);
				}
			}
			else
			{
				p++;
				ShowWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), SW_HIDE);
			}
			i++;
		}

		if (p > 0)
		{
			i = 0;
			while ((i < 15) && (ButtonList[i].FeldId > -1))
				i++;
			if (i < 15)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), SW_SHOW);
				SetWindowText(GetDlgItem(hDlg, IDC_MOVE1 + i), "...");
				MoveWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), j, 2, (currX - j) + 6, 19, TRUE);
				ButtonList[i].x = (currX - j) + 6;
				ButtonList[i].s = j;
				ButtonList[i].r = (currX - j) + 6;
			}
		}

		MoveWindow(ProgList, 6, 25, currX, currY, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDOK2), currX - 20, 40 + currY, 60, 20, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDM_NEU), 6, 40 + currY, 60, 20, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDC_BUTTON1), currX + 60 - 40, 25, 30, 20, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDC_BUTTON2), currX + 60 - 40, 25 + currY - 20, 30, 20, TRUE);

		break;

	case WM_MOVE:

		GetWindowRect(hDlg, &ptrtoWndPos);
		if (InitWin == FALSE)
		{
			pgstartx = ptrtoWndPos.left;
			pgstarty = ptrtoWndPos.top;
			pgsizex = ptrtoWndPos.right - ptrtoWndPos.left;
			pgsizey = ptrtoWndPos.bottom - ptrtoWndPos.top;
		}
		break;

	case WM_LBUTTONUP:
		xPos = LOWORD(lParam);	// horizontal position of cursor 
		yPos = HIWORD(lParam);	// vertical position of cursor 
		if ((yPos >= 2) && (yPos <= 21))
		{
			if ((distance == -9999) && (hSaveCursor == NULL))
			{
				i = 0;
				while (i < 15)
				{
					if ((xPos >= ButtonList[i].s + 2) && (xPos <= ButtonList[i].s + ButtonList[i].r - 4))
					{
						SelectButton = i;
						if (ButtonList[SelectButton].FeldId == -1)
						{
							j = 0;
							while ((j < 15) && (ButtonList[j].FeldId != -1))
								j++;
							SelectButton = j;
						}
						DialogBox(hInst, "LISTFELDSETUP", hDlg, (DLGPROC) ListFeldSettingProc);

						SendMessage(hDlg, RESET_LIST, 0, 0);
						return (TRUE);
					}
					i++;
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
		fwKeys = wParam;		// key flags 
		xPos = LOWORD(lParam);	// horizontal position of cursor 
		yPos = HIWORD(lParam);	// vertical position of cursor 
		SetOK = FALSE;
		if ((yPos >= 2) && (yPos <= 21))
		{

			if ((distance != -9999) && (fwKeys == MK_LBUTTON))
			{
				p = ButtonList[EndPos].x;
				k = ButtonList[EndPos + 1].x;
				p = p + (xPos - distance);
				k = k - (xPos - distance);
				distance = xPos;
				if ((p > 4) && (k > 4))
				{
					ButtonList[EndPos].x = p;
					ButtonList[EndPos + 1].x = k;
					ButtonList[EndPos].s = sPos;
					ButtonList[EndPos + 1].s = sPos + ButtonList[EndPos].x;
					MoveWindow(GetDlgItem(hDlg, IDC_MOVE1 + EndPos), sPos, 2, ButtonList[EndPos].x, 19, TRUE);
					MoveWindow(GetDlgItem(hDlg, IDC_MOVE1 + EndPos + 1), sPos + ButtonList[EndPos].x, 2, ButtonList[EndPos + 1].x, 19, TRUE);
					InvalidateRect(ProgList, NULL, TRUE);
					return (TRUE);
				}
			}
			distance = -9999;
			j = 4;
			sPos = 6;
			i = 0;
			while (i < 14)
			{
				if (ButtonList[i + 1].FeldId != -1)
				{
					j = j + ButtonList[i].x;
					if ((xPos >= j) && (xPos <= j + 4))
					{
						SetOK = TRUE;
						if (hSaveCursor == NULL)
						{
							hSaveCursor = SetCursor(hsizex);
							SetCapture(hDlg);
						}
						if (fwKeys == MK_LBUTTON)
						{
							if (distance == -9999)
							{
								distance = xPos;
								EndPos = i;
								return (TRUE);
							}
						}
					}
				}
				sPos = sPos + ButtonList[i].x;
				i++;
			}
		}
		if ((SetOK == FALSE) && (hSaveCursor != NULL))
		{
			SetCursor(hSaveCursor);
			ReleaseCapture();
			hSaveCursor = NULL;
		}

		break;

	case WM_DRAWITEM:
		/* Get pointer to the DRAWITEMSTRUCT */
		lpdis = (LPDRAWITEMSTRUCT) lParam;

		if (lpdis->itemID == -1)
		{
			/* We have a request to draw an item in the list box, yet there
			 * are no list box items. This is sent when the user TABS into
			 * an empty list box or an empty list box gets the focus. We
			 * have to indicate (somehow) that this owner-draw list box has
			 * the focus. We do it in response to this message. Note that
			 * lpdis->itemData field would be invalid in this instance so
			 * we can't allow it to fall into our standard routines.
			 */
			HandleFocusStateKanalListe(ProgList, lpdis);
		}
		else
		{
			switch (lpdis->itemAction)
			{
			case ODA_DRAWENTIRE:
				DrawEntireItemKanalListe(ProgList, lpdis, 0);
				break;

			case ODA_SELECT:
				HandleFocusStateKanalListe(ProgList, lpdis);
				break;

			case ODA_FOCUS:
				HandleFocusStateKanalListe(ProgList, lpdis);
				break;
			}
		}

		/* Return TRUE meaning that we processed this message. */
		return (TRUE);
		break;

	case WM_MEASUREITEM:
		lpmis = (LPMEASUREITEMSTRUCT) lParam;

		/* All the items are the same height since the list box style is
		 * LBS_OWNERDRAWFIXED
		 */
		lpmis->itemHeight = 14;
		return (TRUE);

	case WM_CHARTOITEM:
		if ((HWND) lParam == ProgList)
		{
			j = CurSel;
			NextBlock = TRUE;
			i = HIWORD(wParam);
			if ((LOWORD(wParam) >= 0x30) && (LOWORD(wParam) <= 0x39))
			{
				EditProgramm = i;
				KeyValue = (char) LOWORD(wParam);
				DialogBox(hInst, "KANALNUMMER", hDlg, (DLGPROC) KanalNummerProc);
				if (j != CurSel)
					NextBlock = FALSE;
			}
			else if ((LOWORD(wParam) == 0x64) || (LOWORD(wParam) <= 0x44))
			{
				for (j = i; j < MAXPROGS - 1; j++)
				{
					memcpy(&Programm[j].Name[0], &Programm[j + 1].Name[0], sizeof(struct TProgramm));
				}
				memset(&Programm[MAXPROGS - 1].Name[0], 0x00, sizeof(struct TProgramm));

				Write_Program_List();
			}

			PostMessage(ProgList, LB_SETCURSEL, CurSel, 0);

		}
		return (TRUE);

	case WM_COMMAND:

		if ((HWND) lParam == ProgList)
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				if (NextBlock == TRUE)
				{
					NextBlock = FALSE;
					return (TRUE);
				}
				i = SendMessage(ProgList, LB_GETCURSEL, 0, 0);
				if (i > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
				if (i + 1 < SendMessage(ProgList, LB_GETCOUNT, 0, 0))
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), FALSE);

				if ((i >= 0) && (i < MAXPROGS))
				{
					CurSel = i;
					if (i != CurrentProgramm)
					{
						if (ValidModes(Programm[i].Typ) == TRUE)
						{
							CurrentProgramm = i;

							if (bDisplayStatusBar == TRUE)
							{
								sprintf(Text, "%04d. %s ", CurrentProgramm + 1, Programm[CurrentProgramm].Name);
								SetWindowText(hwndKeyField, Text);
							}

							if (Programm[CurrentProgramm].Typ == 'A')
								Tuner_SetFrequency(TunerType, MulDiv(Programm[CurrentProgramm].freq * 1000, 16, 1000000));
						}
					}
				}
			}
		}

		if (LOWORD(wParam) == IDC_BUTTON2)
		{
			i = SendMessage(ProgList, LB_GETCURSEL, 0, 0);
			if (i + 1 < MAXPROGS)
			{
				if (Programm[i + 1].freq != 0)
				{
					memcpy(&save, &Programm[i], sizeof(struct TProgramm));
					memcpy(&Programm[i], &Programm[i + 1], sizeof(struct TProgramm));
					memcpy(&Programm[i + 1], &save, sizeof(struct TProgramm));

					CurSel++;
					SendMessage(hDlg, RESET_LIST, i + 1, 0);
					Write_Program_List();
				}
			}
		}

		if (LOWORD(wParam) == IDC_BUTTON1)
		{
			i = SendMessage(ProgList, LB_GETCURSEL, 0, 0);
			if (i >= 1)
			{
				memcpy(&save, &Programm[i], sizeof(struct TProgramm));
				memcpy(&Programm[i], &Programm[i - 1], sizeof(struct TProgramm));
				memcpy(&Programm[i - 1], &save, sizeof(struct TProgramm));

				CurSel--;
				Write_Program_List();
				SendMessage(hDlg, RESET_LIST, i - 1, 0);
			}
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
		}

		break;
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

BOOL APIENTRY KanalNummerProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i, j;
	struct TProgramm SAVE;

	switch (message)
	{
	case WM_INITDIALOG:

		SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
		SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_CHAR, KeyValue, 0);
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK)
		{

			i = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			i--;
			if ((i < 0) || (i >= MAXPROGS))
			{
				MessageBox(hDlg, "Illegal Chanel Number", "dTV", MB_ICONSTOP);
				break;
			}
			if (i != EditProgramm)
			{
				memcpy(&SAVE.Name[0], &Programm[EditProgramm].Name[0], sizeof(struct TProgramm));

				for (j = EditProgramm; j < MAXPROGS - 1; j++)
				{
					memcpy(&Programm[j].Name[0], &Programm[j + 1].Name[0], sizeof(struct TProgramm));
				}
				for (j = MAXPROGS - 2; j >= i; j--)
				{
					memcpy(&Programm[j + 1].Name[0], &Programm[j].Name[0], sizeof(struct TProgramm));
				}
				memcpy(&Programm[i].Name[0], &SAVE.Name[0], sizeof(struct TProgramm));

				Write_Program_List();
				CurSel = i;
			}

			EndDialog(hDlg, TRUE);
			break;
		}

		if (LOWORD(wParam) == IDDEL)
		{
			for (j = EditProgramm; j < MAXPROGS - 1; j++)
			{
				memcpy(&Programm[j].Name[0], &Programm[j + 1].Name[0], sizeof(struct TProgramm));
			}
			memset(&Programm[MAXPROGS - 1].Name[0], 0x00, sizeof(struct TProgramm));

			Write_Program_List();

			EndDialog(hDlg, TRUE);
			break;
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
			break;
		}
	}

	return (FALSE);
}


BOOL APIENTRY ListFeldSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i, j;
	struct TBL save;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 0, (LPARAM) (LPSTR) "Nummer");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 1, (LPARAM) (LPSTR) "Typ");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 2, (LPARAM) (LPSTR) "Name");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 3, (LPARAM) (LPSTR) "Frequenz");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 4, (LPARAM) (LPSTR) "Polarisation");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 5, (LPARAM) (LPSTR) "Video Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 6, (LPARAM) (LPSTR) "Audio Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 7, (LPARAM) (LPSTR) "Text Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 8, (LPARAM) (LPSTR) "SID Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 9, (LPARAM) (LPSTR) "PMT Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 10, (LPARAM) (LPSTR) "ECM Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 11, (LPARAM) (LPSTR) "EMM Pid");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 12, (LPARAM) (LPSTR) "Symbolrate");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 13, (LPARAM) (LPSTR) "FEC");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 14, (LPARAM) (LPSTR) "DiseqC");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 15, (LPARAM) (LPSTR) "Provider");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 16, (LPARAM) (LPSTR) "Land");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 17, (LPARAM) (LPSTR) "Transponder");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, 18, (LPARAM) (LPSTR) "Service-ID");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, ButtonList[SelectButton].FeldId, 0);
		SetDlgItemInt(hDlg, IDC_EDIT1, SelectButton + 1, FALSE);
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK)
		{
			i = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			if ((i >= 0) && (i <= 18))
				ButtonList[SelectButton].FeldId = i;
			i = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			i--;
			if ((i >= 0) && (i <= 18) && (i != SelectButton))
			{

				memcpy(&save.FeldId, &ButtonList[SelectButton].FeldId, 8);
				j = SelectButton;
				while (j < 14)
				{
					memcpy(&ButtonList[j].FeldId, &ButtonList[j + 1].FeldId, 8);
					j++;
				}
				j = 14;
				while (j >= i)
				{
					memcpy(&ButtonList[j + 1].FeldId, &ButtonList[j].FeldId, 8);
					j--;
				}
				memcpy(&ButtonList[i].FeldId, &save.FeldId, 8);
			}
			EndDialog(hDlg, TRUE);
			break;
		}

		if (LOWORD(wParam) == IDDEL)
		{
			j = SelectButton;
			while (j < 14)
			{
				memcpy(&ButtonList[j].FeldId, &ButtonList[j + 1].FeldId, 8);
				j++;
			}
			ButtonList[14].FeldId = -1;
			ButtonList[14].s = 0;
			ButtonList[14].x = 0;
			ButtonList[14].r = 0;

			EndDialog(hDlg, TRUE);
			break;
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
			break;
		}
	}

	return (FALSE);
}

VOID APIENTRY HandleFocusStateKanalListe(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
	int Typ;

	if ((lpdis->itemState & ODS_FOCUS) || (lpdis->itemState & ODS_SELECTED))
	{
		Typ = 1;
	}
	else
	{
		Typ = 0;
	}

	DrawEntireItemKanalListe(hDlg, lpdis, Typ);
}

VOID APIENTRY DrawEntireItemKanalListe(HWND hDlg, LPDRAWITEMSTRUCT lpdis, INT Typ)
{

	int i;
	char Zeile[64];
	HFONT OldFont;

	OldFont = SelectObject(lpdis->hDC, currFont);

	if (Typ == 0)
	{
		SetTextColor(lpdis->hDC, RGB(0, 0, 0));
		SetBkColor(lpdis->hDC, RGB(255, 255, 255));
	}
	else
	{
		SetTextColor(lpdis->hDC, RGB(255, 255, 255));
		SetBkColor(lpdis->hDC, RGB(0, 0, 255));
	}
	for (i = 0; i < 15; i++)
	{
		Zeile[0] = 0x00;
		if ((ButtonList[i].FeldId > -1) && (ButtonList[i].s != 0))
		{
			if (ButtonList[i].FeldId == 0)
				sprintf(Zeile, "%4d              ", lpdis->itemData + 1);
			else if (ButtonList[i].FeldId == 1)
			{
				if (Programm[lpdis->itemData].Typ == 'A')
					sprintf(Zeile, "Analog");
				else
				{
					if (Programm[lpdis->itemData].Video_pid != 0x1fff)
						sprintf(Zeile, "TV");
					else if (Programm[lpdis->itemData].Audio_pid != 0x1fff)
						sprintf(Zeile, "Radio");
					else
						sprintf(Zeile, "Daten");
					if (Programm[lpdis->itemData].CA_ID != 0)
						strcat(Zeile, "[C]");
				}
				strcat(Zeile, "                   ");
			}
			else if (ButtonList[i].FeldId == 2)
				sprintf(Zeile, "%s                ", Programm[lpdis->itemData].Name);
			else if (ButtonList[i].FeldId == 3)
				sprintf(Zeile, "%10.3f                  ", (float) Programm[lpdis->itemData].freq / 1000);
			if (Programm[lpdis->itemData].Typ == 'D')
			{
				if (ButtonList[i].FeldId == 4)
					if (Programm[lpdis->itemData].volt == 1)
						sprintf(Zeile, "Hor.                 ");	// Horizontal
					else
						sprintf(Zeile, "Ver.                 ");
				else if (ButtonList[i].FeldId == 5)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].Video_pid);
				else if (ButtonList[i].FeldId == 6)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].Audio_pid);
				else if (ButtonList[i].FeldId == 7)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].TeleText_pid);
				else if (ButtonList[i].FeldId == 8)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].SID_pid);
				else if (ButtonList[i].FeldId == 9)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].PMT_pid);
				else if (ButtonList[i].FeldId == 10)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].ECM_pid);
				else if (ButtonList[i].FeldId == 11)
					sprintf(Zeile, "0x%04x.                 ", Programm[lpdis->itemData].EMM_pid);
				else if (ButtonList[i].FeldId == 12)
					sprintf(Zeile, "%d                 ", Programm[lpdis->itemData].srate);
				else if (ButtonList[i].FeldId == 13)
				{
					if (Programm[lpdis->itemData].fec == 0)
						sprintf(Zeile, "1/2");
					else if (Programm[lpdis->itemData].fec == 1)
						sprintf(Zeile, "2/3");
					else if (Programm[lpdis->itemData].fec == 2)
						sprintf(Zeile, "3/4");
					else if (Programm[lpdis->itemData].fec == 3)
						sprintf(Zeile, "4/5");
					else if (Programm[lpdis->itemData].fec == 4)
						sprintf(Zeile, "5/6");
					else if (Programm[lpdis->itemData].fec == 5)
						sprintf(Zeile, "6/7");
					else if (Programm[lpdis->itemData].fec == 6)
						sprintf(Zeile, "7/8");
					else if (Programm[lpdis->itemData].fec == 7)
						sprintf(Zeile, "8/9");
					else if (Programm[lpdis->itemData].fec == 8)
						sprintf(Zeile, "Auto");
					strcat(Zeile, "                   ");
				}
				else if (ButtonList[i].FeldId == 14)
					sprintf(Zeile, "%d.                 ", Programm[lpdis->itemData].diseqc);
				else if (ButtonList[i].FeldId == 15)
					sprintf(Zeile, "%s                 ", Programm[lpdis->itemData].Anbieter);
				else if (ButtonList[i].FeldId == 16)
					sprintf(Zeile, "%s                 ", Programm[lpdis->itemData].Land);
				else if (ButtonList[i].FeldId == 17)
					sprintf(Zeile, "%d                 ", Programm[lpdis->itemData].tp_id);
				else if (ButtonList[i].FeldId == 18)
					sprintf(Zeile, "%d                 ", Programm[lpdis->itemData].ServiceTyp);
			}
			TextOut(lpdis->hDC, ButtonList[i].s - 3, lpdis->rcItem.top + 1, Zeile, strlen(Zeile));
		}
	}
	SelectObject(lpdis->hDC, OldFont);
	return;
}


void Write_Program_List()
{
	int i, j;
	int fd;

	if ((fd = open("Program.set", _O_WRONLY | _O_TRUNC | _O_CREAT | _O_BINARY, _S_IWRITE | _S_IREAD)) == -1)
	{
		MessageBox(hWnd, "Can't create program list \"program.set\"", "dTV", MB_ICONSTOP | MB_OK);
		return;
	}

	i = 0;
	while (i < MAXPROGS)
	{
		if (Programm[i].Name[0] != 0x00)
		{
			j = write(fd, &Programm[i], sizeof(struct TProgramm));
		}
		i++;
	}
	close(fd);
}


void Load_Program_List()
{
	int i;
	int fd;

	i = 0;

	for (i = 0; i < MAXPROGS; i++)
	{
		memset(&Programm[i].Name[0], 0x00, sizeof(struct TProgramm));
	}

	i = 0;
	if ((fd = open("Program.set", _O_RDONLY | _O_BINARY)) == -1)
	{
		return;
	}
	while (sizeof(Programm[i]) == read(fd, &Programm[i], sizeof(Programm[i])))
	{
		i++;
	}
	close(fd);
}

void GetFeldName(short id, char *zeile)
{
	zeile[0] = 0x00;
	if (id < 0)
		return;
	if (id > 18)
		return;

	switch (id)
	{

	case 0:
		strcpy(zeile, "Nummer");
		break;
	case 1:
		strcpy(zeile, "Typ");
		break;
	case 2:
		strcpy(zeile, "Name");
		break;
	case 3:
		strcpy(zeile, "Frequenz");
		break;
	case 4:
		strcpy(zeile, "Polarisation");
		break;
	case 5:
		strcpy(zeile, "Video Pid");
		break;
	case 6:
		strcpy(zeile, "Audio Pid");
		break;
	case 7:
		strcpy(zeile, "Text Pid");
		break;
	case 8:
		strcpy(zeile, "SID Pid");
		break;
	case 9:
		strcpy(zeile, "PMT Pid");
		break;
	case 10:
		strcpy(zeile, "ECM Pid");
		break;
	case 11:
		strcpy(zeile, "EMM Pid");
		break;
	case 12:
		strcpy(zeile, "Symbolrate");
		break;
	case 13:
		strcpy(zeile, "FEC");
		break;
	case 14:
		strcpy(zeile, "DiseqC");
		break;
	case 15:
		strcpy(zeile, "Provider");
		break;
	case 16:
		strcpy(zeile, "Land");
		break;
	case 17:
		strcpy(zeile, "Transponder");
		break;
	case 18:
		strcpy(zeile, "Service-ID");
		break;

	}
}

BOOL ValidModes(char Mode)
{
	return (Mode == 'A');
}


BOOL APIENTRY AnalogScanProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char zeile[80];
	char Text[128];

	int i, j;

	PAINTSTRUCT wps;			/* paint structure           */
	HDC hdc;
	HDC hMemDC;
	HBITMAP hOldBm;
	BITMAP bm;

	static BOOL STOP;
	static unsigned int Freq;
	static int ChannelNr;
	static unsigned int FirstFreq = 0;

	MSG msg;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		for (i = 0; i < 35; i++)
		{
			if (Countries[i].Name[0] != 0x00)
				SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, i, (LPARAM) (LPSTR) Countries[i].Name);
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, CountryCode, 0);
		Load_Country_Specific_Settings(CountryCode);
		CheckDlgButton(hDlg, IDC_CHECK1, TRUE);
		if (USETUNER == FALSE)
			EnableWindow(GetDlgItem(hDlg, IDSTART), TRUE);

		return (TRUE);

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &wps);
		hMemDC = CreateCompatibleDC(hdc);
		if (VideoPresent() == TRUE)
			hOldBm = SelectObject(hMemDC, BirneGruen);
		else
			hOldBm = SelectObject(hMemDC, BirneRot);
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR) & bm);

		BitBlt(hdc, 170, 80, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);	// Signal

		SelectObject(hMemDC, hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		EndPaint(hDlg, &wps);
		return (FALSE);

	case WM_USER:
		if (wParam == 0x101)
		{
			sprintf(zeile, "%10.2f", (float) Freq / 1000);
			SetDlgItemText(hDlg, IDC_EDIT2, zeile);
			SetDlgItemText(hDlg, IDC_TEXT20, "Scanne");
			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);
			i = 0;
			if (IsDlgButtonChecked(hDlg, IDC_CHECK1) == TRUE)
				Sleep(400);
			SetDlgItemText(hDlg, IDC_TEXT20, "Sync");
			while ((i < 75) && (VideoPresent() == FALSE))
			{
				i++;
				if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
				{
					SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
				}
				if (STOP == TRUE)
					return (TRUE);
				Sleep(1);
			}

			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);

			if (VideoPresent())
			{
				if (FirstFreq == 0)
					FirstFreq = Freq;
				SetDlgItemText(hDlg, IDC_TEXT20, "VideoSignal gefunden");
				VPS_lastname[0] = 0x00;
				Packet30.Identifier[0] = 0x00;
				j = 0;
				while ((j < MAXPROGS) && (Programm[j].freq != Freq))
					j++;

				if (j >= MAXPROGS)
				{
					j = 0;
					while ((j < MAXPROGS) && (Programm[j].freq != 0))
						j++;
					if (j > MAXPROGS)
					{
						MessageBox(hWnd, "All storage space occupied", "dTV", MB_ICONINFORMATION | MB_OK);
						return (TRUE);
					}
				}
				Programm[j].freq = Freq;
				strcpy(Programm[j].Name, "<No PDC>");
				Programm[j].Typ = 'A';
				Freq = Freq + 2500;
				if (Capture_VBI == TRUE)
				{
					i = 0;
					while (i < 100)
					{
						if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
						{
							SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
						}
						Sleep(2);
						if (VPS_lastname[0] != 0x00)
						{
							strcpy(Programm[j].Name, VPS_lastname);
							i = 100;
						}
						else if (Packet30.Identifier[0] != 0x00)
						{
							strcpy(Programm[j].Name, Packet30.Identifier);
							i = 100;
						}
						i++;
					}
				}

				SetDlgItemText(hDlg, IDC_EDIT15, Programm[j].Name);
			}

			if (STOP == TRUE)
				return (TRUE);
			if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
			{
				ChannelNr++;
				if (ChannelNr < Channels.MaxChannel)
				{
					Freq = Channels.freq[ChannelNr];
					if (!Tuner_SetFrequency(TunerType, MulDiv((Freq * 1000), 16, 1000000)))
					{
						sprintf(Text, "Frequency %10.2f Mhz not adjusted ", (float) Freq / 1000);
						SetWindowText(hwndTextField, Text);
						return (TRUE);
					}
					if (STOP == FALSE)
						PostMessage(hDlg, WM_USER, 0x101, 0);
				}
				else
				{
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDSTART), TRUE);
					if (FirstFreq != 0)
						(void) Tuner_SetFrequency(TunerType, MulDiv((FirstFreq * 1000), 16, 1000000));
				}
			}
			else
			{

				if (Freq < 870000)
				{
					Freq = Freq + 500;
					if (!Tuner_SetFrequency(TunerType, MulDiv((Freq * 1000), 16, 1000000)))
					{
						sprintf(Text, "Frequency %10.2f Mhz not adjusted ", (float) Freq / 1000);
						SetWindowText(hwndTextField, Text);
						return (TRUE);
					}
					if (STOP == FALSE)
						PostMessage(hDlg, WM_USER, 0x101, 0);
				}
				else
				{
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDSTART), TRUE);
					if (FirstFreq != 0)
						(void) Tuner_SetFrequency(TunerType, MulDiv((FirstFreq * 1000), 16, 1000000));
				}
			}
		}

		break;

	case WM_COMMAND:

		if ((HWND) lParam == GetDlgItem(hDlg, IDC_COMBO1))
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				CountryCode = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
				Load_Country_Specific_Settings(CountryCode);
			}
		}

		if (LOWORD(wParam) == IDSTART)
		{

			if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
			{
				Freq = Channels.freq[0];
				ChannelNr = 0;
			}
			else
			{
				Freq = 41000;
			}

			STOP = FALSE;
			sprintf(zeile, "%10.2f", (float) Freq / 1000);
			SetDlgItemText(hDlg, IDC_EDIT2, zeile);

			if (!Tuner_SetFrequency(TunerType, MulDiv((Freq * 1000), 16, 1000000)))
			{
				sprintf(Text, "Frequenz %10.2f Mhz nicht eingestellt ", (float) Freq / 1000);
				SetWindowText(hwndTextField, Text);
			}

			KillTimer(hWnd, 1);
			PostMessage(hDlg, WM_USER, 0x101, 0);
			EnableWindow(GetDlgItem(hDlg, IDSTART), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

		}

		if (LOWORD(wParam) == IDOK)
		{
			Write_Program_List();
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			STOP = TRUE;
			Load_Program_List();
			SetTimer(hWnd, 1, 2000, NULL);
			EndDialog(hDlg, TRUE);
		}

		break;
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}
