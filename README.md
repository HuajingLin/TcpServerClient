# TcpServerClient
 TCP client for retrieving files, a TCP server for serving the requested files.
<br><br>
The client, after establishing a connection with the server, sends over the name of the file it requests. Then, the server searches for the file in its memory cache followed by a predefined directory. If the file is found in either of the places, the server then transmits the content of file back to the client via the same connection, and caches the file content, if it is not inside the memory cache.
<br><br>
The size limitation of the memory cache is 64MB. If the file size is more than 64MB on disk, you do not cache it in memory. And if the cache size will exceed 64MB after putting the file into memory, you have to purge enough cached content to make sure the entire cache size is restricted to 64MB anytime.
<br><br>
Server usage:<br>
tcp_server port_to_listen_on file_directory<br>
$> ./tcp_server 9089 /home/dist/homework1<br>
Client 127.0.0.1 is requesting 1.pdf<br>
Cache miss. File 1.pdf sent to the client<br>
Added file 1.pdf to cache.(used: 4154k)
<br><br>
Client usage:<br>
tcp_client server_host server_port file_name directory<br>
$> ./tcp_client 127.0.0.1 9089 homework1.pdf .<br>
1.pdf saved
<br><br>
# 1. Overview
## (1)Server side:
### a. The Tcp Server must input two arguments: port and directory of files.
        For example:  ./tcp_server 9089 /Users/huajinglin/Downloads


### b. The server get the port number and string of directory path.
     c. The server creates a socket, binds it, and listens to a predefined port. After that, it blocks until a client connects.


### d. open the Specified directory, if can not open, output an error message and close server.


### e. create the cache for files and initialize it to zero.


### f. A loop:
	accept client to connect.
	create a parameter struct for thread when a client connects.
	save some needed information in the parameter.
	create a thread and pass the pointer of parameter into the thread.
	waits the client thread to end.
	if client requires closing, quit this loop.


### g. In the serving client thread, the server will parse the request and figures out which file the client is asking for. It prints a line "Client xxx.xxx.xxx.xxx is requesting file X", where xxx.xxx.xxx.xxx is the client IP address and X is the file name being requested.

If the file is in the cache, it returns the file directly and outputs "Cache hit. File X sent to the client." If not, it will find the file in the predefined directory and save it in the cache, and then return it to the client and output "Cache miss. File X sent to the client", where X is the file name.

If the file does not exist at all, it will return an error message. It prints a line "File X does not exist", where X is the file name. Note that the server only looks up the file in the given directory, not other directories.


### h. clean data
        Close server socket, free cache memory, and close directory.

## (2)Client side:

### a. he Tcp client must input four arguments: ip, port, file name, and directory of saving files.
        For example:  ./tcp_client 127.0.0.1 9089 1.pdf .


### b. The client gets four parameters, creates a socket for connecting server.

### c. Send file name to server after connecting.

### d. When the response comes, the client either saves it as a local file, and prints a line "File X saved", where X is the local filename, or it will output an error message if the file does not exist on the server. In such case, it prints a line "File X does not exist in the server".


# 2. The details of cache implementation
The cache in memory looks like this:

//------------------------------------------------------<br>
| file__ | file__ || file__ | file__ ||  ... ... | file__ | file__  <br>
| head | data || head | data ||  ... ... | head | data  <br>
//------------------------------------------------------<br>

## (1)each file has a head and data area.
### a. The head of file is a data struct:
     struct FILE_NODE
     {
        char filename[FILE_NAME_LEN];
        int filesize;
        struct FILE_NODE* next;
     };

### b. The data area is saved file data, its size is saved in the filesize of file head.


## (2) all file forms a linked list in cache. The max size of cache is 64 M.

## (3) The latest added file is at the end of the linked list.

## (4) The server deletes the file in front of the linked list until there is enough space to store the file in the cache.

## (5) Move files behind linked list to front of linked list, and modify file pointer.

## (6) single file in cache can not over 64M.

# 3. Different test cases you use to test your program:
## (1) requesting an invalid file
    The server will reply this file does not exists.

## (2) requesting from a dead server
    Client output message can not connect.

## (3) requesting a file that's over 64M size.
    The server will reply the file can not add to cache.

## (4) requesting a file, and check if the file is same with original file completely.

## (5) requesting a file, and check if the file is added to cache.

## (6) requesting a file, and then request the file again. check if the file is sent from cache. check if the file is same with original file completely.

## (7) For example, requesting a.pdf and b.pdf one by one, and then requesting a.pdf again. check if a.pdf is same with original file completely.

## (8) requesting few files that's over 64M size. Check if there is some file is deleted from cache and the new file is added to cache.

## (9) When the cache is full, requesting few files which are in cache. check if those files are same with original file completely.

## (10) requesting files randomly 50 times, check if the server is stable.
