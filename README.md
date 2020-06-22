This is the repository for the Link management block for both the android c server and the video c server  
The repository for the android app can be found at [https://github.com/koldbrandt/AppForFagprojekt](https://github.com/koldbrandt/AppForFagprojekt)

# Video Server

The video server can be compiled using the makefile that is provided in the `/VideoServer` folder.  
`video` is the name of the compiled video server program. This program can be found in the `/VideoServer/build` folder after running the makefile.

The video server/client has multiple options available as command line arguments. For a full list simply run `video` or `video -h`. The most important options are listed below

- `-c` : run as client
- `-s` : run as server 
- `-p [port]` : choose which port to use. (server and client) [Default: 1337]  
- `-ip [ip address]`: choose which ip to connect to. (client only) [Default: localhost]   
Either -s or -c needs to be specified as the first argument for the program to run.

When testing the system, the following commands can be used, assuming the setup is  
Client 1 -> Server 1 -> FIFO -> Client 2 -> Server 2

### Iperf testing
Client 1: `iperf -c [server 1 ip] -u -l [max packet size] -b 1m`  
Server 1: `video -s -p 5001 -iperf`  
Client 2: `video -c -d -ip [server 2 ip] -p 5001`  
Server 2: `iperf -s -u -t 10`  

Here server 1 and client 2 should be run on a DE1-SoC board.  

For client 2, an option menu will be printed. Simply chose option 4 ("run the video send loop"). This is just to skip the initial handshake when testing with iperf.  

It is important that client 1 is started last, as it will start sending immediately. Also note that server 2 will time out after 10 seconds of no connections and will have to be restarted if this happens.  

[max packet size] indicates the size of the packets sent by the iperf program in bytes. On the video server, packets of up to 2000 bytes are supported.  

For client 1, the `-b 1m` tells iperf to send at 1 mbit/s. Any value can be used here.

### Video block integration
server 1 - `video -s -p 5001`  
client 2 - `video -c -ip [server 2 ip] -p 5001`  

Here it is assumed that client 1 and server 2 are provided by the video block.  

# Android Server

The android server can be compiled with the makefile provided in the `/androidServer` folder.  
The compiled program will be put into the `/androidServer/build` folder and is named `androidServer`  
The program can then be run using the command `androidServer -p [port]`