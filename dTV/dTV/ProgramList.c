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
// 26 Dec 2000   Eric Schmidt          Made it possible to have whitespace in
//                                     your channel names in program.txt.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgramList.h"
#include "tuner.h"
#include "bt848.h"
#include "dTV.h"

int CurSel;
unsigned short SelectButton;
int EditProgramm;
char KeyValue;
HWND ProgList;


BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i, j, k;
	static BOOL NextBlock = FALSE;
	LOGFONT Mfont = { 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Times New Roman" };
	RECT ptrtoWndPos;
	static int currX, currY;
	int fwKeys = wParam;		// key flags
	static HCURSOR hSaveCursor = NULL;
	static HCURSOR hsizex;
	static int distance = -9999;
	static int sPos;
	static int EndPos;
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
								// LBS_OWNERDRAWVARIABLE |
								  WS_VSCROLL, 
							  // | WS_HSCROLL,
								//    WS_HSCROLL |
								//    LBS_SORT ,
								//     LBS_DISABLENOSCROLL ,
								0, 0, 0, 0, hDlg, NULL, hInst, NULL);

		SetCapture(hDlg);

		CurSel = CurrentProgramm;
		if (pgstarty != -1)
		{
			MoveWindow(hDlg, pgstartx, pgstarty, pgsizex, pgsizey, FALSE);
		}

		SendMessage(ProgList, LB_RESETCONTENT, 0, 0);

		i = 0;

		while (i < MAXPROGS)
		{
			if (Programm[i].freq != 0)
			{
				k = SendMessage(ProgList, LB_ADDSTRING, 0, (LPARAM) Programm[i].Name);
			}
			else
				break;
			i++;
		}

		SendMessage(ProgList, LB_SETCURSEL, CurSel, (LPARAM) 0);

    SetFocus(ProgList); 
 
		break;

	case WM_SIZE:
		GetWindowRect(hDlg, &ptrtoWndPos);
		currY = ptrtoWndPos.bottom - ptrtoWndPos.top;
		currX = ptrtoWndPos.right - ptrtoWndPos.left;
		currY = currY - 90;
		currX = currX - 25;;
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
		while (i < 15)
		{
			ButtonList[i].s = 0;
			if (ButtonList[i].FieldId > -1)
			{
				if (j < currX - 12)
				{
					ShowWindow(GetDlgItem(hDlg, IDC_MOVE1 + i), SW_SHOW);

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

			i++;
		}

		MoveWindow(ProgList, 6, 25, currX, currY, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDOK2), 25, 40 + currY, 60, 20, TRUE);

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

				if ((i >= 0) && (i < MAXPROGS))
				{
					if (i != CurrentProgramm)
					{
						ChangeChannel(i);
					}
				}
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


void Write_Program_List()
{
	int i, j;
	int fd;

	if ((fd = open("program.set", _O_WRONLY | _O_TRUNC | _O_CREAT | _O_BINARY, _S_IWRITE | _S_IREAD)) == -1)
	{
		ErrorBox("Can't create program list \"program.set\"");
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

	if ((fd = open("program.set", _O_RDONLY | _O_BINARY)) == -1)
	{
		return;
	}

	while (sizeof(Programm[i]) == read(fd, &Programm[i], sizeof(Programm[i])))
	{
		i++;
	}
	close(fd);
}

// 
// Save ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//
void Write_Program_List_ASCII()
{
	int i, j;
	FILE *SettingFile;
	
	i = 0;
	while (i < MAXPROGS)
	{
		if (Programm[i].Name[0] == 0x00)
		{
			// i has the number of valid programs
			break;
		}
		i++;
	}

	if (i)
	{
		if ((SettingFile = fopen("program.txt", "w")) != NULL)
		{
			for (j=0;j<i;++j)
			{
				fprintf(SettingFile, "Name: %s\n", Programm[j].Name);
				fprintf(SettingFile, "Freq: %ld\n", Programm[j].freq);
			}

			fclose(SettingFile);
		}
	}
}

// 
// Load ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//

void Load_Program_List_ASCII()
{
	int i, j, ch;
	char sbuf[255];
	FILE *SettingFile;

	// Zero out the program list
	for (i = 0; i < MAXPROGS; i++)
	{
		memset(&Programm[i].Name[0], 0x00, sizeof(struct TProgramm));
	}
	
	if ((SettingFile = fopen("program.txt", "r")) != NULL)
	{
		i=0;

		while (1)
		{
			// Read the name (may contain white space)
            if (fscanf(SettingFile, "%s ", sbuf) == EOF) // Skip past "Name: "
                break;
            j = 0;
            while (1)
            {
                if (j == sizeof(sbuf) - 1 || (ch = getc(SettingFile)) == '\n')
                    break;

                if (ch == EOF)
                    goto error;

                if (ch != '\r')
                    sbuf[j++] = (char)ch;
            }
            sbuf[j] = '\0';
            strcpy(Programm[i].Name, sbuf);

			// Read the frequency
			if (fscanf(SettingFile, "%s %ld\n", sbuf, &Programm[i].freq) == EOF)
			{
				// Error condition - premature EOF
				goto error;
			}
			i++;
		}

		fclose(SettingFile);
	}
	return;

error:
	// Close the file
	fclose(SettingFile);
	
	// Reset program list
	for (i = 0; i < MAXPROGS; i++)
	{
		memset(&Programm[i].Name[0], 0x00, sizeof(struct TProgramm));
	}

}


BOOL APIENTRY AnalogScanProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char line[80];
	char Text[128];

	int i, j;
	
	static int progindex = 0;

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

		return (TRUE);

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &wps);
		hMemDC = CreateCompatibleDC(hdc);
		
		if(BT848_IsVideoPresent() == TRUE)
			hOldBm = SelectObject(hMemDC, RedBulb);
		else
			hOldBm = SelectObject(hMemDC, GreenBulb);

		GetObject(RedBulb, sizeof(BITMAP), (LPSTR) & bm);

		BitBlt(hdc, 170, 80, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);	// Signal

		SelectObject(hMemDC, hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		EndPaint(hDlg, &wps);
		return (FALSE);

	case WM_USER:
		if (wParam == 0x101)
		{
			if(Freq != 0)
			{
				sprintf(line, "%10.2f", (float) Freq / 1000);
				SetDlgItemText(hDlg, IDC_EDIT2, line);
				SetDlgItemText(hDlg, IDC_TEXT20, "Scanning...");

				i = 0;

				Sleep(100);

				while ((i < 75) && (BT848_IsVideoPresent() == FALSE))
				{
					i++;
					if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
					{
						SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
					}

					if (STOP == TRUE)
						return (TRUE);

					Sleep(3);
				}

				if (BT848_IsVideoPresent())
				{
					if (FirstFreq == 0)
						FirstFreq = Freq;

					SetDlgItemText(hDlg, IDC_TEXT20, "Video Signal Found");
					
					VPS_lastname[0] = 0x00;

					Packet30.Identifier[0] = 0x00;

					Programm[progindex].freq = Freq;
					
					Programm[progindex].Typ = 'A';
					
					Freq = Freq + 2500;
					if (Capture_VBI == TRUE)
					{
						strcpy(Programm[progindex].Name, "<No PDC>");

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
								strcpy(Programm[progindex].Name, VPS_lastname);
								i = 100;
							}
							else if (Packet30.Identifier[0] != 0x00)
							{
								strcpy(Programm[progindex].Name, Packet30.Identifier);
								i = 100;
							}
							i++;
						}
					}
					// MAE 7 Nov 2000 Added to get right channel names
					else
					{
						// MAE 7 Nov 2000 Added to get right channel names
						sprintf(Text,"%d",Channels.MinChannel+ChannelNr);

						strcpy(Programm[progindex].Name,Text);
					}
					
					progindex++;

					if (progindex > MAXPROGS)
					{
						ErrorBox("All storage space occupied");
						return (TRUE);
					}
				
					InvalidateRect(hDlg, NULL, FALSE);
					UpdateWindow(hDlg);

				}
			}

			ChannelNr++;

			if (STOP == TRUE)
				return (TRUE);

			if (ChannelNr <= (Channels.MaxChannel-Channels.MinChannel))
			{
				Freq = Channels.freq[ChannelNr];
				if (!Tuner_SetFrequency(TunerType, MulDiv((Freq * 1000), 16, 1000000)))
				{
					sprintf(Text, "SetFrequency %10.2f Failed.", (float) Freq / 1000);
					ErrorBox(Text);
					return (TRUE);
				}

				// MAE 7 Nov 2000 Added to get right channel names
				sprintf(Text,"%d",Channels.MinChannel+ChannelNr);
				SetDlgItemText(hDlg, IDC_EDIT15, Text);
				
				InvalidateRect(hDlg, NULL, FALSE);
				UpdateWindow(hDlg);

				if (STOP == FALSE)
				{
					PostMessage(hDlg, WM_USER, 0x101, 0);
				}
			}
			else
			{
				if (FirstFreq != 0)
				{
					(void) Tuner_SetFrequency(TunerType, MulDiv((Programm[0].freq * 1000), 16, 1000000));

					SetDlgItemText(hDlg, IDC_EDIT15, Programm[0].Name);
					
					InvalidateRect(hDlg, NULL, FALSE);
					UpdateWindow(hDlg);

					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
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

			Freq = Channels.freq[0];
			ChannelNr = 0;

			// Zero out the program list
			for (j = 0; j < MAXPROGS; j++)
			{
				memset(&Programm[j].Name[0], 0x00, sizeof(struct TProgramm));
			}

			progindex = 0;


			STOP = FALSE;
			sprintf(line, "%10.2f", (float) Freq / 1000);
			SetDlgItemText(hDlg, IDC_EDIT2, line);

			if (!Tuner_SetFrequency(TunerType, MulDiv((Freq * 1000), 16, 1000000)))
			{
				sprintf(Text, "Set Frequency %10.2f Mhz fialed", (float) Freq / 1000);
				SetWindowText(hwndTextField, Text);
			}

			// MAE 7 Nov 2000 Added to get right channel names
			sprintf(Text,"%d",Channels.MinChannel+ChannelNr);
			SetDlgItemText(hDlg, IDC_EDIT15, Text);
			
			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);

			PostMessage(hDlg, WM_USER, 0x101, 0);
			EnableWindow(GetDlgItem(hDlg, IDSTART), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

		}

		if (LOWORD(wParam) == IDOK)
		{
			//Write_Program_List();
			Write_Program_List_ASCII();
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			STOP = TRUE;
			//Load_Program_List();
			Load_Program_List_ASCII();
			EndDialog(hDlg, TRUE);
		}
		break;
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}
