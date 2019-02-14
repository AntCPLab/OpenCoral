
gdb_screen()
{
    prog=$1
    shift
    IFS=
    name=${*/-/}
    IFS=' '
    screen -S :$name -d -m bash -l -c "echo $*; echo $LIBRARY_PATH; gdb $prog -ex \"run $*\""
}

run_player() {
    port=$((RANDOM%10000+10000))
    bin=$1
    shift
    if ! test -e $SPDZROOT/logs; then
        mkdir $SPDZROOT/logs
    fi
    if [[ $bin = Player-Online.x || $bin =~ 'party.x' ]]; then
	params="$* -pn $port -h localhost"
	if [[ ! $bin =~ 'rep' ]]; then
	    params="$params -N $players"
	fi
    else
	params="$port localhost $*"
    fi
    if test $bin = Player-KeyGen.x -a ! -e Player-Data/Params-Data; then
	./Setup.x $players $size 40
    fi
    if [[ $bin =~ Player- && ! $bin =~ Player-Online.x ]]; then
	>&2 echo Running $SPDZROOT/Server.x $players $port
	$SPDZROOT/Server.x $players $port &
    fi
    rem=$(($players - 2))
    for i in $(seq 0 $rem); do
      echo "trying with player $i"
      >&2 echo Running $prefix $SPDZROOT/$bin $i $params
      log=$SPDZROOT/logs/$i
      $prefix $SPDZROOT/$bin $i $params 2>&1 |
	  { if test $i = 0; then tee $log; else cat > $log; fi; } &
    done
    last_player=$(($players - 1))
    >&2 echo Running $prefix $SPDZROOT/$bin $last_player $params
    $prefix $SPDZROOT/$bin $last_player $params > $SPDZROOT/logs/$last_player 2>&1 || return 1
}

sleep 0.5

#mkdir /dev/shm/Player-Data

players=${PLAYERS:-2}

SPDZROOT=${SPDZROOT:-.}

#. Scripts/setup.sh

mkdir logs 2> /dev/null
