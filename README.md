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
