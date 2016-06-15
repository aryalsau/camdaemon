#!/bin/bash
uncomitted=$(git status -s)
# some arguments don't have a corresponding value to go with it such as in the --default example.
# ./sync.sh --server=192.168.1.2 --user=ikon --camera==pixis --force --time --code
for i in "$@"
do
case $i in

	-ip=*|--server=*) # the ip of the clent
		ip="${i#*=}"
		shift # pass argument=value
	;;

	-u=*|--user=*) # the user on the client to run camdaemon
		user="${i#*=}"
		shift # pass argument=value
	;;

	-m=*|--camera=*) # camera type
		camera="${i#*=}"
		shift # pass argument=value
	;;

	-c|--code) # send code
		code=true
		shift # pass argument=value
	;;

	-f|--force) # force push even if uncomitted changes available
		force=true
		shift # pass argument=value
	;;

	-t|--time) # force push even if uncomitted changes available
		time=true
		shift # pass argument=value
	;;

	--default)
		DEFAULT=YES
		shift # pass argument with no value
	;;

	*)
		# unknown option
	;;

esac
done

if [ "$time" = true ]; then

	if [ -z "$ip" ] || [ -z "$user" ]; then
		echo 'Requires both ip and user'
	else
		echo setting server time to client time
		# Sun Mar  6 01:34:35 CET 2016
		clienttime="$(date +'"%d %b %Y %T"')"
		ssh "$user"@"$ip" "sudo date --set=$clienttime"
	fi

fi

if [ "$code" = true ]; then

	if [[ -z "$uncomitted" ]] || [ "$force" = true ]; then

		echo pushing git revision "$(git rev-parse HEAD)" to server

		if [ -z "$ip" ] || [ -z "$user" ]; then
			echo 'Requires both ip and user'
		else
			echo rsyncing files to "$user"@"$ip"
			rsync --exclude='data/*' --exclude='LICENSE' --exclude='README.md'  --exclude='camdaemon' --exclude='sync.sh' --exclude='daemon.sh' -a -v * "$user"@"$ip":/home/"$user"/camdaemon
			echo rsyncing complete
			echo compiling camdaemon for "$camera" on "$user"@"$ip"
			ssh "$user"@"$ip" "make "$camera" -C /home/"$user"/camdaemon"

			daemonsh='#!/bin/bash
/bin/su - '"$user"' -c "cd /home/'"$user"'/camdaemon; ./camdaemon -p 3000"'

			echo "$daemonsh" | ssh "$user"@"$ip" "cat > /home/"$user"/camdaemon/daemon.sh"

			ssh "$user"@"$ip" "chmod +x /home/"$user"/camdaemon/daemon.sh"

		fi

		exit

	else

		echo 'uncomitted changes available'
		echo "${uncomitted}"
		echo 'commit changes before pushing to the server'
		exit

	fi


fi
