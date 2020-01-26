# TcpServerClient
 TCP client for retrieving files, a TCP server for serving the requested files.
<br>
The client, after establishing a connection with the server, sends over the name of the file it requests. Then, the server searches for the file in its memory cache followed by a predefined directory. If the file is found in either of the places, the server then transmits the content of file back to the client via the same connection, and caches the file content, if it is not inside the memory cache.
<br>
The size limitation of the memory cache is 64MB. If the file size is more than 64MB on disk, you do not cache it in memory. And if the cache size will exceed 64MB after putting the file into memory, you have to purge enough cached content to make sure the entire cache size is restricted to 64MB anytime.
## 
