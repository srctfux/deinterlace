/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
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
// 09 Oct 2002   Denis Balazuc         Original Release
//                                     mostly cut/paste from ProgramList.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Channels.h"
#include "TVFormats.h"

CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format, BOOL Active)
{        
    m_Name = Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(const CChannel& CopyFrom)
{
    m_Name = CopyFrom.m_Name;
    m_Freq = CopyFrom.m_Freq;
    m_Chan = CopyFrom.m_Chan;
    m_Format = CopyFrom.m_Format;
    m_Active = CopyFrom.m_Active;
}

CChannel::~CChannel()
{
}

LPCSTR CChannel::GetName() const
{
    static char sbuf[256];
    strncpy(sbuf, m_Name.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;
}

 
DWORD CChannel::GetFrequency() const
{
    return m_Freq;
}

int CChannel::GetChannelNumber() const
{
    return m_Chan;
}

int CChannel::GetFormat() const
{
    return m_Format;
}

BOOL CChannel::IsActive() const
{
    return m_Active;
}

void CChannel::SetActive(BOOL Active)
{
    m_Active = Active;
}


CCountry::CCountry(LPCSTR szName)
{           
    m_Name = szName;
    m_MinChannel = 0;
    m_MaxChannel = 0;       
}

CCountry::~CCountry()
{
    m_Frequencies.clear();
}


int CCountry::GetMinChannelFrequency() const 
{    
    return m_Frequencies[GetMinChannelIndex()].Freq;
}

int CCountry::GetMaxChannelFrequency() const 
{
    return m_Frequencies[GetMaxChannelIndex()].Freq;
}


//just a wee helper..makes sure the index is correct by returning a more sensible one
int boundedChannelIndex(const CCountry * const pCountry, int iIndex) 
{
    int dummy = iIndex;
    if (dummy < 0) 
    {
        dummy = 0;
    }
    else if (dummy >= pCountry->GetSize()) 
    {
        dummy = pCountry->GetSize() - 1;
    }
    return dummy;
}


//We check bounds because sometimes,
//the number of frequs is lower than the declared higher rank
//(As for the [Argentina Cable Frequency] entries I have
int CCountry::GetMinChannelIndex() const 
{   
    return boundedChannelIndex(this, m_MinChannel - 1);
}

int CCountry::GetMaxChannelIndex() const 
{
    return boundedChannelIndex(this, m_MaxChannel - 1);
}



//Adds a New TCountryChannel and returns its position
int CCountry::AddChannel(DWORD dwFrequency, int iVideoFormat)
{
    TCountryChannel channel;
    channel.Freq = dwFrequency;
    channel.Format = iVideoFormat;

    m_Frequencies.push_back(channel);
   
    return m_Frequencies.size() - 1;
}

LPCSTR CCountry::GetName() const 
{
    static char sbuf[256];
    strncpy(sbuf, m_Name.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;    
}   



//That is an utility function that was in ProgramList.cpp
//Used in Load_Country_Settings
int StrToVideoFormat(char* pszFormat)
{
    for(int a = 0; a < VIDEOFORMAT_LASTONE; ++a)
    {
        if(!stricmp(pszFormat, VideoFormatNames[a]))
        {
            return a;
        }  
    }
   
    return -1;
}

//TODO - Transform into Class member or something
BOOL Load_Country_Settings(LPCSTR szFilename, COUNTRYLIST * pCountries)
{
    if ((NULL == szFilename) || (NULL == pCountries))
    {
        return FALSE;
    }
    
    FILE*     CountryFile;
    char      line[128];
    char*     Pos;
    char*     Pos1;
    char*     eol_ptr;
    string    Name;
    CCountry* NewCountry = NULL;
    int       Format = -1;
    
    
    if ((CountryFile = fopen(szFilename, "r")) == NULL)
    {
        return FALSE;
    }

    while (fgets(line, sizeof(line), CountryFile) != NULL)
    {
        eol_ptr = strstr(line, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(line, "\n");
        }
        if(eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = strstr(line, "[")) != 0) && ((Pos1 = strstr(line, "]")) != 0) && Pos1 > Pos)
        {
            if(NewCountry != NULL)
            {
                pCountries->push_back(NewCountry);
            }
            Pos++;
            
            //NewCountry = new CCountry();
            //NewCountry->m_Name = Pos;
            //NewCountry->m_Name[Pos1-Pos] = '\0';
            char * dummy = Pos;
            dummy[Pos1-Pos] = '\0';                        
            NewCountry = new CCountry((LPCSTR)dummy);
            Format = -1;
        }
        else
        {
            if (NewCountry == NULL)
            {
                fclose(CountryFile);                
                return FALSE;
            }
            
            if ((Pos = strstr(line, "ChannelLow=")) != 0)
            {
                NewCountry->SetMinChannel(atoi(Pos + strlen("ChannelLow=")));
            }
            else if ((Pos = strstr(line, "ChannelHigh=")) != 0)
            {
                NewCountry->SetMaxChannel(atoi(Pos + strlen("ChannelHigh=")));
            }
            else if ((Pos = strstr(line, "Format=")) != 0)
            {
                Format = StrToVideoFormat(Pos + strlen("Format="));
            }
            else
            {
                Pos = line;
                while (*Pos != '\0')
                {
                    if ((*Pos >= '0') && (*Pos <= '9'))
                    {                        
                        //TCountryChannel Channel;
                        //Channel.Freq = atol(Pos);
                        // convert frequency in KHz to Units that the tuner wants
                        //Channel.Freq = MulDiv(Channel.Freq, 16, 1000000);
                        //Channel.Format = Format;
                        NewCountry->AddChannel(atol(Pos), Format);
                        break;
                    }
                    Pos++;
                }
            }
        }
    }
    if(NewCountry != NULL)
    {
        pCountries->push_back(NewCountry);
    }

    fclose(CountryFile);
    return TRUE;
}

//TODO - Transform into Class member or something
void Unload_Country_Settings(COUNTRYLIST * pCountries)
{
    if (NULL == pCountries) 
    {
        return;
    }
    
    COUNTRYLIST::iterator it;

    // Zero out the program list
    for(it = pCountries->begin(); it != pCountries->end(); ++it)
    {
        delete (*it);
    }
    pCountries->clear();
}


void Unload_Program_List_ASCII(CHANNELLIST * pChannels)
{    
    if (NULL == pChannels) 
    {
        return;
    }

    CHANNELLIST::iterator it;
    for(it = pChannels->begin(); it != pChannels->end(); ++it)
    {
        delete (*it);
    }
    pChannels->clear();
}


//TODO - Transform into Class member or something
BOOL Load_Program_List_ASCII(LPCSTR szFilename, CHANNELLIST * pChannels)
{
    if ((NULL == szFilename) || (NULL == pChannels))
    {
        return FALSE;
    }

    char sbuf[256];
    FILE* SettingFile;  
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    BOOL Active = TRUE;
    string Name;

    
    //DB : Channels.cpp : Load_Program_List_ASCII() unload should be done elsewhere
    Unload_Program_List_ASCII(pChannels);

    SettingFile = fopen(szFilename, "r");
    if (SettingFile == NULL)
    {
        return FALSE;
    }
    while(!feof(SettingFile))
    {
        sbuf[0] = '\0';

        fgets(sbuf, 255, SettingFile);

        char* eol_ptr = strstr(sbuf, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(sbuf, "\n");
        }
        if (eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }


        if(strnicmp(sbuf, "Name:", 5) == 0)
        {
            if(Frequency != -1)
            {
                pChannels->push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
            }

            // skip "Name:"
            char* StartChar = sbuf + 5;

            // skip any spaces
            while(iswspace(*StartChar))
            {
                ++StartChar;
            }
            if(strlen(StartChar) > 0)
            {
                char* EndChar = StartChar + strlen(StartChar) - 1;
                while(EndChar > StartChar && iswspace(*EndChar))
                {
                    *EndChar = '\0';
                    --EndChar;
                }
                Name = StartChar;
            }
            else
            {
                Name = "Empty";
            }
            Frequency = -1;
            ++Channel;
            Format = -1;
            Active = TRUE;
        }
        // cope with old style frequencies
        else if(strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);           
            Frequency = Frequency * 1000;
        }
        else if(strnicmp(sbuf, "Freq2:", 6) == 0)
        {
            Frequency = atol(sbuf + 6);
            Frequency = MulDiv(Frequency, 1000000, 16);
        }
        else if(strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Active:", 7) == 0)
        {
            Active = (atoi(sbuf + 7) != 0);
        }
        else
        {
            ; //some other rubbish
        }
    }

    if(Frequency != -1)
    {
        pChannels->push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
    }

    fclose(SettingFile);
    return TRUE;
}


// 
// Save ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
// 10 October 2002 - Denis Balazuc
// Moved to Channels.cpp

BOOL Write_Program_List_ASCII(LPCSTR szFilename, CHANNELLIST * pChannels)
{
    if ((NULL == szFilename) || (NULL == pChannels))
    {
        return FALSE;
    }
    
    BOOL bSuccess = FALSE;
    FILE* SettingFile;
    CHANNELLIST::iterator it;
    
    if ((SettingFile = fopen(szFilename, "w")) != NULL)
    {
        for(it = pChannels->begin(); it != pChannels->end(); ++it)
        {
            fprintf(SettingFile, "Name: %s\n", (*it)->GetName());
            //fprintf(SettingFile, "Freq2: %ld\n", MulDiv((*it)->GetFrequency(),16,1000000));
            fprintf(SettingFile, "Freq: %ld\n", ((*it)->GetFrequency()/1000));
            fprintf(SettingFile, "Chan: %d\n", (*it)->GetChannelNumber());
            fprintf(SettingFile, "Active: %d\n", (*it)->IsActive());
            if((*it)->GetFormat() != -1)
            {
                fprintf(SettingFile, "Form: %d\n", (*it)->GetFormat());
            }
        }
        fclose(SettingFile);
        bSuccess = TRUE;
    }    
    return bSuccess;
}
