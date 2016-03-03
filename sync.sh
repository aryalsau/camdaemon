#!/bin/bash
uncomitted=$(git status -s)
# some arguments don't have a corresponding value to go with it such as in the --default example.
for i in "$@"
do
case $i in

	-ip=*|--client=*)
		ip="${i#*=}"
		shift # past argument=value
	;;

	-u=*|--user=*)
		user="${i#*=}"
		shift # past argument=value
	;;

	-c=*|--camera=*)
		camera="${i#*=}"
		shift # past argument=value
	;;

	-f|--force) # force push even if uncomitted changes available
		force=true
		shift # past argument=value
	;;

	--default)
		DEFAULT=YES
		shift # past argument with no value
	;;

	*)
		# unknown option
	;;

esac
done

if [[ -z "$uncomitted" ]] || [ "$force" = true ] ; then
	echo 'pushing git revision '$(git rev-parse HEAD)' to server'

	if [ -z "$ip" ] || [ -z "$camera" ]; then
		echo 'Requires both ip and camera'
	else
		echo 'rsyncing files to '"$user"@"$ip"
		rsync --exclude='data/*' --exclude='LICENSE' --exclude='README.md'  --exclude='camdaemon' --exclude='sync.sh' -a -v * "$user"@"$ip":~/camdaemon
		echo 'rsyncing complete'
		echo 'compiling camdaemon for '"$camera"
		ssh "$user"@"$ip" 'make '$camera' -C ~/camdaemon'
	fi

else
	echo 'uncomitted changes available'
	echo "${uncomitted}"
	echo 'commit changes before pushing to the server'
	exit
fi
