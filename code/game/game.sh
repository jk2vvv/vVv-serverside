# Linux makefile for game module
mkdir -p vm
cd vm
rm ./*.asm

echo -e "\nStarting q3lcc to generate bytecode.."
LCC="../../../bin/q3lcc -DQ3_VM -DMISSIONPACK -DCGAME -S -Wf-target=bytecode"

# source files
FILES="g_main.c g_syscalls.c bg_misc.c bg_lib.c bg_pmove.c bg_saber.c bg_slidemove.c bg_panimate.c bg_weapons.c
	q_math.c q_shared.c ai_main.c ai_util.c ai_wpnav.c g_active.c g_arenas.c g_bot.c g_client.c
	g_cmds.c g_combat.c g_items.c g_log.c g_mem.c g_misc.c g_missile.c g_mover.c g_object.c
	g_saga.c g_session.c g_spawn.c g_svcmds.c g_target.c g_team.c g_trigger.c g_utils.c g_weapon.c
	w_force.c w_saber.c"

# compile one file at a time to prevent buggy behaviour
for file in $FILES; do
	$LCC "../$file" || GOT_ERROR=1
done

if [ -n "$GOT_ERROR" ]; then
	echo "q3lcc returned error."
	exit 1
fi

echo "q3lcc done."
echo "Starting q3asm..."

../../../bin/q3asm -vq3 -v -f "../game"
rc=$?
echo

if [ "$rc" -ne 0 ]; then
	echo "!! q3asm returned errorcode $rc."
fi

# Convenience: delete asm files, no use
rm ./*.asm

