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
## 
