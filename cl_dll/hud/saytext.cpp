/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// saytext.cpp
//
// implementation of CHudSayText class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca

#include "vgui_TeamFortressViewport.h"

extern float *GetClientColor( int clientIndex );

extern cvar_t* hud_saytext_lines;

#define MAX_LINES	20
#define MAX_CHARS_PER_LINE	256  /* it can be less than this, depending on char size */

// allow 20 pixels on either side of the text
#define MAX_LINE_WIDTH  ( ScreenWidth - 40 )
#define LINE_START  10
static float SCROLL_SPEED = 5;

struct ChatMessage {
	char text[MAX_CHARS_PER_LINE];
	float chatTime; // when this message appeared
	float* nameColor;
	int nameLength;
};

static ChatMessage g_chatBuffer[ MAX_LINES + 1 ];

static int Y_START = 0;
static int line_height = 0;

DECLARE_MESSAGE( m_SayText, SayText );

int CHudSayText :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( SayText );

	InitHUDData();

	m_HUD_saytext =			gEngfuncs.pfnRegisterVariable( "hud_saytext", "1", 0 );
	m_HUD_saytext_time =	gEngfuncs.pfnRegisterVariable( "hud_saytext_time", "10", 0 );
	m_HUD_saytext_lines =	gEngfuncs.pfnRegisterVariable( "hud_saytext_lines", "10", 0 );

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission

	return 1;
}


void CHudSayText :: InitHUDData( void )
{
	memset(g_chatBuffer, 0, sizeof g_chatBuffer);
	m_lastChatIput = 0;
}


int ScrollTextUp( void )
{
	memmove(g_chatBuffer + 0, g_chatBuffer + 1, sizeof(g_chatBuffer) - sizeof(g_chatBuffer[0]) ); // overwrite the first line
	memset(g_chatBuffer + MAX_LINES, 0, sizeof(ChatMessage));

	return 1;
}

int CHudSayText :: Draw( float flTime )
{
	int y = Y_START + (MAX_LINES-1)*line_height;
	int wantLines = MaxLines();

	if ( ( gViewPort && gViewPort->AllowedToPrintText() == FALSE) || !m_HUD_saytext->value )
		return 1;

	float now = gEngfuncs.GetClientTime();
	bool isTyping = now - m_lastChatIput < 0.1f;

	for ( int i = MAX_LINES-1, drawnLines = 0; i >= 0 && drawnLines < wantLines; i-- )
	{
		ChatMessage& chat = g_chatBuffer[i];
		float age = now - chat.chatTime;
		bool shouldDisplay = isTyping || (age < m_HUD_saytext_time->value && age > 0);

		if ( *chat.text && shouldDisplay)
		{
			if (chat.nameColor )
			{
				// it's a saytext string
				char *buf = static_cast<char *>( _alloca( strlen(chat.text ) ) );
				if ( buf )
				{
					//char buf[MAX_PLAYER_NAME_LENGTH+32];

					// draw the first x characters in the player color
					strncpy( buf, chat.text, V_min(chat.nameLength, MAX_PLAYER_NAME_LENGTH+32) );
					buf[ V_min(chat.nameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;
					gEngfuncs.pfnDrawSetTextColor(chat.nameColor[0], chat.nameColor[1], chat.nameColor[2] );
					int x = DrawConsoleString( LINE_START, y, buf + 1 ); // don't draw the control code at the start
					strncpy( buf, chat.text + chat.nameLength, strlen(chat.text));
					buf[ strlen(chat.text + chat.nameLength) - 1 ] = '\0';
					// color is reset after each string draw
					DrawConsoleString( x, y, buf ); 
				}
				else
				{
					assert( "Not able to alloca chat buffer!\n");
				}
			}
			else
			{
				// normal draw
				DrawConsoleString( LINE_START, y, chat.text );
			}

			drawnLines++;
			y -= line_height;
		}		
	}

	return 1;
}

int CHudSayText :: MsgFunc_SayText( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int client_index = READ_BYTE();		// the client who spoke the message
	SayTextPrint( READ_STRING(), iSize - 1,  client_index );
	
	return 1;
}

int CHudSayText::MaxLines() {
	return V_max(1, V_min(m_HUD_saytext_lines->value, MAX_LINES));
}

int CHudSayText::ChatHeight(bool maxlines) {
	return line_height * ((maxlines ? MAX_LINES : MaxLines()) + 1);
}

void CHudSayText::SetChatInputPos(int* x, int* y) {
	UpdateChatPosition();
	*x = LINE_START;
	*y = Y_START + MAX_LINES * line_height;
	m_lastChatIput = gEngfuncs.GetClientTime();
}

void CHudSayText::UpdateChatPosition() {
	int bottom = (ScreenHeight * 8) / 10.0f; // ideal chat position
	int top = bottom - ChatHeight(false);

	int paddingTop = gHUD.m_iFontHeight * 2.0f;
	int paddingBot = gHUD.m_iFontHeight + line_height * 4; // make room for health and status text
	int maxTop = ScreenHeight / 2 + paddingTop * 2; // don't want chat covering the crosshair
	int maxBottom = ScreenHeight - paddingBot; // don't want it covering the health hud either

	const int SPECTATOR_PANEL_HEIGHT = YRES_HD(64);
	int maxBottomSpec = ScreenHeight - (SPECTATOR_PANEL_HEIGHT + 5);
	maxBottom = V_min(maxBottom, maxBottomSpec);

	if (top < maxTop)
		bottom += maxTop - top;
	if (bottom > maxBottom)
		bottom -= bottom - maxBottom;

	Y_START = bottom - ChatHeight(true);
}

void CHudSayText :: SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex )
{
	ConsolePrint(pszBuf);

	if ( gViewPort && gViewPort->AllowedToPrintText() == FALSE )
	{
		return; // only print to the console
	}

	int maxLines = MaxLines();
	int i;
	// find an empty string slot
	for ( i = 0; i < maxLines; i++ )
	{
		if ( ! *g_chatBuffer[i].text)
			break;
	}
	if ( i == maxLines)
	{
		// force scroll buffer up
		ScrollTextUp();
		i = maxLines - 1;
	}

	g_chatBuffer[i].nameLength = 0;
	g_chatBuffer[i].nameColor = NULL;

	// if it's a say message, search for the players name in the string
	if ( *pszBuf == 2 && clientIndex > 0 )
	{
		gEngfuncs.pfnGetPlayerInfo( clientIndex, &g_PlayerInfoList[clientIndex] );
		const char *pName = g_PlayerInfoList[clientIndex].name;

		if ( pName )
		{
			const char *nameInString = strstr( pszBuf, pName );

			if ( nameInString )
			{
				g_chatBuffer[i].nameLength = strlen( pName ) + (nameInString - pszBuf);
				g_chatBuffer[i].nameColor = GetClientColor( clientIndex );
			}
		}
	}

	strncpy(g_chatBuffer[i].text, pszBuf, V_max(iBufSize , MAX_CHARS_PER_LINE) );
	g_chatBuffer[i].chatTime = gEngfuncs.GetClientTime();

	// make sure the text fits in one line
	EnsureTextFitsInOneLineAndWrapIfHaveTo( i );

	m_iFlags |= HUD_ACTIVE;
	PlaySound(RemapFile("misc/talk.wav"), 1 );

	UpdateChatPosition();
}

void CHudSayText :: EnsureTextFitsInOneLineAndWrapIfHaveTo( int line )
{
	int line_width = 0;
	GetConsoleStringSize(g_chatBuffer[line].text, &line_width, &line_height );

	if ( (line_width + LINE_START) > MAX_LINE_WIDTH )
	{ // string is too long to fit on line
		// scan the string until we find what word is too long,  and wrap the end of the sentence after the word
		int length = LINE_START;
		int tmp_len = 0;
		char *last_break = NULL;
		for ( char *x = g_chatBuffer[line].text; *x != 0; x++ )
		{
			// check for a color change, if so skip past it
			if ( x[0] == '/' && x[1] == '(' )
			{
				x += 2;
				// skip forward until past mode specifier
				while ( *x != 0 && *x != ')' )
					x++;

				if ( *x != 0 )
					x++;

				if ( *x == 0 )
					break;
			}

			char buf[2];
			buf[1] = 0;

			if ( *x == ' ' && x != g_chatBuffer[line].text )  // store each line break,  except for the very first character
				last_break = x;

			buf[0] = *x;  // get the length of the current character
			GetConsoleStringSize( buf, &tmp_len, &line_height );
			length += tmp_len;

			if ( length > MAX_LINE_WIDTH )
			{  // needs to be broken up
				if ( !last_break )
					last_break = x-1;

				x = last_break;

				// find an empty string slot
				int j;
				do 
				{
					for ( j = 0; j < MAX_LINES; j++ )
					{
						if ( ! *g_chatBuffer[j].text )
							break;
					}
					if ( j == MAX_LINES )
					{
						// need to make more room to display text, scroll stuff up then fix the pointers
						int linesmoved = ScrollTextUp();
						line -= linesmoved;
						last_break = last_break - (sizeof(g_chatBuffer[0].text) * linesmoved);
					}
				}
				while ( j == MAX_LINES );

				// copy remaining string into next buffer,  making sure it starts with a space character
				if ( (char)*last_break == (char)' ' )
				{
					int linelen = strlen(g_chatBuffer[j].text);
					int remaininglen = strlen(last_break);

					if ( (linelen - remaininglen) <= MAX_CHARS_PER_LINE )
						strcat(g_chatBuffer[j].text, last_break );
				}
				else
				{
					if ( (strlen(g_chatBuffer[j].text) - strlen(last_break) - 2) < MAX_CHARS_PER_LINE )
					{
						strcat(g_chatBuffer[j].text, " " );
						strcat(g_chatBuffer[j].text, last_break );
					}
				}

				*last_break = 0; // cut off the last string

				EnsureTextFitsInOneLineAndWrapIfHaveTo( j );
				break;
			}
		}
	}
}
