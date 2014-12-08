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

+----------+
| Register |
+----------+

Server register services into prot-mapper table on Port-Mapper.

= Steps =

1. Start port mapper daemon on Port Mapper machine: 
$ cd port-mapper/
$ ./port-mapper 

2. Start server daemon on Server machine(s):
$ cd server/
$ ./server 

3. Run register command on Server terminal (3 servers). It will print feedback message from Port-Mapper:
(server)# register MapReduceLibrary 1
Congratulations, 2 services rigistered successfully!

4. List registerd services on Port-Mapper:
(port-mapper)# list

+---------+
| Request |
+---------+

Client request server info (IP and port) from Port Mapper. Client should provide program name,
version number, and procedure name.
   
= Pre-steps =
1. Server has registered services into port mapper table, as Register Services step 2 or step 4 shows.

2. On client machine, enter into client directory:
$ cd client/

= Steps =
1. When the Client requesting, Port-Mapper will send back all the matched Server ip and port:
$ ./minigoogle request MapReduceLibrary 1 Index                             
$ ./minigoogle request MapReduceLibrary 1 Search

+-------+
| Index |
+-------+

Minigoogle execute the services provided by Server. Client should provide program name,
version number, procedure name, input directory, and output directory.

= Pre-steps =
1. Server has registered services into port mapper table, as Register Services step 2 or step 4 shows.

2. On client machine, enter into client directory:
$ cd client/

3. There are some text file to be indexing in the input directory (../data):
$ ls ../data/ -lsh

= Steps =
1. When the Client executing, it print out the file operation information of each steps:

$ ./minigoogle execute MapReduceLibrary 1 Index ../data ../index

2. The index task will be delivered to three servers separately.
2.1 Check the message on server1.
2.2 Check the message on server2.
2.3 Check the message on server3.

3. Check the output directory (../index) on client side:
$ ls -lsh ../index/


+--------+
| Search |
+--------+

Minigoogle execute the services provided by Server. Client should provide program name,
version number, procedure name, input directory, output directory, and terms to search.

= Pre-steps =

1. Index has done.

2. On client machine, enter into client directory:
$ cd client/

= Steps =
1. When the Client searching, it print out the file operation information of each steps:

$ ./minigoogle execute MapReduceLibrary 1 Search ../index/ ../output "how are you doing"

2. The searching task distributed to different servers:
2.1 Check the message on server1.
2.2 Check the message on server2.
2.3 Check the message on server3.

3. Check the output on client:
$ cat ../output/how.txt 
$ cat ../output/are.txt 
$ cat ../output/you.txt 
$ cat ../output/doing.txt 
