Project2
========

Mini Google system.

+-------------+
| How to Make |
+-------------+

1. Unpack the tarball Project2.tar:
$ tar xvf Project2.tar
2. Enter into the project directory:
$ cd Project2/
3. Build the project:
$ make

Three executable files are generated after making:
Project2/port-mapper/port-mapper
Project2/server/server
Project2/client/minigoogle

+-------------------+
| Register Services |
+-------------------+

Server register services into prot-mapper table on Port-Mapper.

= Steps =

1. Start port mapper daemon on Port Mapper:
$ ./port-mapper 

2. Start server daemon Server:
$ ./server 

3. Run register command on Server terminal. It will print feedback message from Port-Mapper:
(server)# register MapReduceLibrary 1
Congratulations, 2 services rigistered successfully!

4. List registerd services on Port-Mapper:
(port-mapper)# list
Server_IP     |Port |Program_name|Version|Procedure
127.0.0.1|38701|MapReduceLibrary|1|Index
127.0.0.1|38701|MapReduceLibrary|1|Reduce

+------------------+
| Request services |
+------------------+

Client request server info (IP and port) from Port Mapper. Client should provide program name,
version number, and procedure name.
   
= Pre-steps =
Server has registered services into port mapper table, as Register Services step 2 or step 4 shows.

= Steps =
1. When the Client requesting, Port-Mapper will send back all the matched Server ip and port:
$ ./minigoogle request MapReduceLibrary 1 Index
Server_IP     |Port |Program_name     |Version|Procedure
127.0.0.1|42467|MapReduceLibrary|1      |Index

Choose to use service on 127.0.0.1 with port 42467 for load balance.

+------------------+
| Execute services |
+------------------+

Minigoogle execute the services provided by Server. Client should provide program name,
version number, procedure name, input directory, and output directory.

= Pre-steps =
Server has registered services into port mapper table, as Register Services step 2 or step 4 shows.

= Steps =
1. When the Client requesting, Port-Mapper will send back all the matched Server ip and port:
$ ./minigoogle execute MapReduceLibrary 1 Index ../data ../output


Note:
Currently only Split finished, to be continue...
