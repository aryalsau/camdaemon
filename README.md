# camdaemon
Socket server to control different types of cameras for hit&amp;mis

Run by `./camdaemon -p 3000 -v` to start the server on port 3000

The flags are

* -p for port (default 8000)
* -v for verbose

Edit daemon.sh and `chmod +x daemon.sh` append the following in `etc/inittab` (with the correct location) to have the daemon respawn if interrupted

	h1:12345:respawn:/home/'user'/camdaemon/daemon.sh

see `config.cfg` file for configuration

		site=Umass LOCSST lab
		camera=Princeton Instruments PIXIS 1024
		path=data

The fields are

* site : Site where the instrument is located
* camera : Camera used in the instrument
* path : Folder Location to write files to (does not require a trailing slash / if going in to the current folder)


## Server deployment
Use `sync.sh` to deploy, example `./sync.sh --client=192.168.1.2 --user=ikon --camera==pixis --time`. Make sure all changes are committed before syncing.

The parameters are for `sync.sh`

* -ip or --server : ip of the server
* -u or --user : User on the
* -c or --camera : camera type
* -f or --force : flag to force an upload with uncommitted changes
* -t or --time : sync server time to the dev computer time
