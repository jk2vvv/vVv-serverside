// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"

level_locals_t	level;



gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

qboolean gDuelExit = qfalse;

vmCvar_t	g_gametype;
vmCvar_t	g_MaxHolocronCarry;
vmCvar_t	g_ff_objectives;
vmCvar_t	g_autoMapCycle;
vmCvar_t	g_dmflags;
vmCvar_t	g_maxForceRank;
vmCvar_t	g_forceBasedTeams;
vmCvar_t	g_privateDuel;
vmCvar_t	g_saberLocking;
vmCvar_t	g_forceRegenTime;
vmCvar_t	g_spawnInvulnerability;
vmCvar_t	g_forcePowerDisable;
vmCvar_t	g_weaponDisable;
vmCvar_t	g_duelWeaponDisable;
vmCvar_t	g_fraglimit;
vmCvar_t	g_duel_fraglimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_capturelimit;
vmCvar_t	g_saberInterpolate;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_friendlySaber;
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_fps;
vmCvar_t	g_timescale;
vmCvar_t	g_knockback;
vmCvar_t	g_quadfactor;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_adaptRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;

vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_log;
vmCvar_t	g_logSync;
vmCvar_t	g_statLog;
vmCvar_t	g_statLogFile;
vmCvar_t	g_blood;
vmCvar_t	g_podiumDist;
vmCvar_t	g_podiumDrop;
vmCvar_t	g_allowVote;
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_debugForward;
vmCvar_t	g_debugRight;
vmCvar_t	g_debugUp;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
#ifndef LITE
vmCvar_t	pmove_float;
#endif
vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
vmCvar_t	g_redteam;
vmCvar_t	g_blueteam;
vmCvar_t	g_singlePlayer;
vmCvar_t	g_enableDust;
vmCvar_t	g_enableBreath;
vmCvar_t	g_dismember;
vmCvar_t	g_forceDodge;
vmCvar_t	g_timeouttospec;


//new cvars
vmCvar_t	g_pauseGame;
vmCvar_t	g_minmsec;
vmCvar_t	g_maxmsec;
vmCvar_t	g_minefix;
vmCvar_t	g_moverfix;
vmCvar_t	g_fairflag;
vmCvar_t	g_allowChatPause;
vmCvar_t	g_logSecurity;
vmCvar_t	g_logKills;
vmCvar_t	g_logItems;
vmCvar_t	g_logbs;

int gDuelist1 = -1;
int gDuelist2 = -1;

// bk001129 - made static to avoid aliasing
/* static */ cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse },
	{ &g_fps, "sv_fps", "", 0, 0, qtrue },
	{ &g_timescale, "timescale", "", 0, 0, qtrue },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_MaxHolocronCarry, "g_MaxHolocronCarry", "3", /* CVAR_SERVERINFO | */ CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	// change anytime vars
	{ &g_ff_objectives, "g_ff_objectives", "0", /*CVAR_SERVERINFO |*/  CVAR_NORESTART, 0, qtrue },

	{ &g_autoMapCycle, "g_autoMapCycle", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },

	{ &g_maxForceRank, "g_maxForceRank", "6", /* CVAR_SERVERINFO | */ CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_forceBasedTeams, "g_forceBasedTeams", "0",/*  CVAR_SERVERINFO | */ CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },
	{ &g_privateDuel, "g_privateDuel", "1", /* CVAR_SERVERINFO | */ CVAR_ARCHIVE, 0, qtrue  },
	{ &g_saberLocking, "g_saberLocking", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	{ &g_forceRegenTime, "g_forceRegenTime", "200", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },

	{ &g_spawnInvulnerability, "g_spawnInvulnerability", "3000", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_forcePowerDisable, "g_forcePowerDisable", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },
	{ &g_weaponDisable, "g_weaponDisable", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },
	{ &g_duelWeaponDisable, "g_duelWeaponDisable", "1", /* CVAR_SERVERINFO | */ CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  },

	{ &g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_duel_fraglimit, "duel_fraglimit", "10", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },


	{ &g_pauseGame, PAUSEGAME_CVARNAME, "0", CVAR_VVV, 0, qtrue, qfalse, "Pauses the game, preventing players from moving, items from respawning, etc."  },
	{ &g_minmsec, "g_minmsec", "0", CVAR_VVV|CVAR_ARCHIVE/* |CVAR_SERVERINFO */, 0, qfalse, qfalse,
		"Setting for limiting high fps physics. An example value of 4: fps physics where the fps is over 1000/4=250 will get blocked, and the player cannot roll nor accelerate in air.\n"
		"    For blocking 333 fps physics, set the value to 4. To disable, set to 0." },		//
	{ &g_maxmsec, "g_maxmsec", "0", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse,
		"This variable can be used to block low fps rolling. Example value of 22: rolls done with fps under 1000/22=45 will be blocked." },
	{ &g_minefix, "g_minefix", "1", CVAR_VVV|CVAR_ARCHIVE, 0, qtrue, qfalse, "This setting is a fix to the behavior where mines that are dropped by a player always will have an ammo count of 3. There are several values:\n"
	"1 : ammo count will always be the true amount, no matter if the player suicided or was killed\n"
	"2 : ammo count will only be the true amount if the player who dropped them suicided\n"
	"In addition, using a value of -1 or -2 will work the same way as 1 and 2, however, the ammo count will be a maximum of 3."
	},

	{ &g_moverfix, "g_moverfix", "1", CVAR_VVV|CVAR_ARCHIVE/* |CVAR_SERVERINFO */, 0, qfalse, qfalse, "If set, dead bodies will not block side doors in ctf_yavin."  },
	{ &g_logSecurity, "g_logSecurity", "security.log", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse, "Log connects and disconnects to this file"  },
	{ &g_logKills, "g_logKills", "1", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse  },
	{ &g_logItems, "g_logItems", "1", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse  },

	#ifdef ANALYZE_BS
	{ &g_logbs, "g_logbs", "0", CVAR_ARCHIVE|CVAR_VVV, 0, qfalse, qfalse, "Log all d/bs events to disk"  },
	#endif


	{ &g_fairflag, "g_fairflag", "1", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse, "If the setting is enabled: in situations where more than one player is standing/touching a ctf flag, checks will be made to ensure that the guy standing closest to it will get/cap it, as an alternative to randomness deciding who should get it."  },
	{ &g_allowChatPause, "g_allowChatPause", "0", CVAR_VVV|CVAR_ARCHIVE, 0, qfalse, qfalse, "Players not on spectator team can pause/unpause the game by using !pause and !unpause in chat."  },



	{ &g_saberInterpolate, "g_saberInterpolate", "1", CVAR_ARCHIVE, 0, qtrue },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_friendlySaber, "g_friendlySaber", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", 0, 0, qtrue  },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_statLog, "g_statLog", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_statLogFile, "g_statLogFile", "statlog.log", CVAR_ARCHIVE, 0, qfalse },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

	{ &g_speed, "g_speed", "250", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue  },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "5", 0, 0, qtrue },
	{ &g_adaptRespawn, "g_adaptrespawn", "1", 0, 0, qtrue  },		// Make weapons respawn faster with a lot of players.
	{ &g_forcerespawn, "g_forcerespawn", "60", 0, 0, qtrue },		// One minute force respawn.  Give a player enough time to reallocate force.
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },
	{ &g_blood, "com_blood", "1", 0, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },


	{ &g_redteam, "g_redteam", "Empire", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue },
	{ &g_blueteam, "g_blueteam", "Rebellion", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue  },
	{ &g_singlePlayer, "ui_singlePlayerActive", "", 0, 0, qfalse, qfalse  },

	{ &g_enableDust, "g_enableDust", "0", 0, 0, qtrue, qfalse },
	{ &g_enableBreath, "g_enableBreath", "0", 0, 0, qtrue, qfalse },
	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},
	#ifndef LITE
	{ &pmove_float, "pmove_float", "0", CVAR_SYSTEMINFO, 0, qfalse, qfalse, "If set, don't snap playerstate velocity vectors to integers. If set to 2, only snap the Z component (so jump heights still will be framerate dependant)"},
	#endif
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse},

	{ &g_dismember, "g_dismember", "0", 0, 0, qtrue  },
	{ &g_forceDodge, "g_forceDodge", "1", 0, 0, qtrue  },

	{ &g_timeouttospec, "g_timeouttospec", "70", CVAR_ARCHIVE, 0, qfalse },
};

// bk001129 - made static to avoid aliasing
/* static */ int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );


void G_InitGame					( int levelTime, int randomSeed, int restart );
void G_RunFrame					( int levelTime );
void G_ShutdownGame				( int restart );
void CheckExitRules				( void );
void G_ROFF_NotetrackCallback	( gentity_t *cent, const char *notetrack);

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	// DebugStuff("vmMain start");

	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		// DebugStuff("GAME_INIT end");
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (int)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		// DebugStuff("ClientThink end");
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		{
			gclient_t *cl = &level.clients[ arg0 ];

			//This client already requested new userinfo but they havent taken effect yet.
			if (cl->pers.userinfoRequestTime) {
				return 0;
			}


			if (level.time - cl->pers.userinfoChangeTime <= FLOODCONTROL_MS) {
				// It is checked in the main game loop when to apply his new clientinfo.
				cl->pers.userinfoRequestTime = level.time + FLOODCONTROL_MS;
				// trap_SendServerCommand( arg0, va("print \"Your userinfo change has been postponed and will take effect in ^5%d^7 second.\n\"",
						// FLOODCONTROL_MS / 1000) );
			} else {
				//OK, apply the new client info immediately, np.
				ClientUserinfoChanged( arg0, qtrue );
			}

		}
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		// DebugStuff("ClientDisconnect end");
		return 0;
	case GAME_CLIENT_BEGIN:
		//This one is called after bad people have tried to eat flags and on transition between teams and stuff.
		//However, we are sure this one is called by the engine after the flag eating cmd, so we only need to check for some stuff here and not elsewhere.
		//Basically, if this guy appears to have any flag, return it.
		#define FLAGEAT
		#ifdef FLAGEAT
		if (arg0 >= 0 && arg0 < MAX_CLIENTS) {
			gentity_t *ent = &g_entities[arg0];
			qboolean naughty = qfalse;

			if (ent && ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
				if (ent->client->ps.powerups[PW_REDFLAG]) {
					Team_ReturnFlag( TEAM_RED );
					naughty = qtrue;
				}
				if (ent->client->ps.powerups[PW_BLUEFLAG]) {
					Team_ReturnFlag( TEAM_BLUE );
					naughty = qtrue;
				}
			}

			if (naughty) {
				//we just returned flag instantly in above code, so make sure the flag doesnt drop to the ground in ClientDisconnect when hes kicked.
				ent->client->ps.powerups[PW_BLUEFLAG] = ent->client->ps.powerups[PW_REDFLAG] = 0;

				G_SecurityLogPrint( "Flag eating attempt", ent );
				trap_DropClient(arg0, "was kicked for trying to do very bad thing!");
				return 0;
			}
		}
		#endif
		ClientBegin( arg0, qtrue );
		// DebugStuff("ClientBegin end");
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		// DebugStuff("ClientCommand end");
		return 0;
	case GAME_RUN_FRAME:
		// DebugStuff("G_RunFrame");
		G_RunFrame( arg0 );
		// DebugStuff("G_RunFrame end");
		return 0;
	case GAME_CONSOLE_COMMAND:
		// DebugStuff("GAME_CONSOLE_COMMAND");
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		// DebugStuff("BOTAI_START_FRAME");
		return BotAIStartFrame( arg0 );
	case GAME_ROFF_NOTETRACK_CALLBACK:
		G_ROFF_NotetrackCallback( &g_entities[arg0], (const char *)arg1 );
	}

	return -1;
}

void	trap_SendServerCommandReal( int clientNum, const char *text );	//this is the syscall

//is faster than strlen when the string is longer than len
qboolean StringIsLongerThan (const char *s, int len) {
	++len;

	while ( *(s++) ) {
		if (!--len)
			return qtrue;
	}

	return qfalse;
}

#ifdef Q3_VM
void trap_SendServerCommand( int clientNum, const char *text ) {
	//q3msgboom exploit
	// if (strlen(text) > 1022) {
	#if 0
	if (StringIsLongerThan(text, 1022)) {
		char snip[128];
		Q_strncpyz(snip, text, sizeof(snip));

		//dont attempt to log the long string, but a snippet
		G_SecurityLogPrint( va("trap_SendServerCommand to client %d exceeded 1022 chars. Snippet: \"%s\"\n", clientNum, snip), NULL );
		return;
	}
	#endif

	trap_SendServerCommandReal(clientNum, text);
}
#endif


void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Printf( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	trap_Error( text );
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

void G_RemapTeamShaders( void ) {
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags & (~CVAR_VVV) );	//engine doesn't need to know about vvv flag
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

		if (cv->teamShader) {
			remapped = qtrue;
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) {
		G_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		trap_Cvar_Set( "g_gametype", "0" );
		trap_Cvar_Update( &g_gametype );
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}


void CalcViewAngle( playerState_t *ps, const usercmd_t *cmd, vec3_t out ) {
	short		temp;
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;		// no view changes at all
	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		out[i] = SHORT2ANGLE(temp);
	}
}

/*
=================
G_UpdateCvars
=================
*/
extern qboolean pmovePausedgame;

int pauseGameStartTime = 0;

#ifdef DEBUGSTUFF
int *DEBUGSTUFFval = NULL;

void DebugStuff(const char *s) {
	#if 0
	static int lastShit = -1337;

	if (DEBUGSTUFFval && *DEBUGSTUFFval != lastShit) {
		G_Printf("-------------DEBUGSTUFFval went from %d to %d after '%s' and before '%s'\n", lastShit, *DEBUGSTUFFval, lastPlace, s);
		lastShit = *DEBUGSTUFFval;
	}
	#else

	static char userinfos[MAX_CLIENTS][1024] = {0};
	static char lastPlace[128]= {0};
	int i;

	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		char test[1024] = {0};
		const char *val1, *val2;

		static const char *fields[] = {
			"rate", "snaps", "team_model", "model", "forcepowers", "cl_anonymous", "color2", "color1"
		};
		static const int numfields = ARRAY_LEN(fields);
		int k;

		if (level.clients[i].pers.connected == CON_DISCONNECTED)
			continue;

		trap_GetUserinfo(i, test, sizeof(test));

		for (k = 0; k < numfields; ++k)
		{
			val1 = Info_ValueForKey(userinfos[i], fields[k]);
			val2 = Info_ValueForKey(test, fields[k]);
			if (*val1 && !*val2)
				G_LogPrintf("-------------'%s' of client %d went from \"%s\" to \"%s\" after '%s' and before '%s'\n",
					fields[k], i, val1, val2, lastPlace, s);
		}

		Q_strncpyz(userinfos[i], test, sizeof(userinfos[i]));
	}

	Q_strncpyz(lastPlace, s, sizeof(lastPlace));

	#endif
}
#else
void DebugStuff(const char *s){
}
#endif


void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	#ifdef DEBUGSTUFF
	// DebugStuff("G_UpdateCvars start");
	#endif

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {

			#ifdef DEBUGSTUFF
/* 			if (!Q_stricmp(cv->cvarName, "dedicated")) {
				if (!DEBUGSTUFFval) {
					DEBUGSTUFFval = &cv->vmCvar->handle;
					G_Printf("Debug val set.\n");
				}
				// G_Printf("vmCvar->handle=%d, name='%s'\n", cv->vmCvar->handle, cv->cvarName);
			}

			if (cv->vmCvar->handle < 0 || cv->vmCvar->handle > 1000) {
				G_Printf("WARNING: cvar '%s' handle %d\n",cv->cvarName, cv->vmCvar->handle);
			} */
			#endif

			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"",
						cv->cvarName, cv->vmCvar->string ) );
				}

				if (cv->vmCvar == &g_motd) {
					//motd string was updated.
					trap_SetConfigstring( CS_MOTD, g_motd.string );		// message of the day
				}

				// hack to make smooth pauses
				// Thanks to Daggolin for this tip!
				#if 1
				if (cv->vmCvar == &g_pauseGame) {
					int val;
					int k;

					if ( cv->vmCvar->integer )
						val = 1;
					else
						val = 0;

					/*trap_SendConsoleCommand( EXEC_APPEND, va("g_synchronousClients %d\n", val) );*/

					if (val) {
						gentity_t *ent;
						pauseGameStartTime = level.time;
						G_SendClientPrint(-1, "Game was paused.\n");

						//save clients' viewangles..
						for (k = 0, ent = g_entities; k < MAX_CLIENTS; ++k, ++ent) {
							if (ent && ent->client && ent->client->pers.connected != CON_DISCONNECTED) {
								VectorCopy(ent->client->ps.viewangles, ent->client->pauseSavedViewangles);
							}
						}
					} else {
						// PAUSE STOPPED
						level.unpauseClient = -1;

						//SAFETY CHECK
						if (pauseGameStartTime > 0 && pauseGameStartTime < level.time) {
							//postpone all think functions
							gentity_t *ent;
							const int pauseDuration = level.time - pauseGameStartTime;

							//postpone think functions that were blocked during pause.
							for ( k = MAX_CLIENTS; k < MAX_GENTITIES; ++k )	{
								ent = &g_entities[ k ];
								if ( ent && ent->inuse && ent->think && ent->nextthink > 0 )	{
									ent->nextthink += pauseDuration;
								}
							}

							// G_LogPrintf("Pause stopped after %s.\n", pauseGameStartTime, G_MsToString(pauseDuration));
							G_SendClientPrint(-1, "Pause ended after %s.\n", G_MsToString(pauseDuration));

							level.startTime += pauseDuration;

							//Roll back the time that cg_drawTimer shows
							trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

							pauseGameStartTime = 0;

							//fix so times are correct on scoreboard. we dont wanna count time while game is paused
							for (k = 0, ent = g_entities; k < MAX_CLIENTS; ++k, ++ent) {
								if (ent && ent->client && ent->client->pers.connected != CON_DISCONNECTED) {
									ent->client->pers.enterTime += pauseDuration;

									//ok, in case someone joined during pause, ensure they dont get negative time..
									if (ent->client->pers.enterTime > level.time)
										ent->client->pers.enterTime = level.time;


									if (ent->client->pers.teamState.flagsince) {
										//This guy is holding a flag. Update the timer so its not counting pause time.
										ent->client->pers.teamState.flagsince += pauseDuration;

										if (ent->client->pers.teamState.flagsince > level.time)
											ent->client->pers.teamState.flagsince = level.time;
									}
									if (ent->client->pers.teamState.lastreturnedflag) {
										ent->client->pers.teamState.lastreturnedflag += pauseDuration;
									}
									if (ent->client->pers.teamState.lastfraggedcarrier) {
										ent->client->pers.teamState.lastfraggedcarrier += pauseDuration;
									}
									if (ent->client->pers.teamState.lasthurtcarrier) {
										ent->client->pers.teamState.lasthurtcarrier += pauseDuration;
									}

									if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
										//restore this clients viewangles as the same as before pause.
										SetClientViewAngle(ent, ent->client->pauseSavedViewangles);
									}
									//Somehow, this causes bugging if someone entered the game during pause (they get infinite invulnerability)
									// if (object->client->invulnerableTimer)
										// object->client->invulnerableTimer += pauseDuration;
								}
							}
						}
					}
				}
				#endif

				#ifdef DEBUGFPS
				else if (cv->vmCvar == &g_fps || cv->vmCvar == &g_timescale) {
					void FPS_ResetStats(void);
					FPS_ResetStats();
				}
				#endif

				if (cv->teamShader) {
					remapped = qtrue;
				}
			}
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	trap_SetConfigstring( CS_INTERMISSION, "" );

	#ifdef DEBUGSTUFF
	// DebugStuff("G_UpdateCvars end");
	#endif
}

/*
============
G_InitGame

============
*/

extern const char *gameNames[];

int trap_RealTime( qtime_t *qtime );

void G_InitClientCommands (void);

void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;

	B_InitAlloc(); //make sure everything is clean

	trap_Cvar_Set("gamename", GAMEVERSION) ;

	#if 0
	// This was Daggolin's idea for a workaround for nitrado servers whose startscripts set vm_game to 0 for loading dlls which
	// is good for loading nts server mod but not for any other standard qvm mod
	// nice!

	if (trap_Cvar_VariableIntegerValue("vm_game") != 2) {
		trap_Cvar_Set("vm_game", "2") ;
		trap_Cvar_Set("gamename", GAMEVERSION) ;
		trap_SendConsoleCommand(EXEC_INSERT, "map ctf_yavin\n");
		return;	//?
	}

/*	if (trap_Cvar_VariableIntegerValue("sv_allowDownload") != 0) {
		//security measures when using standard jk2ded
		trap_Cvar_Set("sv_allowDownload", "0") ;
		G_Printf("sv_allowDownload was forced to 0 for security reasons.\n");
	}*/

/* 	{
		char shit[128];

		trap_Cvar_VariableStringBuffer("fs_game", shit, sizeof(shit));

		if (Q_stricmp(shit, "ntxii")) {
			trap_Cvar_Set("fs_game", "ntxii");
			trap_SendConsoleCommand(EXEC_INSERT, "map ctf_yavin\n");
			return;	//?
		}
	} */
	#endif

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", __DATE__);

	srand( randomSeed );

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime

	//trap_SP_RegisterServer("mp_svgame");

	if (g_logSecurity.string[0] && Q_stricmp(g_logSecurity.string, "none") != 0) {
		trap_FS_FOpenFile( g_logSecurity.string, &level.logFileSecurity, FS_APPEND_SYNC );	//always sync in case of crash

		if ( !level.logFileSecurity ) {
			G_Printf( "WARNING: Couldn't open security logfile: \"%s\"\n", g_logSecurity.string );
		} else {
			G_SecurityLogPrint ("---------------- Started logging ----------------", NULL);
		}
	} else {
		G_Printf("Not logging security events.\n");
	}

	//for logging d/bs events
	#ifdef ANALYZE_BS
	trap_FS_FOpenFile( "bsevents.dat", &level.bsLogFile, FS_APPEND_SYNC );
	#endif

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_log.string[0] ) {
		if ( g_logSync.integer )
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_SYNC );
		else
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND );

		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
		} else {
			char	serverinfo[MAX_INFO_STRING];
			qtime_t t;

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf ("------------------- Game Initialization --------------------\n");
			G_LogPrintf ("gamename: %s\n", GAMEVERSION);

			//Log startup time and date.
			trap_RealTime( &t );

			G_LogPrintf("Log started %02d-%02d-%d (day-month-year) @ %02d:%02d:%02d\n", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);
			if (g_gametype.integer >= 0 && g_gametype.integer < GT_MAX_GAME_TYPE)
				G_LogPrintf("gametype: %s (%d)\n", gameNames[ g_gametype.integer ], g_gametype.integer);

			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	} else {
		G_Printf( "Not logging to disk.\n" );
	}

	G_LogWeaponInit();

	G_InitWorldSession();

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// initialize saga mode before spawning entities so we know
	// if we should remove any saga-related entities on spawn
	InitSagaMode();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if( g_gametype.integer >= GT_TEAM ) {
		G_CheckTeamItems();
	}
	else if ( g_gametype.integer == GT_JEDIMASTER )
	{
		trap_SetConfigstring ( CS_CLIENT_JEDIMASTER, "-1" );
	}

	trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("-1|-1") );
	trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("-1") );
	gDuelist1 = -1;
	gDuelist2 = -1;

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	if( g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue( "com_buildScript" ) ) {
		G_ModelIndex( SP_PODIUM_MODEL );
		G_SoundIndex( "sound/player/gurp1.wav" );
		G_SoundIndex( "sound/player/gurp2.wav" );
	}

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}

	G_RemapTeamShaders();

	G_InitClientCommands();
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	G_Printf ("==== ShutdownGame ====\n");

	G_LogWeaponOutput();

	if ( level.logFile ) {
		G_LogPrintf("ShutdownGame...\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
		level.logFile = 0;
	}

	if (level.logFileSecurity) {
		G_SecurityLogPrint ("----------------  Ended logging  ----------------", NULL);
		trap_FS_FCloseFile( level.logFileSecurity );
	}

	#ifdef ANALYZE_BS
	if (level.bsLogFile) {
		trap_FS_FCloseFile( level.bsLogFile );
	}
	#endif

	// write all the client session data so we can get it back
	G_WriteSessionData();

	trap_ROFF_Clean();

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}

	B_CleanupAlloc(); //clean up all allocations made with B_Alloc
}



//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	G_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	G_Printf ("%s", text);
}

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int			i;
	gclient_t	*client;
	gclient_t	*nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
//	if ( level.intermissiontime ) {
//		return;
//	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ||
			client->sess.spectatorClient < 0  ) {
			continue;
		}

		if ( !nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime ) {
			nextInLine = client;
		}
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f" );
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[0];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores( void ) {
	int			clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.wins++;
		ClientUserinfoChanged( clientNum, qfalse );

		trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", clientNum ) );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.losses++;
		ClientUserinfoChanged( clientNum, qfalse );
	}

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t	*ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}


	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( ca->sess.spectatorTime < cb->sess.spectatorTime ) {
			return -1;
		}
		if ( ca->sess.spectatorTime > cb->sess.spectatorTime ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		< cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int		i;
	int		rank;
	int		score;
	int		newScore;
	int		preNumSpec = 0;
	int		nonSpecIndex = -1;
	gclient_t	*cl;

	preNumSpec = level.numNonSpectatorClients;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots
	for ( i = 0; i < TEAM_NUM_TEAMS; i++ ) {
		level.numteamVotingClients[i] = 0;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
				level.numNonSpectatorClients++;
				nonSpecIndex = i;

				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {
					level.numPlayingClients++;
					if ( !(g_entities[i].r.svFlags & SVF_BOT) ) {
						level.numVotingClients++;
						if ( level.clients[i].sess.sessionTeam == TEAM_RED )
							level.numteamVotingClients[0]++;
						else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							level.numteamVotingClients[1]++;
					}
					if ( level.follow1 == -1 ) {
						level.follow1 = i;
					} else if ( level.follow2 == -1 ) {
						level.follow2 = i;
					}
				}
			}
		}
	}

	if (!g_warmup.integer)
	{
		level.warmupTime = 0;
	}


	qsort( level.sortedClients, level.numConnectedClients,
		sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va("%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numConnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else if ( level.numConnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}

		if (g_gametype.integer != GT_TOURNAMENT)
		{ //when not in duel, use this configstring to pass the index of the player currently in first place
			if ( level.numConnectedClients >= 1 )
			{
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, va("%i", level.sortedClients[0] ) );
			}
			else
			{
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );
			}
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( ent );
	}


	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent ) {	// the map creator forgot to put in an intermission point...
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle );
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

qboolean DuelLimitHit(void);

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

		AdjustTournamentScores();
		if (DuelLimitHit())
		{
			gDuelExit = qtrue;
		}
		else
		{
			gDuelExit = qfalse;
		}
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	if (g_singlePlayer.integer) {
		trap_Cvar_Set("ui_singlePlayerActive", "0");
		UpdateTournamentInfo();
	}

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			respawn(client);
		}
		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();
}

qboolean DuelLimitHit(void)
{
	int i;
	gclient_t *cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		if ( g_duel_fraglimit.integer && cl->sess.wins >= g_duel_fraglimit.integer )
		{
			return qtrue;
		}
	}

	return qfalse;
}

void DuelResetWinsLosses(void)
{
	int i;
	gclient_t *cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		cl->sess.wins = 0;
		cl->sess.losses = 0;
	}
}

/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel (void) {
	int		i;
	gclient_t *cl;

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT  ) {
		if (!DuelLimitHit())
		{
			if ( !level.restarted ) {
				trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
				level.restarted = qtrue;
				level.changemap = NULL;
				level.intermissiontime = 0;
			}
			return;
		}

		DuelResetWinsLosses();
	}


	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/

typedef struct {
	//8 bytes. can we make it 4?
	byte tm_sec;     /* seconds after the minute - [0,59] */
	byte tm_min;     /* minutes after the hour - [0,59] */
	byte tm_hour;    /* hours since midnight - [0,23] */
	byte tm_mday;    /* day of the month - [1,31] */
	byte tm_mon;     /* months since January - [0,11] */
	byte tm_year;    /* years since 1900 */		//as of 2015, value is 115, so we still have a few years before we hit 127
	byte tm_wday;	 /* days since Sunday - [0,6] */
	byte tm_isdst;	 /* daylight savings time flag */
} smallTime_t;

void QtimeToSmallTime ( qtime_t *qt, smallTime_t *st ) {
	st->tm_sec = (byte)qt->tm_sec;
	st->tm_min = (byte)qt->tm_min;
	st->tm_hour = (byte)qt->tm_hour;
	st->tm_mday = (byte)qt->tm_mday;
	st->tm_mon = (byte)qt->tm_mon;
	st->tm_year = (byte)qt->tm_year;
	st->tm_wday = (byte)qt->tm_wday;
	st->tm_isdst = (byte)qt->tm_isdst;
}

#ifdef ANALYZE_BS
void QDECL G_LogBsEvent ( bsRecord_t *bsr, gentity_t *ent ) {
	int 			len;
	qtime_t			time;
	smallTime_t 	smalltime;

	if ( !level.bsLogFile ) {
		return;
	}

	trap_RealTime( &time );
	QtimeToSmallTime( &time, &smalltime );

	//write player IP in 4 bytes
	trap_FS_Write( &ent->client->sess.ipb, sizeof(ent->client->sess.ipb), level.bsLogFile );

	//write player name; use len+1 so we write the 0 byte also
	len = strlen( ent->client->pers.netnameClean );
	trap_FS_Write( ent->client->pers.netnameClean, len + 1, level.bsLogFile );

	//write timestamp
	trap_FS_Write ( &smalltime, sizeof(smallTime_t), level.bsLogFile );

	//write the BS record data
	trap_FS_Write( bsr, sizeof(bsRecord_t), level.bsLogFile );
}
#endif


void QDECL G_SecurityLogPrint ( const char *event, gentity_t *ent ) {
	char		timeStr[16] = {0}, dateStr[16] = {0};
	char		extra[128] = {0};
	char		string[1024];
	qtime_t		t;

	if (ent && ent->client) {
		const char *name = ent->client->pers.netnameClean;

		Com_sprintf(extra, sizeof(extra), "      %s  (%02d) \"%s\"",
			(ent->r.svFlags & SVF_BOT) ? "[BOT]" : ent->client->sess.ip,
			ent - g_entities,
			(*name) ? name : "<N/A>??"
			);
	}

	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", event, extra );
	}

	if ( !level.logFileSecurity ) {
		return;
	}

	trap_RealTime( &t );

	Com_sprintf(dateStr, sizeof(dateStr), "%02d-%02d-%d", t.tm_mday, t.tm_mon+1, t.tm_year+1900);

	G_ClockString( timeStr, sizeof(timeStr) );

	Com_sprintf(string, sizeof(string), "%s %s  %s%s\n", dateStr, timeStr, event, extra);

	trap_FS_Write( string, strlen( string ), level.logFileSecurity );
}

void G_ClockString( char *buf, int bufsize )
{
	qtime_t t;

	trap_RealTime( &t );
	Com_sprintf( buf, bufsize, "[%02d:%02d:%02d] ", t.tm_hour, t.tm_min, t.tm_sec );
}

void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[2048];
	int len;

	G_ClockString(string, sizeof(string));

	len = strlen(string);

	va_start( argptr, fmt );
	vsprintf( string + len, fmt, argptr );
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + len );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;
	qboolean		won = qtrue;
	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
			level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	G_LogPrintf( "game duration: %s\n", G_MsToString(level.time - level.startTime ) );

	for (i=0 ; i < numSorted ; i++) {
		int		ping;
		int		time;
		int team;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		team = cl->sess.sessionTeam;

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		time = (level.time - cl->pers.enterTime)/60000;

		G_LogPrintf( "score: %4d  ping: %3d  time: %3d   %s   (%02d) %s\n",
			cl->ps.persistant[PERS_SCORE], ping, time,
			FixedLengthSpace( (team == TEAM_RED ? "red" : (team == TEAM_BLUE ? "blue" : "")), 4 ),
			level.sortedClients[i], cl->pers.netnameClean );

		if (g_singlePlayer.integer && g_gametype.integer == GT_TOURNAMENT) {
			if (g_entities[cl - level.clients].r.svFlags & SVF_BOT && cl->ps.persistant[PERS_RANK] == 0) {
				won = qfalse;
			}
		}
	}
}

qboolean gDidDuelStuff = qfalse; //gets reset on game reinit

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/

void CheckIntermissionExit( void ) {
	int			ready, notReady;
	int			i;
	gclient_t	*cl;
	int			readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}

		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	if ( g_gametype.integer == GT_TOURNAMENT && !gDidDuelStuff &&
		(level.time > level.intermissiontime + 2000) )
	{
		gDidDuelStuff = qtrue;

		// if we are running a tournement map, kick the loser to spectator status,
		// which will automatically grab the next spectator and restart
		if (!DuelLimitHit())
		{
			RemoveTournamentLoser();

			AddTournamentPlayer();

			if (level.numPlayingClients >= 2)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
			}

			return;
		}

		//this means we hit the duel limit so reset the wins/losses
		//but still push the loser to the back of the line, and retain the order for
		//the map change
		RemoveTournamentLoser();
		AddTournamentPlayer();
		if (level.numPlayingClients >= 2)
		{
			trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
			trap_SetConfigstring ( CS_CLIENT_DUELWINNER, "-1" );

			gDuelist1 = level.sortedClients[0];
			gDuelist2 = level.sortedClients[1];
		}
	}

	if (g_gametype.integer == GT_TOURNAMENT && !gDuelExit)
	{ //in duel, we have different behaviour for between-round intermissions
		if ( level.time > level.intermissiontime + 4000 )
		{ //automatically go to next after 4 seconds
			ExitLevel();
			return;
		}

		for (i=0 ; i< g_maxclients.integer ; i++)
		{ //being in a "ready" state is not necessary here, so clear it for everyone
		  //yes, I also thinking holding this in a ps value uniquely for each player
		  //is bad and wrong, but it wasn't my idea.
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED )
			{
				continue;
			}
			cl->ps.stats[STAT_CLIENTS_READY] = 0;
		}
		return;
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	{

		// never exit in less than five seconds
		if ( level.time < level.intermissiontime + 5000 ) {
			return;
		}

		// if nobody wants to go, clear timer
		if ( !ready ) {
			level.readyToExit = qfalse;
			return;
		}

		// if everyone wants to go, go now
		if ( !notReady ) {
			ExitLevel();
			return;
		}

		// the first person to ready starts the ten second timeout
		if ( !level.readyToExit ) {
			level.readyToExit = qtrue;
			level.exitTime = level.time;
		}

		// if we have waited ten seconds since at least one player
		// wanted to exit, go ahead
		if ( level.time < level.exitTime + 10000 ) {
			return;
		}

		ExitLevel();

	}
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int		a, b;

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
 	int			i;
	gclient_t	*cl;
	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit ();
		return;
	}

	if ( level.intermissionQueued ) {
		int time = (g_singlePlayer.integer) ? SP_INTERMISSION_DELAY_TIME : INTERMISSION_DELAY_TIME;
		if ( level.time - level.intermissionQueued >= time ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}

	// check for sudden death
	if ( ScoreIsTied() ) {
		// always wait for sudden death
		return;
	}

	if ( g_timelimit.integer && !level.warmupTime && !g_pauseGame.integer ) {
		if ( level.time - level.startTime >= g_timelimit.integer*60000 ) {
//			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"");
			trap_SendServerCommand( -1, va("print \"%s.\n\"",G_GetStripEdString("SVINGAME", "TIMELIMIT_HIT")));
			LogExit( "Timelimit hit." );
			return;
		}
	}

	if ( level.numPlayingClients < 2 ) {
		return;
	}

	if ( g_gametype.integer < GT_CTF && g_fraglimit.integer ) {
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"Red %s\n\"", G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")) );
			LogExit( "Kill limit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"Blue %s\n\"", G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")) );
			LogExit( "Kill limit hit." );
			return;
		}

		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE ) {
				continue;
			}

			if ( g_gametype.integer == GT_TOURNAMENT && g_duel_fraglimit.integer && cl->sess.wins >= g_duel_fraglimit.integer )
			{
				LogExit( "Duel limit hit." );
				gDuelExit = qtrue;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the win limit.\n\"",
					cl->pers.netname ) );
				return;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) {
				LogExit( "Kill limit hit." );
				gDuelExit = qfalse;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s.\n\"",
												cl->pers.netname,
												G_GetStripEdString("SVINGAME", "HIT_THE_KILL_LIMIT")
												)
										);
				return;
			}
		}
	}

	if ( g_gametype.integer >= GT_CTF && g_capturelimit.integer ) {

		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/

/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournament( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( level.numPlayingClients == 0 ) {
		return;
	}

	if ( g_gametype.integer == GT_TOURNAMENT ) {

		// pull in a spectator if needed
		if ( level.numPlayingClients < 2 ) {
			AddTournamentPlayer();

			if (level.numPlayingClients >= 2)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
			}
		}

		if (level.numPlayingClients >= 2)
		{
			if (gDuelist1 == -1 ||
				gDuelist2 == -1)
			{
				trap_SetConfigstring ( CS_CLIENT_DUELISTS, va("%i|%i", level.sortedClients[0], level.sortedClients[1] ) );
				gDuelist1 = level.sortedClients[0];
				gDuelist2 = level.sortedClients[1];
			}
		}

		//rww - It seems we have decided there will be no warmup in duel.
		//if (!g_warmup.integer)
		{ //don't care about any of this stuff then, just add people and leave me alone
			level.warmupTime = 0;
			return;
		}

	} else if ( g_gametype.integer != GT_SINGLE_PLAYER && level.warmupTime != 0 ) {
		int		counts[TEAM_NUM_TEAMS];
		qboolean	notEnough = qfalse;

		if ( g_gametype.integer > GT_TEAM ) {
			counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( -1, TEAM_RED );

			if (counts[TEAM_RED] < 1 || counts[TEAM_BLUE] < 1) {
				notEnough = qtrue;
			}
		} else if ( level.numPlayingClients < 2 ) {
			notEnough = qtrue;
		}

		if ( notEnough ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return; // still waiting for team members
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( !level.voteTime ) {
		//No vote in progress.
		return;
	}
	if ( level.time - level.voteTime >= VOTE_TIME 	//Vote time (30 secs) passed
			/*|| level.voteYes + level.voteNo == level.numVotingClients	//all who can vote have voted */
	   ) {
		//vote time is over

		const int total = level.voteYes + level.voteNo;

		if (!total) {
			G_SendClientPrint( -1, "No votes were cast for '%s^7'\n", level.voteDisplayString );
		} else {
			float pctYes, pctNo;

			pctYes = (float)level.voteYes / (float)total * 100.f;
			pctNo  = (float)level.voteNo  / (float)total * 100.f;

			// G_SendClientPrint( -1, "^5Vote ended:\n" );
			G_SendClientPrint( -1, "Vote ended with ^3%d^7 cast votes for: '%s^7'\n", total, level.voteDisplayString );
			G_SendClientPrint( -1, "Yes: %3d (^3%.1f^7 percent)\n", level.voteYes, pctYes );
			G_SendClientPrint( -1, "No:  %3d (^3%.1f^7 percent)\n", level.voteNo, pctNo );
		}

		level.voteTime = 0;
		trap_SetConfigstring( CS_VOTE_TIME, "" );
	}

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand( i, message );
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client) {
	int i;

	if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
		return;
	}
	if (level.clients[client].sess.sessionTeam != team) {
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
		return;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader) {
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i, qfalse);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged( client, qfalse );
	// PrintTeam(team, va("print \"%s %s\n\"", level.clients[client].pers.netname, G_GetStripEdString("SVINGAME", "NEWTEAMLEADER")) );
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader( int team ) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader)
			break;
	}
	if (i >= level.maxclients) {
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (!(g_entities[i].r.svFlags & SVF_BOT)) {
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			level.clients[i].sess.teamLeader = qtrue;
			break;
		}
	}
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team ) {
}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}


static void G_CheckUnpause (void)
{
	static int lastPrint = -1;
	int secs;

	//if game is not paused, an admin unpaused, so just cancel.
	if (!g_pauseGame.integer) {
		level.unpauseTime = 0;
		return;
	}

	secs = level.unpauseTime - level.time;		// how many ms until pause?
	secs /= 1000;	//how many seconds left until game will unpause?
	secs += 1;		//so it prints 3,2,1 instead of 2,1,0

	if ( secs != lastPrint ) {
		G_SendClientCenterPrint( -1, "Unpause in ^5%d ^7secs\n", secs );
		lastPrint = secs;
	}

	if (level.time >= level.unpauseTime) {
		//time to unpause.
		trap_SendConsoleCommand( EXEC_APPEND, PAUSEGAME_CVARNAME" 0\n");

		//clear the "unpause in 1 sec" message.
		G_SendClientCenterPrint( -1, " " );

		level.unpauseTime = 0;
		level.unpauseClient = -2;
		lastPrint = -1;
	}


}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
	float	thinktime;

	if (g_pauseGame.integer)
		return;

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}

	ent->nextthink = 0;
	if (!ent->think) {
		G_Error ( "NULL ent->think");
	}
	ent->think (ent);
}

int g_LastFrameTime = 0;
int g_TimeSinceLastFrame = 0;

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/

void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	#ifdef DEBUGFPS
	#define	FPS_FRAMES	16	//orig
	{
		static int	previousTimes[FPS_FRAMES];
		static int	index = 0, previous = 0;
		int			i, total, fps;
		int			t, frameTime;

		t = trap_Milliseconds();
		frameTime = t - previous;
		previous = t;

		// frameTime = cg.realframetime;	 HAX!
		previousTimes[index % FPS_FRAMES] = frameTime;
		if ( ++index > FPS_FRAMES ) {
			// average multiple frames together to smooth changes out a bit
			total = 0;
			for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
				total += previousTimes[i];
			}
			if ( !total ) {
				total = 1;
			}
			fps = 1000 * FPS_FRAMES / total;

			level.avgfps = fps;

			level.fpsSamples++;
			level.fpsFrameTime += frameTime;
		}
	}
	#endif

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;

	g_TimeSinceLastFrame = (level.time - g_LastFrameTime);

	// get any cvar changes
	G_UpdateCvars();


	//
	// go through all allocated objects
	//
	// start = trap_Milliseconds();
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) {
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				if (ent->s.eFlags & EF_SOUNDTRACKER)
				{ //don't trigger the event again..
					ent->s.event = 0;
					ent->s.eventParm = 0;
					ent->s.eType = 0;
					ent->eventTime = 0;
				}
				else
				{
					G_FreeEntity( ent );
					continue;
				}
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_CheckClientTimeouts ( ent );

			if ( ent->client->pers.userinfoRequestTime &&
				level.time >= ent->client->pers.userinfoRequestTime
			) {

				ent->client->pers.userinfoChangeTime = 0;
				ClientUserinfoChanged( ent - g_entities, qtrue );
				ent->client->pers.userinfoRequestTime = 0;
			}


			if((!level.intermissiontime)&&!(ent->client->ps.pm_flags&PMF_FOLLOW))
			{
				WP_ForcePowersUpdate(ent, &ent->client->pers.cmd );
				WP_SaberPositionUpdate(ent, &ent->client->pers.cmd);
			}
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}
// end = trap_Milliseconds();

	trap_ROFF_UpdateEntities();

// start = trap_Milliseconds();
	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}
// end = trap_Milliseconds();

	// see if it is time to do a tournement restart
	CheckTournament();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	if (level.unpauseTime) {
		// an unpause is pending.
		G_CheckUnpause();
	}

	// check team votes
	CheckTeamVote( TEAM_RED );
	CheckTeamVote( TEAM_BLUE );

	// for tracking changes
	CheckCvars();

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}

	//At the end of the frame, send out the ghoul2 kill queue, if there is one
	G_SendG2KillQueue();

	g_LastFrameTime = level.time;
}

const char *G_GetStripEdString(char *refSection, char *refName)
{
	/*
	static char text[1024]={0};
	trap_SP_GetStringTextString(va("%s_%s", refSection, refName), text, sizeof(text));
	return text;
	*/

	//Well, it would've been lovely doing it the above way, but it would mean mixing
	//languages for the client depending on what the server is. So we'll mark this as
	//a striped reference with @@@ and send the refname to the client, and when it goes
	//to print it will get scanned for the striped reference indication and dealt with
	//properly.
	static char text[1024]={0};
	Com_sprintf(text, sizeof(text), "@@@%s", refName);
	return text;
}

