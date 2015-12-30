# camdaemon
Socket server to control different types of cameras for hit&amp;mis

Run by `./camdaemon -p 3000 -v` to start the server on port 3000

* -p for port (default 8000)

* -v for verbose

Edit daemon.sh and `chmod +x daemon.sh`
append the following in `etc/inittab` to have the program respawn if interrupted
`h1:12345:respawn:/home/ikon/camdaemon/daemon.sh`

see `config.cfg` file for configuration

		SITE=Umass LOCSST lab
		CAMERA=Princeton Instruments PIXIS 1024
		PATH=data

* SITE : Site where the instrument is located
* CAMERA : Camera used in the instrument
* PATH : Folder Location to write files to (does not require a trailing slash / if going in to the current folder)
