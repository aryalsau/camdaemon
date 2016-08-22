#!/bin/bash

# to push code only from macbook to vmware
# sh sync.sh --server=192.168.205.140 --user=root --location=/opt --code --make=nullcam,nullcompass,camdaemon --force

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

		-m=*|--make=*) # remotely build for a certain make
			make="${i#*=}"
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
		echo rsyncing data from "$user"@"$ip":"$location"/camdaemon to the current directory
		rsync -a -v "$user"@"$ip":"$location"/camdaemon/data .
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
			echo rsyncing all files to "$user"@"$ip":"$location"/camdaemon
			rsync -a -v * "$user"@"$ip":"$location"/camdaemon
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
			echo rsyncing just code to "$user"@"$ip":"$location"/camdaemon
			rsync --exclude='.git' --exclude='data/*' --exclude='.gitignore' --exclude='LICENSE' --exclude='README.md' --exclude='sync.sh' --exclude='camdaemon' -a -v * "$user"@"$ip":"$location"/camdaemon
			echo rsync complete
			echo generating daemon.sh
			deamonsh='#!/bin/bash\ncd '"$location"'/camdaemon; ./camdaemon -v -p 3000'
			echo "$deamonsh" | ssh "$user"@"$ip" "cat > "$location"/camdaemon/daemon.sh"
			echo make daemon.sh executable
			ssh "$user"@"$ip" "chmod +x "$location"/camdaemon/daemon.sh"
		fi
	else
		echo 'uncomitted changes available'
		echo "${uncomitted}"
		echo 'commit changes before pushing to the server'
		exit
	fi
fi


if [ ! -z "$make" ]; then
	recipes=$(echo "$make" | tr "," "\n")
	for recipe in "$recipes"
	do
		echo compiling "$recipes" on "$user"@"$ip":"$location"/camdaemon
		ssh "$user"@"$ip" "make "$recipe" -C " "$location"/camdaemon
	done
fi


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
	echo '-m or --make=xxx (Build on the destination computer for xxx device)'
	echo '-f or --force (Force a sync even if uncomitted changes are available)'
	echo '-d or --data (Sync data folder)'
fi
