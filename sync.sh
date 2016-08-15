#!/bin/bash

# to push code only from eos desktop to virtmanger
# sh sync.sh --server=192.168.205.139 --user=kuravih --location=/home/kuravih/camdaemon --code --camera=pixis --force
















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

	-l=*|--location=*) # the location for the files in user home on the science camera computer
		location="${i#*=}"
		shift # pass argument=value
	;;

	-a|--all) # send everything
		all=true
		shift # pass argument=value
	;;

	-c|--code) # send only code
		code=true
		shift # pass argument=value
	;;

	-m=*|--camera=*) # remotely build for a certain camera
		camera="${i#*=}"
		shift # pass argument=value
	;;

	-f|--force) # force push even if uncomitted changes available
		force=true
		shift # pass argument=value
	;;

	-d|--data) # sync only data
		data=true
		shift # pass argument=value
	;;

	-t|--time) # force push even if uncomitted changes available
		time=true
		shift # pass argument=value
	;;

	-h|--help) # show help
		help=true
		shift # pass argument=value
	;;

	--default)
		DEFAULT=YES
		shift # pass argument with no value
	;;

	*)
		# unknown option
		echo 'Unknown option'
	;;

esac
done


if [ "$data" = true ]; then
	if [ -z "$ip" ] || [ -z "$user" ] || [ -z "$location" ]; then
		echo 'Requires ip, user and location'
		exit
	else
		echo rsyncing data from "$user"@"$ip":"$location" to the current directory
		rsync -a -v "$user"@"$ip":"$location"/data .
		exit
	fi
fi


if [ "$all" = true ]; then
	uncomitted=$(git status -s)
	if [[ -z "$uncomitted" ]] || [ "$force" = true ]; then
		echo pushing revision "$(git rev-parse HEAD)" all to computer
		if [ -z "$ip" ] || [ -z "$user" ] || [ -z "$location" ]; then
			echo 'Requires ip, user and location'
			exit
		else
			echo rsyncing all files to "$user"@"$ip":"$location"
			rsync -a -v * "$user"@"$ip":"$location"
			echo rsync complete
			exit
		fi
	else
		echo 'uncomitted changes available'
		echo "${uncomitted}"
		echo 'commit changes before pushing to the server'
		exit
	fi
fi


if [ "$code" = true ]; then
	uncomitted=$(git status -s)
	if [ -z "$uncomitted" ] || [ "$force" = true ]; then
		echo pushing revision "$(git rev-parse HEAD)" code to computer
		if [ -z "$ip" ] || [ -z "$user" ] || [ -z "$location" ]; then
			echo 'Requires ip, user and location'
			exit
		else
			echo rsyncing just code to "$user"@"$ip":"$location"
			rsync --exclude='.git' --exclude='data/*' --exclude='reference' --exclude='.gitignore' --exclude='LICENSE' --exclude='README.md' --exclude='sync.sh' --exclude='camdaemon' --exclude='daemon.sh' -a -v * "$user"@"$ip":"$location"
			echo rsync complete
		fi
	else
		echo 'uncomitted changes available'
		echo "${uncomitted}"
		echo 'commit changes before pushing to the server'
		exit
	fi
fi


case "$camera" in
	clean)
		if [ -z "$ip" ] || [ -z "$user" ] || [ -z "$location" ]; then
			echo 'Requires ip, user and location'
		else
			echo compiling camdaemon for "$camera" on "$user"@"$ip"
			ssh "$user"@"$ip" "make clean -C " "$location"
		fi
	;;

	*)
		if [ -z "$ip" ] || [ -z "$user" ] || [ -z "$location" ]; then
			echo 'Requires ip, user and location'
		else
			echo compiling camdaemon for "$camera" on "$user"@"$ip"
			ssh "$user"@"$ip" "make "$camera" -C " "$location"

# 			daemonsh='#!/bin/bash
# /bin/su - '"$user"' -c "cd '"$location"'; ./camdaemon -p 3000"'
# 			echo wrting daemon.sh
# 			echo "$daemonsh" | ssh "$user"@"$ip" "cat > "$location"/daemon.sh"
# 			echo configuring daemon.sh
# 			ssh "$user"@"$ip" "chmod +x "$location"/daemon.sh"

		fi
	;;

esac


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


if [ "$help" = true ]; then
	echo '-h or --help (This help screen)'
	echo '-ip=x.x.x.x or --server=x.x.x.x (Destination computer ip)'
	echo '-u=someuser or --user=someuser (User name on the destination computer)'
	echo '-l=/somelocation or --location=/somelocation (Absoulte directory on the destination computer)'
	echo '-a or --all (Sync all files)'
	echo '-c or --code (Sync only code)'
	echo '-m or --camera=xxx (Build on the destination computer for a xxx camera)'
	echo '-f or --force (Force a sync even if uncomitted changes are available)'
	echo '-d or --data (Sync data folder)'
fi
