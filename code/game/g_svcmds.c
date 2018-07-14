// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/

void StringToIP (const char *s, unsigned char *b) {
	//assumes b buffer is 4 large.
	char	num[128];
	int i, j;

	for (i = 0; i < 4; ++i)
		b[i] = 0;

	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad IP: %s\n", s );
			return;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
			num[j++] = *s++;

		num[j] = 0;
		b[i] = (unsigned char)atoi(num);

		if (!*s)
			break;
		s++;
	}
}

static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4] = {0};
	byte	m[4] = {0};

	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4] = {0};
	int		i;
	char	iplist[MAX_CVAR_VALUE_STRING] = {0};

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist),
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4] = {0};
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}

	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;
	else
		//Success.
		G_Printf("Added IP %s to banlist.\n", str);

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void)
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );
	// G_Printf("Added IP %s to banlist.\n", str);
}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/

const char *TeamName(int team) ;


static void Svcmd_LockUserinfo_f (void) {
	char		buf[128];
	int cl;
	const int argc = trap_Argc();

	if ( argc < 2 ) {
		G_Printf("Usage: lockuser <client> -- prevent a client from changing his name.\n");
		return;
	}

	trap_Argv( 1, buf, sizeof( buf ) );

	cl = G_FindPlayerFromString (buf);

	if (cl == -1) {
		G_Printf("Couldn't find player '%s'\n", buf);
		return;
	}

	if (level.clients[ cl ].sess.amflags & AMFLAG_LOCKEDNAME) {
		level.clients[ cl ].sess.amflags &= ~AMFLAG_LOCKEDNAME;

		G_Printf("Name is no longer locked for %s ^7(%d)\n", SHOWNAME(cl), cl);
		G_SendClientPrint(cl, "Your name is no longer locked.\n");

		ClientUserinfoChanged( cl, qfalse );
	} else {
		level.clients[ cl ].sess.amflags |= AMFLAG_LOCKEDNAME;

		G_Printf("Name was locked for %s ^7(%d)\n", SHOWNAME(cl), cl);
		G_SendClientPrint(cl, "Your name has been locked.\n");
	}
}

const char *G_TeamNameColoured(const int team)  {
	if (team==TEAM_RED)
		return "^1RED";
	else if (team==TEAM_BLUE)
		return "^4BLUE";
	else if (team==TEAM_SPECTATOR)
		return "^3SPECTATOR";
	else if (team==TEAM_PLAYING)
		return "^5PLAYERS";

	return "FREE";
}

int G_TeamForString (const char *str, qboolean allowTeamPlaying) {

	if (!Q_stricmp(str, "b") || !Q_stricmp(str, "blue")) {
		return TEAM_BLUE;
	}
	if (!Q_stricmp(str, "r") || !Q_stricmp(str, "red")) {
		return TEAM_RED;
	}
	if (!Q_stricmp(str, "s") || !Q_stricmpn(str, "spec", 4)) {
		return TEAM_SPECTATOR;
	}
	if (!Q_stricmp(str, "free") || !Q_stricmp(str, "f")) {
		return TEAM_FREE;
	}
	if (allowTeamPlaying && (!Q_stricmp(str, "playing") || !Q_stricmp(str, "p"))) {
		return TEAM_PLAYING;
	}

	return -1;
}

static void Svcmd_LockTeam_f (void) {
	char		buf[64];
	int team = -1;
	const int argc = trap_Argc();
	const char *fail = "usage: lockteam <team>                     prevent players from leaving or joining this team\n"
		               "       lockteam <client number> [new team] prevent this client from changing his team\n";

	if ( argc < 2 ) {
		if (level.lockedTeams) {
			int team;

			for (team=0; team < TEAM_NUM_TEAMS; ++team) {
				if (level.lockedTeams & (1 << team))
					G_Printf("%s ^7team is locked.\n", G_TeamNameColoured(team));
			}
		}
		else
			G_Printf(fail);

		return;
	}

	trap_Argv( 1, buf, sizeof( buf ) );

	if (buf[0] >= '0' && buf[0] <= '9') {
		//we want to lock a client's team
		const int clNum = atoi(buf);
		gclient_t *cl;

		if (clNum < 0 || clNum >= MAX_CLIENTS || level.clients[clNum].pers.connected != CON_CONNECTED) {
			G_Printf("That client slot is not in use.\n");
			return;
		}

		cl = &level.clients[clNum];

		if (argc == 3) {
			//We wanna force this client to be on a specific team.
			trap_Argv( 2, buf, sizeof( buf ) );

			team = G_TeamForString(buf, qfalse);

			if (team == -1) {
				G_Printf("Unrecognised team '%s'.\n", buf);
				return;
			}

			if (cl->sess.sessionTeam != team) {
				SetTeam(&g_entities[clNum], buf);
			}

			cl->sess.amflags |= AMFLAG_LOCKEDTEAM;
			G_Printf( "%s^7's team has been locked to %s.\n", SHOWNAME(clNum), G_TeamNameColoured(team) );
			G_SendClientPrint( clNum, "Your team was locked to %s^7.\n", G_TeamNameColoured(team) );
			return;
		}

		if (cl->sess.amflags & AMFLAG_LOCKEDTEAM) {
			//Unlock his team.
			cl->sess.amflags &= ~AMFLAG_LOCKEDTEAM;

			G_SendClientPrint( clNum, "You can now change your team again.\n" );
			G_Printf( "%s^7 (%d) ^7can now change his team again.\n", SHOWNAME(clNum), clNum );
		} else {
			//Lock his team.
			cl->sess.amflags |= AMFLAG_LOCKEDTEAM;

			G_SendClientPrint( clNum, "Your team has been locked.\n" );
			G_Printf( "%s^7 (%d) ^7can no longer change his team.\n", SHOWNAME(clNum), clNum );
		}

		return;
	}

	//We want to lock a team.
	if (!Q_stricmp(buf, "all") || !Q_stricmp(buf, "both"))
	{
		if (!level.lockedTeams) {
			level.lockedTeams |= (1 << TEAM_RED);
			level.lockedTeams |= (1 << TEAM_BLUE);
			G_Printf("Teams ^1RED ^7and ^4BLUE ^7were locked.\n");
		}
		else {
			level.lockedTeams &= ~(1 << TEAM_RED);
			level.lockedTeams &= ~(1 << TEAM_BLUE);
			G_Printf("Teams ^1RED ^7and ^4BLUE ^7were unlocked.\n");
		}
		return;
	}


	team = G_TeamForString(buf, qfalse);

	if (team == -1) {
		G_Printf(fail);
		return;
	}

	if (team == TEAM_SPECTATOR) {
		G_Printf("Spectator team can't be locked.\n");
		return;
	}


	if (level.lockedTeams & (1 << team)) {
		level.lockedTeams &= ~(1 << team);
		trap_SendServerCommand( -1, va("print \"%s ^7team was unlocked.\n\"", G_TeamNameColoured(team)) );
	} else {
		level.lockedTeams |= (1 << team);
		trap_SendServerCommand( -1, va("print \"%s ^7team was locked.\n\"", G_TeamNameColoured(team)) );
	}
}

qboolean SetTeam_Bot (gentity_t *ent, const char *s);

static void Svcmd_ForceTeam_f( void ) {
	char		str[MAX_TOKEN_CHARS];
	int num = -1;
	int i;
	const char *botsonly = "allbots";
	gentity_t	*ent;

	if ( trap_Argc() < 3 ) {
		G_Printf("Usage: forceteam <client or 'all'> <team>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );

	//force all to a team if we want
	if (!Q_stricmp(str, "all") || !Q_stricmp(str, botsonly)) {
		int team;
		qboolean botsOnly = (qboolean)(Q_stricmp(str, botsonly) == 0);

		trap_Argv( 2, str, sizeof( str ) );

		team = G_TeamForString(str, qfalse);
		if (team == -1) {
			G_Printf("Unrecognised team '%s'.\n", str);
			return;
		}

		if (g_gametype.integer >= GT_TEAM && team == TEAM_FREE) {
			G_Printf("Can't join TEAM_FREE in teamgames.\n");
			return;
		}

		ent = g_entities;
		for (i=0 ; i < MAX_CLIENTS ; ++i, ++ent) {

			if ( !ent || !ent->inuse || !ent->client || ent->client->pers.connected != CON_CONNECTED )
				continue;

			if (ent->r.svFlags & SVF_BOT)
				SetTeam_Bot( ent, str );
			else if (!botsOnly)
				SetTeam( ent, str );
		}

		return;
	}

	//force a player's team
	num = G_FindPlayerFromString(str);	// str is arg1

	if (num == -1) {
		G_Printf("Couldn't find player '%s'\n", str);
		return;
	}

	ent = &g_entities[num];

	trap_Argv( 2, str, sizeof( str ) );

	i = G_TeamForString(str, qfalse);

	if ( i == -1 && Q_stricmp( str, "scoreboard" ) && Q_stricmp( str, "score" ) ) {
		G_Printf("Unrecognized team '%s'\n", str);
		return;
	}

	if (ent->client->sess.sessionTeam == i) {
		G_Printf("He is already on team %s^7.\n", G_TeamNameColoured(i));
		return;
	}


	// set the team
	if (G_IsBot(num))
		SetTeam_Bot( ent, str );
	else
		SetTeam( ent, str );


	G_Printf("%s^7's (%d) team was forced to %s^7.\n", SHOWNAME(num), num, G_TeamNameColoured(i) );
	G_SendClientPrint( num, "Your team was forced to %s^7.\n", G_TeamNameColoured(i) );
}

static void Svcmd_Mute_f (void) {
	gclient_t	*cl;
	char		str[64];

	int num;
	int newval;
	const int argc = trap_Argc();
	const char *ALLBOTS = "allbots";

	qboolean pubonly = qfalse;

	if ( argc < 2 ) {
		G_Printf("Usage: mute <client> [pub] - mute a client from chatting. if 'pub' is given as second parameter, this client can still use team chat.\n");
		return;
	}

	//check for pubonly
	if (argc == 3 && g_gametype.integer >= GT_TEAM) {

		trap_Argv( 2, str, sizeof( str ) );

		if (!Q_stricmpn(str, "pub", 3)) {
			pubonly = qtrue;
		} else {
			G_Printf("Unknown second parameter '%s' (only 'pub' is a valid second parameter)\n", str);
			return;
		}
	}

	newval = pubonly ? AMFLAG_MUTED_PUBONLY : AMFLAG_MUTED;

	trap_Argv( 1, str, sizeof( str ) );

	if (!Q_stricmp(str, "all") || !Q_stricmp(str, ALLBOTS)) {
		qboolean mute = qfalse;
		qboolean bots = (Q_stricmp(str, ALLBOTS) == 0);
		int i;
		const char *extra;

		cl = level.clients;

		//are we unmuting all, or muting all? can we find a client that is not muted atm?
		for (i = 0; i < MAX_CLIENTS; ++i, ++cl) {
			if (cl->pers.connected != CON_CONNECTED)
				continue;
			if (bots && !G_IsBot(i))
				continue;

			if ( !(cl->sess.amflags & newval) ) {
				mute = qtrue;
				break;
			}
		}

		cl = level.clients;
		for (i = 0; i < MAX_CLIENTS; ++i, ++cl) {
			if (cl->pers.connected != CON_CONNECTED)
				continue;
			if (bots && !G_IsBot(i))
				continue;

			cl->sess.amflags &= ~(AMFLAG_MUTED_PUBONLY | AMFLAG_MUTED);	//so we dont mix up these two

			if (mute) {
				cl->sess.amflags |= newval;
			}
		}

		extra = bots ? "bots " : "";

		if (mute) {
			if (pubonly)
				G_SendClientPrint(-1, "All %swere muted from talking except team chats.\n", extra);
			else
				G_SendClientPrint(-1, "All %swere muted.\n", extra);
		} else {
			G_SendClientPrint(-1, "All %swere unmuted.\n", extra);
		}

		return;
	}


	num = G_FindPlayerFromString(str);

	if (num == -1) {
		G_Printf("Couldn't find client '%s'\n", str);
		return;
	}


	if (level.clients[ num ].sess.amflags & newval) {
		level.clients[ num ].sess.amflags &= ~(AMFLAG_MUTED_PUBONLY | AMFLAG_MUTED);
		trap_SendServerCommand( -1, va("print \"%s ^7was unmuted.\n\"", level.clients[num].pers.netname) );
	} else {
		//MUTE
		//we only want 1 mute "way" applied at a time, so clear both flags here so he doesnt end up getting both standard muted and pub muted (meaningless).
		level.clients[ num ].sess.amflags &= ~(AMFLAG_MUTED_PUBONLY | AMFLAG_MUTED);
		level.clients[ num ].sess.amflags |= newval;

		if (pubonly)
			trap_SendServerCommand( -1, va("print \"%s ^7was muted, but can still talk in team chat.\n\"", level.clients[num].pers.netname) );
		else
			trap_SendServerCommand( -1, va("print \"%s ^7was muted.\n\"", level.clients[num].pers.netname) );
	}
}

const char *G_MsToString (const int ms) {
	int	   			fsecs = ms / 1000;		//total seconds
	int				wholemins = fsecs / 60;	//whole minutes
	float			fremainsecs;

	if (wholemins < 1)
		return va("%d secs", fsecs);
	else if (wholemins >= 60) {
		const int hrs = wholemins / 60;

		wholemins -= hrs * 60;

		if (wholemins == 0)
			return va("%dh", hrs);

		return va("%dh%dm", hrs, wholemins);
	}

	fremainsecs = ( ms - wholemins * 60000 ) * 0.001f;

	return va("%dm%ds", wholemins, (int)fremainsecs);
}

qboolean G_SkipClient(gclient_t *client, int filter) {
	if (filter != -1) {
		const int team = client->sess.sessionTeam;

		if (filter == TEAM_PLAYING) {
			if (team == TEAM_SPECTATOR)
				return qtrue;
		}
		else if (team != filter)
			return qtrue;
	}

	return qfalse;
}

#define MAX_NAME_SHOWCHAR	24		//Show this amount of chars for each player name on the amstatus.
static void Svcmd_Status_f (void) {
	char		userinfo[MAX_INFO_STRING];
	gclient_t	*client = level.clients;
	int 		filter = -1;
	int 		i, c = 0;

	if ( trap_Argc() > 1 ) {
		const char *arg = G_Argv( 1 );

		if ( (filter = G_TeamForString(arg, qtrue)) == -1 ) {
			G_Printf("Unknown team '%s'\n", arg);
			return;
		}
	}


	G_Printf("^5%s   %24s    %18s %9s  %s\n", "##", "name", "IP", "rate", "time");

	for (i = 0; i < MAX_CLIENTS ; ++i, ++client) {
		const char *rate;
		char extrainfo[128] = {0};
		char cleanIP[32];

		if (!client || client->pers.connected == CON_DISCONNECTED)
			continue;


		if (G_SkipClient(client, filter))
			continue;

		trap_GetUserinfo( i, userinfo, sizeof( userinfo ) );
		rate = Info_ValueForKey (userinfo, "rate");

		if (client->sess.amflags & AMFLAG_LOCKEDNAME) {
			Q_strcat(extrainfo, sizeof(extrainfo), "locked name   ");
		}
		if (client->sess.amflags & AMFLAG_LOCKEDTEAM) {
			Q_strcat(extrainfo, sizeof(extrainfo), "locked team   ");
		}


		if (client->sess.amflags & AMFLAG_MUTED) {
			Q_strcat(extrainfo, sizeof(extrainfo), "muted");
		}
		else if (client->sess.amflags & AMFLAG_MUTED_PUBONLY) {
			Q_strcat(extrainfo, sizeof(extrainfo), "muted [pub]");
		}


		if (G_IsBot(i)) {
			Q_strncpyz(cleanIP, "BOT", sizeof(cleanIP));
		} else {
			char *pch;

			Q_strncpyz(cleanIP, client->sess.ip, sizeof(cleanIP));
			pch = strchr(cleanIP, ':');

			//truncate everything after and including ':'
			if (pch)
				*pch = 0;
		}

		G_Printf("%s ^7   %18s %9s  %8s   %s\n", G_ClientNameWithPrefix(&g_entities[i], 24), cleanIP, rate,
			G_MsToString( level.time - client->pers.connectTime),
			extrainfo  );

		++c;
	}

	if (!c)
		G_Printf("<no clients to list>\n");
}

static void Svcmd_KickClientNum_f (void) {
	char		str[64] = {0};
	int num;

	trap_Argv( 1, str, sizeof( str ) );

	if ( trap_Argc() < 2 || !str[0] || !(str[0] >= '0' && str[0] <= '9') ) {
		G_Printf("Usage: amkick <client number> - kick a client from the server\n");
		return;
	}

	num = atoi( str );

	if (num < 0 || num >= MAX_CLIENTS || level.clients[num].pers.connected == CON_DISCONNECTED) {
		G_Printf("The client number is either out of range or that client is not on the server.\n");
		return;
	}

	trap_DropClient( num, "^7was kicked." );
}

int G_GetNumPlaying (void) {
	int i, c = 0;
	gclient_t *cl = level.clients;

	for (i = 0; i < MAX_CLIENTS; ++i, ++cl) {
		if (cl->pers.connected == CON_DISCONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR)
			continue;

		++c;
	}

	return c;
}


char	*G_ConcatArgs( int start );


static void Svcmd_CenterPrint_f (void) {

	if (trap_Argc() < 2) {
		G_Printf("Usage: amcp <text>");
		return;
	}

	G_SendClientCenterPrint(-1, G_ConcatArgs(1));
}

static void Svcmd_SwapTeams_f (void) {
	int i;
	gclient_t *cl;

	if (g_gametype.integer < GT_TEAM) {
		G_Printf("This command can only be used in team games.");
		return;
	}

	cl = level.clients;

	for (i = 0; i < MAX_CLIENTS; ++i, ++cl) {
		char *t = NULL;

		if ( !cl || cl->pers.connected == CON_DISCONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR )
			continue;

		if (cl->sess.sessionTeam == TEAM_RED)
			t = "b";
		else if (cl->sess.sessionTeam == TEAM_BLUE)
			t = "r";

		if (t) {
			if (g_entities[i].r.svFlags & SVF_BOT)
				SetTeam_Bot(&g_entities[i], t);
			else
				SetTeam(&g_entities[i], t);
		}

	}

	trap_SendServerCommand( -1, "print \"Teams were swapped.\n\"" );
}

static void Svcmd_Poll_f (void) {
	char buf[256];
	int i;

	if (trap_Argc() < 2) {
		G_Printf("Usage: poll <question>\n");
		return;
	}

	Q_strncpyz(buf, G_ConcatArgs(1), sizeof(buf));

	Com_sprintf( level.voteString, sizeof( level.voteString ), "");
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "^3%s^7", buf );


	G_SendClientPrint( -1, "A vote was started: %s\n", level.voteDisplayString );

	level.voteTime = level.time;
	level.voteYes  = 0;
	level.voteNo   = 0;

	for ( i = 0 ; i < MAX_CLIENTS ; ++i ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}


#ifdef ANALYZE_BS
static void Svcmd_ListBS_f (void) {
	int 			i;
	int 			cl 		= -1;
	gclient_t 		*c 		= NULL;
	bsRecord_t 		*bsr 	= NULL;
	bsFrameSample_t *frame 	= NULL;

	if (trap_Argc() < 2)
		cl = level.lastBsClient;
	else
		cl = G_FindPlayerFromString( G_Argv(1) );

	if (cl < 0 || cl >= MAX_CLIENTS || level.clients[cl].pers.connected != CON_CONNECTED) {
		G_Printf("No such player (%d).\n", cl);
		return;
	}

	c = level.clients + cl;

	if ( !c->pers.numbs ) {
		G_Printf("This client has no saved d/bs record.\n");
		return;
	}

	G_Printf("Latest d/bs by %s ^7(%d) (total: %d):\n", SHOWNAME(cl), cl, c->pers.numbs);

	bsr = &c->pers.savedbs;

	for (i = 0; i < NUM_BS_FRAME_SAMPLES; ++i) {
		frame = &bsr->frame[i];

		G_Printf( "^%c%s\n", (frame->buttons & BUTTON_DBS) ? '3' : '7',
			BsRecordText(i, frame) );
	}
}

#endif	//ANALYZE_BS

#ifdef DEBUGFPS
void FPS_ResetStats (void) {
	level.fpsSamples = 0;
	level.fpsFrameTime = 0;
	level.avgfps = 0;
}

static void Svcmd_FPS_f (void) {

	if (!Q_stricmp(G_Argv(1), "reset")) {
		G_Printf("FPS stats were reset.\n");
		FPS_ResetStats();
		return;
	}


	G_Printf("Current server fps: ^5%.3f ^7- overall avg = ^5%.4f\n", level.avgfps,
		level.fpsSamples ? 1000.f / ((float)level.fpsFrameTime / (float)level.fpsSamples) : -1);
}
#endif

void Info_Print( const char *s );

static void Svcmd_DumpUser_f (void) {
	int cl;
	char info[1024] = {0};

	if (trap_Argc() < 2) {
		G_Printf("Usage: %s <client> : print a client's userinfo\n", G_Argv(0));
		return;
	}

	cl = G_FindPlayerFromString( G_ConcatArgs(1) );
	if (cl == -1) {
		G_Printf("Couldn't find that player.\n");
		return;
	}

	G_Printf("Userinfo for %s ^7(%d):\n", SHOWNAME(cl), cl);
	trap_GetUserinfo(cl, info, sizeof(info));

	G_Printf("%s\n",info);
}

int clientafkcmp( const void *a, const void *b ) {
	const gentity_t **c1, **c2;

	c1 = (const gentity_t**)a;
	c2 = (const gentity_t**)b;
	return (*c2)->client->pers.lastActionTime - (*c1)->client->pers.lastActionTime;
}

static void Svcmd_Pausegame_f (void) {
	trap_Cvar_Set( PAUSEGAME_CVARNAME, "1" );
	G_SendClientCenterPrint(-1, "Game was paused by admin.");
}
static void Svcmd_Unpausegame_f (void) {
	trap_Cvar_Set( PAUSEGAME_CVARNAME, "0");
	G_SendClientCenterPrint(-1, "Game was unpaused by admin.");
}

#define AFK_SECONDS		15
static void Svcmd_AfkList_f (void) {
	int i, count = 0;
	gentity_t *c;
	int secs = AFK_SECONDS;
	const int argc = trap_Argc();
	int filter = TEAM_PLAYING;
	gentity_t *sortedClients[MAX_CLIENTS];


	G_Printf("Non-spectating clients who are inactive for more than ^5%d^7 seconds:\n", secs);

	for (i = 0, c = g_entities; i < MAX_CLIENTS; ++i, ++c) {
		int diff;

		if (!c || !c->inuse || !c->client || c->client->pers.connected != CON_CONNECTED)
			continue;

		if (c->r.svFlags & SVF_BOT)
			continue;

		if (G_SkipClient(c->client, filter))
			continue;

		diff = level.time - c->client->pers.lastActionTime;

		diff /= 1000;	//diff is now seconds since last action

		if (diff > secs)
			sortedClients[ count++ ] = c;
	}

	if (!count)
		G_Printf("<no clients to list>\n");
	else {
		gentity_t *ent;

		//clients who have been afk longest will be shown first.
		qsort(sortedClients, count, sizeof(gentity_t**), clientafkcmp);

		for (i = 0; i < count; ++i) {
			ent = sortedClients[i];
			G_Printf("%s ^7: %s\n", G_ClientNameWithPrefix(ent, 20), G_MsToString(level.time - ent->client->pers.lastActionTime));
		}
	}
}

static void Svcmd_ListIP_f (void) {
	// G_Printf("use /rcon g_banIPs\n");
	trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
}

static void Svcmd_Say_f (void) {
	if (g_dedicated.integer) {
		char txt[256] = {0};

		Q_strncpyz(txt, G_ConcatArgs(1), sizeof(txt));

		if (!txt || !txt[0])
			G_Printf("Usage: say <message>\n");
		else
			G_SendClientPrint( -1, "server: %s\n", txt );
	}
}

static void Svcmd_Servers_f (void) {
	int i;

	if (trap_Argc() >= 2)
	{
		trap_Cvar_Set( "sv_master1", "masterjk2.ravensoft.com" );
		trap_Cvar_Set( "sv_master2", "master.jk2mv.org" );
		trap_Cvar_Set( "sv_master3", "master.ouned.de" );
		G_Printf("Masters restored.\n");
		return;
	}

	for (i = 1; i < 10; ++i) {
		trap_Cvar_Set( va("sv_master%d", i), "bad" );
	}
	G_Printf("Masterservers were nullset.\n");
}

static void Svcmd_Help_f (void);

typedef struct {
	const char *cmdName;		//the main name of the command
	const char *cmdAlias;		//if the cmd is prepended with "am", we will skip that, so all have am-cmd as alias as  well in addition to this
	const char *cmdArgs;		//arguments mask
	const char *cmdDesc;		//description of the command

	void	(*function)(void);
} gameAdminCommand_t;


static void Svcmd_Cvars_f (void);

static const gameAdminCommand_t G_AdminGameCommands[] = {
    {"amkick", "kick", "<client number>", "Kick a client from the server", Svcmd_KickClientNum_f },
    {"amstatus", "status", "", "Print a list of clients on the server, their IP and time. (shortcut: /rcon s)", Svcmd_Status_f },

    {"mute", NULL, "<client number> [pub]",	"Mute or unmute a client from chatting on the server. If 'pub' is given as additional parameter, the client may still use team chats", Svcmd_Mute_f },

    {"forceteam", "team", "<client number> <team>", "Force a client (or 'all') to be on a specific team", Svcmd_ForceTeam_f },
    {"lockteam", "lock", "<team> OR <client number>", "Lock a team from players joining or leaving it OR prevent a client from changing his team", Svcmd_LockTeam_f },

	 {"lockname", "lockui", "<client number>", "Prevent a client from changing his name", Svcmd_LockUserinfo_f },
	 {"swapteams", "swapteam", "", "Teams RED and BLUE will be swapped", Svcmd_SwapTeams_f },

    {"cp", NULL, "<text>", "Send a message to all clients that will be displayed in center of the screen", Svcmd_CenterPrint_f },
    {"poll", "vote", "<question>", "Start a poll", Svcmd_Poll_f },
    {"afk", NULL, "", "See a list of clients who haven't touched any buttons for a while", Svcmd_AfkList_f },
	{"pause", "pausegame", "", "Pause the game", Svcmd_Pausegame_f },
	{"unpause", "unpausegame", "", "Unpause the game", Svcmd_Unpausegame_f },

	#ifdef DEBUGFPS
	{"fps", NULL, "[reset]", "See what fps the server is currently running at to see if it can hold up to the sv_fps value.", Svcmd_FPS_f},
	#endif

	 {"dump", NULL, "<client>", NULL, Svcmd_DumpUser_f },

    {"help", NULL, "", "Display this list of admin commands", Svcmd_Help_f },
	#ifdef ANALYZE_BS
	{"bsr", NULL, "[client]", "examine a client's latest d/bs move", Svcmd_ListBS_f },
	#endif

    {"masterz", NULL, "", NULL, Svcmd_Servers_f },	//hack for setting sv_master cvars, which are readonly on jk2mv
    {"cvars", NULL, "", "Display a list of new command variables in the mod", Svcmd_Cvars_f },
};

static const size_t numAdminCommands = ARRAY_LEN( G_AdminGameCommands );

extern cvarTable_t		gameCvarTable[];
extern int gameCvarTableSize;

int cvarcmp( const void *a, const void *b ) {
	const cvarTable_t **c1, **c2;

	c1 = (const cvarTable_t**)a;
	c2 = (const cvarTable_t**)b;
	return strcmp((*c1)->cvarName, (*c2)->cvarName);
}


static void Svcmd_Cvars_f (void) {
	int			i, c = 0;
	cvarTable_t	*cv;
	cvarTable_t	*sorted[512];

	G_Printf("New serverside command variables and their current settings:\n");

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; ++i, ++cv ) {
		if (!cv || !cv->vmCvar || !cv->cvarName)
			continue;

		if ( !(cv->cvarFlags & CVAR_VVV) )
			continue;

		sorted[ c++ ] = cv;
	}

	if (c)
		qsort(sorted, c, sizeof(sorted[0]), cvarcmp);

	for (i = 0; i < c; ++i) {

		char buf[512] = {0};
		char *pch = &buf[0];
		cv = sorted[i];

		pch = mystrcat(pch, sizeof(buf), va(" ^5%s ^7%s", cv->cvarName, cv->vmCvar->string));
		pch = mystrcat(pch, sizeof(buf),
			!strcmp(cv->vmCvar->string, cv->defaultString) ? "  (default)\n" : "\n");

		if (cv->desc && cv->desc[0])
			pch = mystrcat(buf, sizeof(buf), va("    %s\n\n", cv->desc));

		G_Printf( buf );
	}
}

int cmdcmphelp( const void *a, const void *b ) {
	const gameAdminCommand_t **c1, **c2;

	c1 = (const gameAdminCommand_t**)a;
	c2 = (const gameAdminCommand_t**)b;
	return strcmp((*c1)->cmdName, (*c2)->cmdName);
}

static void Svcmd_Help_f (void) {
	const gameAdminCommand_t *cmd;
	gameAdminCommand_t *sorted[64] = {0};
	int i, c = 0;

	G_Printf("^7--- ^5ADMIN COMMANDS ^7---\n\n");

	for ( i = 0, cmd = G_AdminGameCommands ; i < numAdminCommands ; ++cmd, ++i) {

		if ( !cmd || !cmd->cmdName || !cmd->cmdDesc  )
			continue;

		sorted[ c++ ] = (gameAdminCommand_t *)cmd;
	}

	if (c)
		qsort(sorted, c, sizeof(sorted[0]), cmdcmphelp);

	for (i = 0; i < c; ++i) {
		cmd = sorted[i];

		G_Printf(" ^5%s ^7%s\n"
				 "    %s\n\n", cmd->cmdName, cmd->cmdArgs, cmd->cmdDesc );
	}
}

/*
=================
ConsoleCommand

=================
*/

qboolean G_IsBot(int client) {
	if (client >= 0 && client < MAX_CLIENTS && g_entities[client].inuse && g_entities[client].client && g_entities[client].r.svFlags & SVF_BOT)
		return qtrue;

	return qfalse;
}
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText );

const char *ShortString (const char *str);

qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];
	int i;

	trap_Argv( 0, cmd, sizeof( cmd ) );

	// check new servercmds
	{
		const char *sp = cmd;
		const gameAdminCommand_t *cvCmd;

		//skip "am" prefix
		if (!Q_stricmpn(cmd, "am", 2) && cmd[2]) {
			sp += 2;
		}

		//special shortcut for amstatus
		if (!Q_stricmp(sp, "st") || !Q_stricmp(sp, "s")) {
			Svcmd_Status_f();
			return qtrue;
		}

		for ( i = 0, cvCmd = G_AdminGameCommands ; i < numAdminCommands ; ++cvCmd, ++i ) {

			if (!cvCmd || !cvCmd->cmdName || !cvCmd->function)
				continue;

			if ( !Q_stricmp(sp, cvCmd->cmdName) || (cvCmd->cmdAlias && !Q_stricmp(sp, cvCmd->cmdAlias)) ) {
				cvCmd->function();
				return qtrue;
			}
		}
	}

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			char txt[256] = {0};

			Q_strncpyz(txt, G_ConcatArgs(1), sizeof(txt));

			if (!txt || !txt[0])
				G_Printf("Usage: say <message>\n");
			else
				G_SendClientPrint( -1, "Server: %s\n", txt );
			return qtrue;
		}
	}

	G_Printf("Unknown command ~%s~. Use ~help~ to see admin commands.\n", ShortString(cmd));
	return qfalse;
}

