# camdaemon
Socket server to control different types of cameras for hit&amp;mis

Run by `./camdaemon -p 3000 -v` to start the server on port 3000

* -p for port (default 8000)

* -v for verbose

Edit daemon.sh and `chmod +x daemon.sh`

see `config.cfg` file for configuration

    CAMDAEMON_SITE=LAB00
    CAMDAEMON_CAM=PIXIS
    CAMDAEMON_PATH=/home/ikon/camdaemon/data/

* CAMDAEMON_SITE : Five character word to be stored in the 'site' location of the header
* CAMDAEMON_CAM : Five character word to be stored in the 'camera' location of the header
* CAMDAEMON_PATH : Location to write files to.
