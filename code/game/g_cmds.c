// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

void BG_CycleInven(playerState_t *ps, int direction);
void BG_CycleForce(playerState_t *ps, int direction);


#define TEAMSTATS_CMD		"tstats"
static void G_SendTeamStats( gentity_t *ent ) {
	/*
		format for 1 team: (6 stat fields)
		%d %d %d %d %d %d
		rets, defense, assist, flaghold, flaggrabs, frags, score
	*/
	char msg[1024] = {0};
	int i;
	char *pch = &msg[0];


	for (i = 0; i < 2; ++i) {
		teamStats_t *ts = &level.teamstats[ i ];
		const char *s;

		s = va("%d %d %d %d %d %d %d ", ts->rets, ts->defense, ts->assist, ts->flaghold, ts->flaggrabs, ts->frags, ts->score);
		pch = mystrcat(pch, sizeof(msg), s);
	}

	pch = va(TEAMSTATS_CMD" %s", msg);

	trap_SendServerCommand( ent-g_entities, pch );
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024]  = {0};
	char		string[1400] = {0};
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;
	int			endTime;	//dont allow time to pass during intermission.

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	if (numSorted > MAX_CLIENT_SCORE_SEND)
		numSorted = MAX_CLIENT_SCORE_SEND;

	if (level.intermissiontime)
		endTime = level.intermissiontime;
	else
		endTime = level.time;

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING )
			ping = -1;
		else
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;


		if( cl->accuracy_shots )
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		else
			accuracy = 0;

		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE],
			ping,
			(endTime - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );

	if (g_gametype.integer == GT_CTF && ent->client->pers.teamInfo == 2)
		G_SendTeamStats( ent );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOCHEATS")));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MUSTBEALIVE")));
		return qfalse;
	}
	return qtrue;
}


/*
==================
G_ConcatArgs
==================
*/
char	*G_ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		name[MAX_TOKEN_CHARS];
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( ent ) ) {
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (WP_DET_PACK+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap_Argv( 2, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->client->ps.ammo[i] = num;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (trap_Argc() == 3) {
			trap_Argv( 2, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		return;
	}

	if (g_pauseGame.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"Can't kill yourself while game is paused :----D\n\"") );
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT && level.numPlayingClients > 1 && !level.warmupTime)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "ATTEMPTDUELKILL")) );
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];

		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEREDTEAM")) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBLUETEAM")));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHESPECTATORS")));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		if (g_gametype.integer == GT_TOURNAMENT)
		{

		}
		else
		{
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStripEdString("SVINGAME", "JOINEDTHEBATTLE")));
		}
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	if (!ent || !ent->inuse || !ent->client) {
		G_SecurityLogPrint( va("SetTeam: invalid entity, team: '%s'", s), ent);
		return;
	}


	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if (!Q_stricmp(s, "s") || !Q_stricmpn(s, "spec", 4)) {	//edited to match behavior in G_TeamForString
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			team = PickTeam( clientNum );
		}

		if ( g_teamForceBalance.integer  ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				trap_SendServerCommand( ent->client->ps.clientNum,
					va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYRED")) );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				trap_SendServerCommand( ent->client->ps.clientNum,
					va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "TOOMANYBLUE")) );
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}
	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( g_gametype.integer == GT_TOURNAMENT && level.numNonSpectatorClients >= 2 ) {
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}


	// decide if we will allow the change
	oldTeam = client->sess.sessionTeam;

	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	// execute the team change

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
	}

	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		client->sess.spectatorTime = level.time;
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum, qfalse );

	ClientBegin( clientNum, qfalse );
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;

	//IOq3 add
	SetClientViewAngle( ent, ent->client->ps.viewangles );

	// don't use dead view angles
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		ent->client->ps.stats[STAT_HEALTH] = 1;
	}
	//
}

/*
=================
Cmd_Team_f
=================
*/
//
//
//
//
qboolean G_SkipClient(gclient_t *client, int filter);
int G_TeamForString (const char *str, qboolean allowTeamPlaying);

//Used to set a bot's team by altering userinfo. Because a bot's team is always read from its userinfo string.
qboolean SetTeam_Bot (gentity_t *ent, const char *s) {
	int team;
	char userinfo[MAX_STRING_CHARS] = {0};
	int oldTeam;

	if (!ent || !ent->inuse || !ent->client)
		return qfalse;

	team = G_TeamForString( s, qfalse );

	if (team == -1)
		return qfalse;

	oldTeam = ent->client->sess.sessionTeam;

	trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
	Info_RemoveKey(userinfo, "team");
	Info_SetValueForKey(userinfo, "team", s);
	trap_SetUserinfo(ent->s.number, userinfo);


	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
	}

	ClientUserinfoChanged(ent - g_entities, qfalse);

	ent->client->sess.sessionTeam = team;
	ent->client->pers.teamState.state = TEAM_BEGIN;
	ClientBegin( ent - g_entities, qfalse);
	BroadcastTeamChange( ent->client, oldTeam );

	return qtrue;
}

//Called when a user tried to change his team himself
//separated so admins and code can use normal setteam, but setteam_user will check for locked teams, etc.
qboolean SetTeam_User( gentity_t *ent, const char *s ) {
	int oldTeam;
	int newTeam;
	const char *found = NULL;

	if (!ent || !ent->client)
		return qfalse;

	oldTeam = ent->client->sess.sessionTeam;

	//check if hes not allowed to change team
	if (ent->client->sess.amflags & AMFLAG_LOCKEDTEAM) {
		G_SendClientPrint( ent - g_entities, "You have been denied permission to change team.\n" );
		return qfalse;
	}

	#if 0	//i guess this is not standard behaviour..
	if (level.lockedTeams & (1 << oldTeam)) {
		//This client's current team is locked, so dont let him escape it
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", "Your team is locked; you can't leave it.") );
		return qfalse;
	}
	#endif

	if ( (newTeam = G_TeamForString(s, qfalse)) == -1) {
		G_SendClientPrint( ent - g_entities, "Unknown team '%s'.\n", s);
		return qfalse;
	}



	if ( ent->client->switchTeamTime > level.time ) {
		G_SendClientPrint( ent-g_entities, "%s\n", G_GetStripEdString("SVINGAME", "NOSWITCH") );
		return qfalse;
	}


	if (level.lockedTeams & (1 << newTeam)) {
		G_SendClientPrint( ent-g_entities, "That team is locked.\n" );
		return qfalse;
	}

	if (level.lockedTeams && newTeam == TEAM_FREE && g_gametype.integer >= GT_TEAM) {
		//prevent players from using team free which will assign players to either blue or red in team games,
		//when those teams are locked

		if ( !(level.lockedTeams & (1 << TEAM_RED)) )
			found = "r";
		else if ( !(level.lockedTeams & (1 << TEAM_BLUE)) )
			found = "b";
		else
			found = NULL;

		if (!found) {
			G_SendClientPrint( ent-g_entities, "Teams ^1RED^7 and ^4BLUE^7 are both locked.\n" );
			return qfalse;
		}

		s = found;
	}


	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT ) && oldTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}


	//all checks parsed, he can change team
	SetTeam( ent, (char*)s );
	ent->client->switchTeamTime = level.time + 3000;	// MOVED and reduced time from 5000
	return qtrue;
}

void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[32];	//should really not take more
	int argc = trap_Argc();

	if ( argc == 1 ) {
		oldTeam = ent->client->sess.sessionTeam;
		G_SendClientPrint( ent-g_entities, "You are on team %s^7.\n", G_TeamNameColoured( oldTeam ) );
		return;
	}
	else if (argc > 2) {
		G_SendClientPrint(ent-g_entities, "Usage: team [new team]\n");
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );
	SetTeam_User( ent, s );
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	// buf = G_GetStripEdString("SVINGAME", "FORCEPOWERCHANGED");

	// strcpy(fpChStr, buf);

	// trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap_Argv( 1, arg, sizeof( arg ) );

		if (arg && arg[0])
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}




/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[64];

	if ( trap_Argc() < 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		} else {
			G_SendClientPrint(ent - g_entities, "Usage: follow <client>\n");
		}
		return;
	}

	// trap_Argv( 1, arg, sizeof( arg ) );
	Q_strncpyz(arg, G_ConcatArgs(1), sizeof(arg));

	i = G_FindPlayerFromStringCheap(ent, arg, qtrue);

	if ( i == -1 ) {
		G_SendClientPrint(ent - g_entities, "Could not find client '%s'\n", arg);
		return;
	}

	if (i < 0 || i >= MAX_CLIENTS)
		return;

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		G_SendClientPrint(ent - g_entities, "Can't follow yourself.\n");
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		G_SendClientPrint(ent - g_entities, "Can't follow a spectator.\n");
		return;
	}

	// if they are playing a tournement game, count as a loss
	// if ( (g_gametype.integer == GT_TOURNAMENT )
		// && ent->client->sess.sessionTeam == TEAM_FREE ) {
		// ent->client->sess.losses++;
	// }

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		if (!SetTeam_User( ent, "spectator" ))
			return;	//He cant change his team to spec atm.
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	if (!ent || !ent->inuse)
		return;

	if ( ent->client->sess.spectatorState == SPECTATOR_NOT || ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		if (!SetTeam_User( ent, "s" ))
			return;	//user was not allowed to change his team at this time.
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients || clientnum >= MAX_CLIENTS ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;

		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}

	if (mode == SAY_TEAM) {
		if (!OnSameTeam(ent, other) ) {
			return;
		}
	}

	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		return;
	}

	if (other->client->sess.ignoredclients & ( 1 << (ent - g_entities) )) {
		return;
	}

	trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"",
		mode == SAY_TEAM ? "tchat" : "chat",
		name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

static qboolean Com_CharIsOneOfCharset( const char c, const char *set ) {
	int i;

	while (*set) {
		if (*(set++) == c)
			return qtrue;
	}

	return qfalse;
}

//like a strchr that checks for more than 1 char (checks for all letters in "set" variable)
qboolean G_StringContainsAnyInCharset(const char *str, const char *set) {

	if (!set)
		return qfalse;

	while ( *(str++) ) {
		if ( Com_CharIsOneOfCharset( *str, set ) )
			return qtrue;
	}

	return qfalse;
}

const char *SkipWhitespace2( const char *data ) {
	while (*data == ' ')
		++data;

	return data;
}

static char *G_GrabToken (const char *p, qboolean skipLeadingWhiteSpace) {
	static char token[256] = {0};
	int i = 0;
	int c;

	if (!p)
		return NULL;

	memset(&token, 0, sizeof(token));

	if (skipLeadingWhiteSpace)
		while (*p == ' ') ++p;


	while ( (c = *(p++)) != 0 && i < 255 ) {
		if (c == ' ')
			break;
		token[i++] = c;
	}
	token[i] = 0;

	return token;
}

typedef struct {
	const char	*name;
	const char 	*argsMask;
	const char 	*desc;
	void 		(*fun)(gentity_t *ent, const char *args);
} chatCommand_t;

static void Cmd_PauseGame_C (gentity_t *ent, const char *args);
static void Cmd_UnpauseGame_C (gentity_t *ent, const char *args);

static const chatCommand_t chatCmds[] = {
	{ "pause", NULL, "Pause the game", Cmd_PauseGame_C },
	{ "unpause", NULL, "Unpause the game", Cmd_UnpauseGame_C },
};
static const size_t numChatCommands = ARRAY_LEN( chatCmds );

static void Cmd_PauseGame_C (gentity_t *ent, const char *args) {
	int c;

	if (args && args[0]) {
		return;
	}

	if (!g_allowChatPause.integer) {
		G_SendClientPrint(ent - g_entities, "This command is currently disabled.\n");
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		G_SendClientPrint(ent - g_entities, "Only playing players can use this command.\n");
		return;
	}

	if (level.unpauseTime) {
		//game is about to be unpaused, so we dont care if the pause cvar is on. make sure it stays paused!
		level.unpauseTime = 0;
	}
	else if (g_pauseGame.integer) {
		G_SendClientPrint(ent - g_entities, "Game is already paused.\n");
		return;
	}

	level.unpauseClient = ent - g_entities + 1;

	G_SendClientCenterPrint(-1, "Game was paused by %s^7.\n", ent->client->pers.netname);
	G_LogPrintf("Game was paused by client %d\n", ent - g_entities);

	trap_SendConsoleCommand( EXEC_APPEND, PAUSEGAME_CVARNAME" 1\n");
}

static void Cmd_UnpauseGame_C (gentity_t *ent, const char *args) {
	int real;

	if (args && args[0]) {
		return;
	}

	if (!g_allowChatPause.integer) {
		G_SendClientPrint(ent - g_entities, "This command is currently disabled.\n");
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		G_SendClientPrint(ent - g_entities, "Only playing players can use this command.\n");
		return;
	}

	if (!g_pauseGame.integer) {
		G_SendClientPrint(ent - g_entities, "Game is not paused.\n");
		return;
	}

	/*
	real = level.unpauseClient - 1;

	if (real >= 0 && real < MAX_CLIENTS) {

		if (!g_entities[real].inuse || !g_entities[real].client) {
			//the guy who paused left. = anyone can unpause
		}
		else if (ent-g_entities == real) {
			//hes the one who paused, so he can unpause.
		}
		else if (level.clients[real].sess.sessionTeam != TEAM_SPECTATOR) {
			G_SendClientPrint(ent - g_entities, "Game was paused by %s ^7(%d); only he can unpause.\n", SHOWNAME(real), real);
			return;
		}
	}
	else {
		G_SendClientPrint(ent - g_entities, "no.\n");
		return;
	}
	*/

	if (level.unpauseTime) {
		G_SendClientPrint(ent - g_entities, "Unpause is already pending.\n");
		return;
	}

	G_LogPrintf("Game was unpaused by client %d\n", ent - g_entities);

	//instart of unpausing immediately, count down so players can prepare
	level.unpauseTime = level.time + UNPAUSE_COUNTDOWN;		//when level.time hits this, unpause!
}

//for limiting user args length so we're not wasting bandwidth echoing bad user requests.
const char *ShortString (const char *str) {
	static char buf[32] = {0};

	if (str)
		Q_strncpyz(buf, str, sizeof(buf));
	else
		buf[0] = 0;

	return &buf[0];
}

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[MAX_NETNAME+10];
	// don't let text be too long for malicious reasons
	// char		text[MAX_SAY_TEXT];
	char		text[192];	//increased
	int 		senderClientNum;

	if (!ent || !ent->inuse || !ent->client)
		return;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( G_StringContainsAnyInCharset(chatText, "\r\n") ) {
		G_SecurityLogPrint( "Malicious chat string", ent );
		return;
	}

	senderClientNum = ent - g_entities;

	// CHECK MUTE
	if (ent->client->sess.amflags & AMFLAG_MUTED) {
		trap_SendServerCommand( ent - g_entities, "cp \"You are muted.\"" );
		return;
	} else if ( (ent->client->sess.amflags & AMFLAG_MUTED_PUBONLY) && mode != SAY_TEAM) {
		trap_SendServerCommand( ent - g_entities, "cp \"You can only talk in team chat.\"" );
		return;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say     %c%02d %s: \"%s\"\n", ' ', ent-g_entities, ent->client->pers.netnameClean, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
	{
		char tt;
		int team = ent->client->sess.sessionTeam;

		if (team == TEAM_RED)
			tt = 'R';
		else if (team == TEAM_BLUE)
			tt = 'B';
		else
			tt = 'S';

		G_LogPrintf( "sayteam %c%02d %s: \"%s\"\n", tt, ent-g_entities, ent->client->pers.netnameClean, chatText );

		Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	}
	case SAY_TELL:
		Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}


	//Check for chat cmds.
	if (chatText[0] == '!') {
		const char *token = G_GrabToken(++chatText, qfalse);
		const chatCommand_t *cmd;

		for (j = 0, cmd = chatCmds; j < numChatCommands; ++j, ++cmd) {
			if (cmd && cmd->fun && !Q_stricmp(token, cmd->name)) {
				//only allow pause cmds in say_all
				if (cmd->fun == Cmd_PauseGame_C || cmd->fun == Cmd_UnpauseGame_C)
				{
					if (mode != SAY_ALL)
						break;
				}

				chatText += strlen(token);
				cmd->fun( ent, SkipWhitespace2(chatText) );
				break;
			}
		}
	}

	// send it to all the appropriate clients
	for (j = 0, other = g_entities; j < level.maxclients; ++j, ++other) {
		G_SayTo( ent, other, mode, color, name, text );
	}
}



/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: tell <client number> <message>\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients || targetNum >= MAX_CLIENTS ) {
		G_SendClientPrint( ent-g_entities, "tell: client slot %d is out of range.\n", targetNum);
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		G_SendClientPrint( ent-g_entities, "tell: client slot %d is not in use.\n", targetNum);
		return;
	}

	p = G_ConcatArgs( 2 );

	if ( G_StringContainsAnyInCharset(p, "\r\n") ) {
		G_SecurityLogPrint( "Malicious chat string", ent );
		return;
	}

	G_LogPrintf( "tell: \"%s\" (%d) to \"%s\" (%d): \"%s\"\n",
		ent->client->pers.netnameClean, ent-g_entities, target->client->pers.netnameClean, target-g_entities, p );

	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}


/* static */ const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Single Player",
	"Team FFA",
	"N/A",
	"Capture the Flag",
	"Capture the Ysalamiri"
};

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTE")) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "VOTEALREADY")) );
		return;
	}
	#if 0
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTEASSPEC")) );
		return;
	}
	#endif

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PLVOTECAST")) );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOVOTE")) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOTEAMVOTEINPROG")) );
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	if (ps->usingATST)
	{
		return 0;
	}

	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT );
		if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	default:
		return 1;
	}
}

extern int saberOffSound;
extern int saberOnSound;

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	if (!saberOffSound || !saberOnSound)
	{
		saberOffSound = G_SoundIndex("sound/weapons/saber/saberoffquick.wav");
		saberOnSound = G_SoundIndex("sound/weapons/saber/saberon.wav");
	}

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	if (g_pauseGame.integer)
		return;

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (ent->client->ps.saberHolstered)
		{
			ent->client->ps.saberHolstered = qfalse;
			G_Sound(ent, CHAN_AUTO, saberOnSound);
		}
		else
		{
			ent->client->ps.saberHolstered = qtrue;
			G_Sound(ent, CHAN_AUTO, saberOffSound);

			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
	}
}

void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;

	if ( !ent || !ent->client )
	{
		return;
	}

	if (ent->client->saberCycleQueue)
	{ //resume off of the queue if we haven't gotten a chance to update it yet
		selectLevel = ent->client->saberCycleQueue;
	}
	else
	{
		selectLevel = ent->client->ps.fd.saberAnimLevel;
	}

	selectLevel++;
	if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABERATTACK] )
	{
		selectLevel = FORCE_LEVEL_1;
	}

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->saberCycleQueue = selectLevel;
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT || g_gametype.integer >= GT_TEAM)
	{ //rather pointless in this mode..
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		return;
	}

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
	if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	if (G_OtherPlayersDueling())
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "CANTDUEL_BUSY")) );
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}

		if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELACCEPT"), ent->client->pers.netname) );

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				G_Sound(ent, CHAN_AUTO, saberOffSound);
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = qtrue;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				G_Sound(challenged, CHAN_AUTO, saberOffSound);
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = qtrue;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap_SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStripEdString("SVINGAME", "PLDUELCHALLENGE")) );
			trap_SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStripEdString("SVINGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

const char *G_ClientNameWithPrefix (gentity_t *ent, const int maxNameChars) {

	const char col = ColorCodeForClient(ent, qfalse);
	static char buf[256] = {0};

	memset(&buf, 0, 256);

	if (ent && ent->client)
		Com_sprintf(buf, sizeof(buf), "%02d^%c) ^7 %s", ent - g_entities, col, maxNameChars > 0 ?
			G_ClientNameFixedLength(ent, maxNameChars) :
			ent->client->pers.netname);
	else
		Q_strncpyz(buf, "?", sizeof(buf));

	return &buf[0];
}

#define G_MAXBUFNAMELEN	256		//...
//if clampMax is true, dont let string be longer than numChars visible chars
const char *G_StringFixedLength(const char* str, int numChars, char fillChar, qboolean clampMax) {
	int nc = 0, m;

	const char *ret;
	const int _strlen = strlen( str );
	static char outName[2][G_MAXBUFNAMELEN];
	static int index = 0;

	if (numChars <= 0)
		numChars = 1024;

	for (m = 0;  nc < numChars && m < G_MAXBUFNAMELEN-1 ; ++m, ++nc) {
		int c;

		if (m < _strlen) {
			c = str[m];

			if ( Q_IsColorString( str + m ) ) {
				nc -= 2;	//this char and the next will not count
			}
		}
		else if (!clampMax) {
			break;
		}
		else {
			c = fillChar;
		}

		outName[index][m] = c;
	}
	outName[index][m] = 0;

	ret = outName[index];
	index ^= 1;
	return ret;
}


const char *G_ClientNameFixedLength(gentity_t *ent, const int numChars) {
	return G_StringFixedLength(ent->client->pers.netname, numChars, ' ', qtrue);
}

//"cheap" findplayer version for use by clients. Only checks clean names, and only checks Q_stricmp and q_stristr (both case insensitive)
//ent = guy who wants to find someone.
int G_FindPlayerFromStringCheap (gentity_t *ent, const char *find, qboolean trySkipSpecs ) {
	int			i, k;
	int ret = -1;
	gclient_t 	*cl;

	if (!find || !find[0])
		return -1;

	if (ent && ent->client && ent->inuse && !Q_stricmp(find, "self")) {
		return ent - g_entities;
	}

	if (find[0] >= '0' && find[0] <= '9') {
		i = atoi(find);

		if (i >= 0 && i < MAX_CLIENTS && level.clients[i].pers.connected != CON_DISCONNECTED) {
			return i;
		}

		return -1;
	}

	for (k = 0; k < 2; ++k) {
		for ( i = 0 , cl = level.clients ; i < MAX_CLIENTS ; ++i, ++cl ) {
			const char	*name = NULL;

			if (!cl || cl->pers.connected == CON_DISCONNECTED)
				continue;

			if (trySkipSpecs && cl->sess.sessionTeam == TEAM_SPECTATOR)
				continue;

			name = cl->pers.netnameClean;

			switch(k)
			{
				case 0:
					if (!Q_stricmp(name, find)) ret = i;		//case insensitive string compare.
					break;
				case 1:
					if (Q_stristr(name, find)) ret = i;		//case insensitive substring search.
					break;
			}
		}
	}

	if (trySkipSpecs && ret == -1) {
		return G_FindPlayerFromStringCheap(ent, find, qfalse);
	}

	return ret;
}

//this thing is rather ineffective/slow, but that should not be a problem since its only used for server / rcon commands.
int G_FindPlayerFromString (const char *buf ) {
	int			i, k;
	gclient_t 	*cl;

	if (!buf || !buf[0])
		return -1;

	if (buf[0] >= '0' && buf[0] <= '9') {
		i = atoi(buf);

		if (i >= 0 && i < MAX_CLIENTS && level.clients[i].pers.connected != CON_DISCONNECTED) {
			return i;
		}

		return -1;
	}

	for (k = 0; k < 8; ++k) {
		for ( i = 0 , cl = level.clients ; i < MAX_CLIENTS ; ++i, ++cl ) {
			char		nameStr[256] = {0};
			const char	*name;

			if (!cl || cl->pers.connected == CON_DISCONNECTED)
				continue;


			if (k % 2) {
				name = cl->pers.netnameClean;
			} else {
				name = cl->pers.netname;
			}

			switch(k)
			{
				case 0:
				case 1:
					if (!strcmp(name, buf)) return i;			//case sensitive string compare.
					break;
				case 2:
				case 3:
					if (!Q_stricmp(name, buf)) return i;		//case insensitive string compare.
					break;
				case 4:
				case 5:
					if (strstr(name, buf)) return i;		//case sensitive substring search.
					break;
				case 6:
				case 7:
					if (Q_stristr(name, buf)) return i;		//case insensitive substring search.
					break;
			}
		}
	}

	return -1;	// Client isnt valid
}

char ColorCodeForClient (gentity_t *target, qboolean botSpecialColor) {
	char col = '7';

	//set a special color according to team or connect status
	if (botSpecialColor && target->r.svFlags & SVF_BOT) {
		col = '5';	//Cyan for bots!
	} else if (target->client->pers.connected == CON_CONNECTING) {
		col = '2';
	} else if (target->client->sess.sessionTeam == TEAM_BLUE) {
		col = '4';
	} else if (target->client->sess.sessionTeam == TEAM_RED) {
		col = '1';
	} else if (target->client->sess.sessionTeam == TEAM_SPECTATOR) {
		col = '3';
	}

	return col;

}

void G_SendClientPrint (const int client, const char *fmt, ...) {
	va_list		argptr;
	char		text[1024];

	if (!fmt)
		return;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);


	trap_SendServerCommand( client, va("print \"%s\"", text) );
}

void G_SendClientCenterPrint (const int client, const char *fmt, ...) {
	va_list		argptr;
	char		text[1024];

	if (!fmt)
		return;

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);


	trap_SendServerCommand( client, va("cp \"%s\"", text) );
}

static void Cmd_IgnoreClear_f (gentity_t *ent) {
	int i;
	const int clNum = ent - g_entities;

	if (!ent->client->sess.ignoredclients) {
		G_SendClientPrint(clNum, "Your ignorelist is already empty.\n");
	} else {
		G_SendClientPrint(clNum, "Your ignorelist was cleared.\n");
	}

	ent->client->sess.ignoredclients = 0;
}

static void Cmd_IgnoreList_f (gentity_t *ent) {
	int i;
	const int clNum = ent - g_entities;

	if (!ent->client->sess.ignoredclients) {
		G_SendClientPrint(clNum, "Your ignorelist is empty.\n");
		return;
	}

	G_SendClientPrint(clNum, "^5Ignorelist:\n");

	for (i = 0; i < MAX_CLIENTS; ++i) {
		if (level.clients[i].pers.connected == CON_DISCONNECTED)
			continue;

		if (ent->client->sess.ignoredclients & (1 << i)) {
			const char *str = va("  %02d^%c)^7 %s \n", i, ColorCodeForClient(&g_entities[i], qfalse), g_entities[i].client->pers.netname);

			G_SendClientPrint(clNum, str);
		}
	}
}

static void Cmd_Ignore_f (gentity_t *ent) {
	const int clNum = ent - g_entities;
	char buf[64] = {0};
	int him = -1;

	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( clNum, "print \"Usage: ignore <client name/number>\n\"" );
		return;
	}

	trap_Argv( 1, buf, sizeof( buf ) );

	if (!Q_stricmp(buf, "all")) {
		int i;
		qboolean ignore = qfalse;

		//check if all are ignored
		for (i = 0; i < MAX_CLIENTS; ++i) {
			if (level.clients[i].pers.connected == CON_DISCONNECTED)
				continue;

			if ( !(ent->client->sess.ignoredclients & (1 << i)) ) {
				// we must ignore all.
				ignore = qtrue;
				break;
			}
		}

		for (i = 0; i < MAX_CLIENTS; ++i) {
			if (level.clients[i].pers.connected == CON_DISCONNECTED)
				continue;

			if (ignore) {
				ent->client->sess.ignoredclients |= (1 << i);
			} else {
				ent->client->sess.ignoredclients &= ~(1 << i);
			}
		}

		if (ignore) {
			trap_SendServerCommand( clNum, "print \"All are now being ignored.\n\"" );
		} else {
			trap_SendServerCommand( clNum, "print \"All have been unignored.\n\"" );
		}

		return;
	}

	him = G_FindPlayerFromStringCheap(ent, buf, qfalse);

	if (him == -1) {
		trap_SendServerCommand( clNum, va("print \"Couldn't find client %s\n\"", buf) );
		return;
	}

	if (level.clients[him].pers.connected == CON_DISCONNECTED)
		return;

	if ( ent->client->sess.ignoredclients & (1 << him) ) {
		ent->client->sess.ignoredclients &= ~(1 << him);

		G_SendClientPrint(clNum, "%s ^7(%d) is no longer ignored.\n", SHOWNAME(him), him );
	} else {
		ent->client->sess.ignoredclients |= (1 << him);
		G_SendClientPrint(clNum, "%s ^7(%d) is now being ignored.\n", SHOWNAME(him), him);
	}
}


static void Cmd_ClientStatus_f (gentity_t *ent) {
	int i, c = 0;
	int filter = -1;
	const int clNum = ent - g_entities;

	gentity_t *target;

	if ( trap_Argc() == 2 ) {
		const char *arg = G_Argv( 1 );

		if ( (filter = G_TeamForString(arg, qtrue)) == -1 ) {
			G_SendClientPrint(clNum, "Unknown team '%s'\n", arg);
			return;
		}
	}
	else if (trap_Argc() > 2) {
		G_SendClientPrint(clNum, "Usage: clientstatus [team]\n");
		return;
	}

	if (filter != -1)
		G_SendClientPrint(clNum, "Clients on %s^7 team:\n", G_TeamNameColoured(filter));

	target = g_entities;

	for ( i = 0 ; i < MAX_CLIENTS ; ++i, ++target ) {
		if (!target || !target->client || !target->inuse || target->client->pers.connected == CON_DISCONNECTED) {
			continue;
		}

		if (G_SkipClient(target->client, filter))
			continue;


		G_SendClientPrint( clNum, "%s\n", G_ClientNameWithPrefix(target, 0) );
		++c;
	}

	if (!c)
		G_SendClientPrint( clNum, "<no clients to list>\n" );
}

static void Cmd_SpectatorList_f (gentity_t *sendToMe) {
	int i;
	gentity_t *ent = g_entities;
	const int clNum = sendToMe - g_entities;
	int c = 0;
	qboolean matchme = qfalse;

	if (!Q_stricmp(G_Argv(1), "me"))
		matchme = qtrue;

	G_SendClientPrint(clNum, "%s%24s     %s\n", "     ", "^5spectators", "speccing");

	for (i = 0; i < MAX_CLIENTS; ++i, ++ent) {

		if (!ent || !ent->client || ent->client->pers.connected != CON_CONNECTED || ent->client->sess.sessionTeam != TEAM_SPECTATOR)
			continue;

		if (matchme && ent->client->ps.clientNum != clNum)
			continue;

		if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
			ent->client->ps.clientNum >= 0 && ent->client->ps.clientNum < MAX_CLIENTS) {

			char extra[8] = {0};

			if (ent->client->ps.clientNum == clNum) {
				//he is speccing me
				Q_strncpyz(extra, " (YOU)", sizeof(extra));
			}

			G_SendClientPrint( clNum, "%s^7   %s^7 (%d)%s\n", G_ClientNameWithPrefix(ent, 24),
				SHOWNAME(ent->client->ps.clientNum), ent->client->ps.clientNum, extra );
		} else {
			G_SendClientPrint( clNum, "%24s^7   (nobody)\n", G_ClientNameWithPrefix(ent, 24) );
		}
		++c;
	}

	if (!c)
		G_SendClientPrint( clNum, "<no spectators to list>\n" );
}


const char *G_Argv(int arg) {
	static char buf[256];

	trap_Argv(arg, buf, 256);

	return &buf[0];
}

void *bsearch(const void *key, const void *base, size_t num, size_t width, int (*compare)(const void *, const void *)) {
	char *lo = (char *) base;
	char *hi = (char *) base + (num - 1) * width;
	char *mid;
	unsigned int half;
	int result;

	while (lo <= hi) {
		if (half = num / 2) {
			mid = lo + (num & 1 ? half : (half - 1)) * width;

			if (!(result = (*compare)(key,mid))) {
				return mid;
			} else if (result < 0) {
				hi = mid - width;
				num = num & 1 ? half : half - 1;
			} else {
				lo = mid + width;
				num = half;
			}

		} else if (num) {
			return ((*compare)(key, lo) ? NULL : lo);
		} else {
			break;
		}
	}

	return NULL;
}

/*
=================
ClientCommand
=================
*/


void Cmd_AddBot_f( gentity_t *ent ) {
	//because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
	trap_SendServerCommand( ent - g_entities, va("print \"%s.\n\"", G_GetStripEdString("SVINGAME", "ONLY_ADD_BOTS_AS_SERVER")));
}

void Cmd_FollowNext_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, 1 );
}

void Cmd_FollowPrev_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, -1 );
}

static void Cmd_Say_f( gentity_t *ent ) {
	char *p = NULL;

	if ( trap_Argc () < 2 ) {
		G_SendClientPrint(ent - g_entities, "Usage: say <message>\n");
		return;
	}

	p = G_ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_ALL, p );
}

static void Cmd_SayTeam_f( gentity_t *ent ) {
	char *p = NULL;

	if ( trap_Argc () < 2 ) {
		G_SendClientPrint(ent - g_entities, "Usage: say_team <message>\n");
		return;
	}

	p = G_ConcatArgs( 1 );

	G_Say( ent, NULL, (g_gametype.integer>=GT_TEAM) ? SAY_TEAM : SAY_ALL, p );
}

void Cmd_TheDestroyer_f( gentity_t *ent ) {

	if ( ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
			if (ent->client->ps.dualBlade)
			{
				ent->client->ps.dualBlade = qfalse;
			}
			else
			{
				ent->client->ps.dualBlade = qtrue;

				trap_SendServerCommand( -1, va("print \"%sTHE DESTROYER COMETH\n\"", S_COLOR_RED) );
				G_ScreenShake(vec3_origin, NULL, 10.0f, 800, qtrue);
			}
		}
	}
}

static void Cmd_AltFolow_f( gentity_t *ent ) {
	if (!ent || !ent->client)
		return;

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		G_SendClientPrint(ent - g_entities, "You need to be on spectator team to use this feature.\n");
		return;
	}


	//setting persists through maprestarts
	ent->client->sess.amflags ^= AMFLAG_ALTFOLLOW;
	if (ent->client->sess.amflags & AMFLAG_ALTFOLLOW)
		G_SendClientPrint(ent - g_entities, "Alt-follow is now ^2enabled^7 (use alt-attack to spectate the previous player.)\n");
	else
		G_SendClientPrint(ent - g_entities, "Alt-follow is now ^1disabled^7.\n");
}

// AFK command for checking which players are afk
#define AFK_SECONDS		20
static void Cmd_AfkList_f (gentity_t *sendto) {
	int i, count = 0;
	gentity_t *c;
	const int secs = AFK_SECONDS;
	const int clnum = sendto - g_entities;
	gentity_t *sortedClients[MAX_CLIENTS];

	G_SendClientPrint(clnum, "Non-spectating players who are inactive for more than ^1%d^7 seconds:\n", secs);

	for (i = 0, c = g_entities; i < MAX_CLIENTS; ++i, ++c) {
		int diff;

		if (!c || !c->inuse || !c->client || c->client->pers.connected != CON_CONNECTED)
			continue;

		if (c->r.svFlags & SVF_BOT)
			continue;

		if (G_SkipClient(c->client, TEAM_PLAYING))
			continue;

		diff = level.time - c->client->pers.lastActionTime;

		diff /= 1000;	//diff is now seconds since last action

		if (diff > secs)
			sortedClients[ count++ ] = c;
	}

	if (!count)
		G_SendClientPrint(clnum, "<no players to list>\n");
	else {
		gentity_t *ent;

		//clients who have been afk longest will be shown first.
		qsort(sortedClients, count, sizeof(gentity_t**), clientafkcmp);

		for (i = 0; i < count; ++i) {
			ent = sortedClients[i];
			G_SendClientPrint(clnum, "%s ^7: %s\n", G_ClientNameWithPrefix(ent, 20), G_MsToString(level.time - ent->client->pers.lastActionTime));
		}
	}
}

//Stuff from openJK
#define CMD_NOINTERMISSION		(1<<0)
#define CMD_CHEAT				(1<<1)
#define CMD_ALIVE				(1<<2)

typedef struct command_s {
	const char	*name;	//string that invokes the command
	const char  *args;	//Args mask
	const char  *desc;	//description
	void		(*func)(gentity_t *ent);	//function to execute
	int			flags;
} clientCommand_t;

//for bsearch
int cmdcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((clientCommand_t*)b)->name );
}

static void Cmd_Help_f (gentity_t *ent);
clientCommand_t clientCommands[] = {
	{ "addbot",				NULL, NULL, Cmd_AddBot_f,				0 },
	{ "callteamvote",		NULL, NULL, Cmd_CallTeamVote_f,			CMD_NOINTERMISSION },
	{ "callvote",			NULL, NULL, Cmd_CallVote_f,				CMD_NOINTERMISSION },
	{ "follow",				NULL, NULL, Cmd_Follow_f,				CMD_NOINTERMISSION },
	{ "follownext",			NULL, NULL, Cmd_FollowNext_f,			CMD_NOINTERMISSION },
	{ "followprev",			NULL, NULL, Cmd_FollowPrev_f,			CMD_NOINTERMISSION },
	{ "forcechanged",		NULL, NULL, Cmd_ForceChanged_f,			0 },
	{ "give",				NULL, NULL, Cmd_Give_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "god",				NULL, NULL, Cmd_God_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "kill",				NULL, NULL, Cmd_Kill_f,					CMD_NOINTERMISSION },
	{ "noclip",				NULL, NULL, Cmd_Noclip_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "notarget",			NULL, NULL, Cmd_Notarget_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "say",				NULL, NULL, Cmd_Say_f,					0 },
	{ "say_team",			NULL, NULL, Cmd_SayTeam_f,				0 },
	{ "score",				NULL, NULL, Cmd_Score_f,				0 },
	{ "setviewpos",			NULL, NULL, Cmd_SetViewpos_f,			CMD_CHEAT|CMD_NOINTERMISSION },
	{ "team",				NULL, NULL, Cmd_Team_f,					CMD_NOINTERMISSION },
	{ "teamvote",			NULL, NULL, Cmd_TeamVote_f,				CMD_NOINTERMISSION },
	{ "tell",				NULL, NULL, Cmd_Tell_f,					0 },
	{ "thedestroyer",		NULL, NULL, Cmd_TheDestroyer_f,			CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "vote",				NULL, NULL, Cmd_Vote_f,					CMD_NOINTERMISSION },

	{"clientstatus", "[team]", "Print a list of players on the server.", Cmd_ClientStatus_f, 0 },
    {"specs", "", "Print a list of spectators and whom they are spectating.", Cmd_SpectatorList_f, CMD_NOINTERMISSION },
    {"ignore", "<client or 'all'>", "Ignore/unignore a player or everyone.", Cmd_Ignore_f, 0 },
    {"ignorelist", "", "Print a list of players you have on ignore.", Cmd_IgnoreList_f, 0 },
    {"ignoreclear", "", "Remove all players from your ignorelist.", Cmd_IgnoreClear_f, 0 },
    {"altf", "", "Toggle being able to use alt-attack to spec the previous player.", Cmd_AltFolow_f, CMD_NOINTERMISSION },
	{"afk", "", "See a list of players who haven't touched any buttons lately.", Cmd_AfkList_f, CMD_NOINTERMISSION },
    {HELP_CMD, "", NULL/*"Display this list."*/, Cmd_Help_f, 0 },
};

static const size_t numCommands = ARRAY_LEN( clientCommands );

static void Cmd_Help_f ( gentity_t *ent )
{
	const clientCommand_t 	*cmd;
	const int 				entNum	= ent - g_entities;
	int 					i;

	G_SendClientPrint(entNum, "This server is running ^1[ vVv ]^7 serverside mode by Padaget. (Build date: %s, version %s)\n", __DATE__, MOD_VERSION);
	G_SendClientPrint(entNum, "The project is open-source:   http://github.com/jk2vvv/vVv-serverside \n");

	G_SendClientPrint(entNum, "--- CLIENT COMMANDS ---\n");

	for ( i = 0, cmd = clientCommands ; i < numCommands; ++cmd, ++i )
	{
		if ( cmd && cmd->desc )
			G_SendClientPrint( entNum, " ^1/%s ^7%s\n"
				 "    %s\n\n", cmd->name, cmd->args, cmd->desc );
	}

	if ( g_allowChatPause.integer )
		G_SendClientPrint(entNum, "Pausing/unpausing is currently enabled. Game can be paused by typing ^1!pause^7 in chat, and unpaused by ^1!unpause^7.\n");
}


//For qsort
int cmdcmp2( const void *a, const void *b ) {
	return strcmp ( ((clientCommand_t*)a)->name, ((clientCommand_t*)b)->name );
}

void G_InitClientCommands (void) {
	//Sort the commands array alphabetically to allow for binary search.
	qsort( clientCommands, numCommands, sizeof(clientCommands[0]), cmdcmp2 );
}

//Now uses binary search as in openjk.
void ClientCommand( int clientNum ) {
	gentity_t 		*ent;
	char			cmd[MAX_TOKEN_CHARS];
	clientCommand_t *command;

	// DebugStuff("start of clientcommand");

	trap_Argv( 0, cmd, sizeof( cmd ) );

	ent = g_entities + clientNum;
	if ( !ent || !ent->client || !ent->inuse || ent->client->pers.connected != CON_CONNECTED ) {
		G_SecurityLogPrint( va("ClientCommand \"%s\" without an active connection", cmd), ent );
		return;		// not fully in game yet
	}

	ent->client->pers.lastActionTime = level.time;

	//rww - redirect bot commands
	if (G_StringStartsWithCaseIns(cmd, "bot_") && AcceptBotCommand(cmd, ent))
		return;

	command = (clientCommand_t *)bsearch( cmd, clientCommands, numCommands, sizeof( clientCommands[0] ), cmdcmp );
	if ( !command )	{
		G_SendClientPrint( clientNum, "Unknown command ~%s~\n", ShortString(cmd) );
	}
	else if ( (command->flags & CMD_NOINTERMISSION)	&& ( level.intermissionQueued || level.intermissiontime ) )
	{
		G_SendClientPrint( clientNum, "You cannot perform this task during the intermission.\n" );
	}
	else if ( (command->flags & CMD_CHEAT) && !g_cheats.integer )
	{
		trap_SendServerCommand( clientNum, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NOCHEATS")));
	}
	else if ( (command->flags & CMD_ALIVE) && (ent->health <= 0 || ent->client->sess.sessionTeam == TEAM_SPECTATOR) )
	{
		trap_SendServerCommand( clientNum, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "MUSTBEALIVE")));
	}
	else
		command->func( ent );

	// DebugStuff("end of clientcommand");
}

