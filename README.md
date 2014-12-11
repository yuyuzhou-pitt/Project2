Project2: Mini Google system.
Yubo Feng, Yuyu Zhou
========


Part I: Socket version
----------------------

+-------------+
| How to Make |
+-------------+

1. Unpack the tarball Project2.tar:
$ tar xvf Project2.tar
2. Enter into the project directory:
$ cd Project2/
3. Build the project:
$ make

Three executable files for socket version are generated after making:
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

+----------+
| Indexing |
+----------+

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


+-----------+
| Searching |
+-----------+

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
$ cat ../output/*


Part II: Hadoop Version
-----------------------

+-------------+
| How to Make |
+-------------+

It's necessary to build on a Hadoop environment.

1. Logon the Hadoop machine
2. Enter into the project directory:
$ cd Project2/Hadoop_version
3. Build the project:
$ make

A jar packet for Hadoop version is generated after making:
Project2/Hadoop_version/MiniGoogle.jar

+----------+
| Indexing |
+----------+

= Pre-steps =
A Hadoop environment is setup and ready to use.

= Steps =
1. Copy the data from local to hdfs:
$ hadoop dfs -mkdir data
$ hadoop dfs -copyFromLocal ../data/*txt data
2. index the data:
$ hadoop jar MiniGoogle.jar Indexing data index
3. Check the output result:
$ hadoop dfs -cat data/part-m-00000


+-----------+
| Searching |
+-----------+

= Pre-steps =
A Hadoop environment is setup and ready to use.

= Steps =
1. Search the keyword "what a nice day" in the index directory:
$ hadoop jar MiniGoogle.jar MiniGoogle/MiniGoogle index result what a nice day
2. Check the search result:
$ hadoop dfs -cat result/part-r-00000

Part III: Performance
---------------------

This section compares the performance between the Socket version and the Hadoop version.

= Steps = 

Socket version:

1. Defing TIMESTAMP in client/socket_execute.c and rebuild the project:
   #define TIMESTAMP 1

2. Run Indexing step 1 for 10 times, and record the detailed time (take real time for reference).
$ for i in 0 1 2 3 4 5 6 7 8 9; do time ./minigoogle execute MapReduceLibrary 1 Index ../data ../index; done

3. Run Search step 1 for 10 times, and record the detailed time (take real time for reference).
$ for i in 0 1 2 3 4 5 6 7 8 9; do time ./minigoogle execute MapReduceLibrary 1 Search ../index/ ../output "how are you doing"; done

Hadoop version:

1. Run Indexing step 2 for 10 times, and record the Map-Reduce Framework/CPU time spent (ms) (take real time for reference):
$ for i in 0 1 2 3 4 5 6 7 8 9; do time hadoop jar MiniGoogle.jar Indexing data index; done

2. Run Searching step 1 for 10 times, and record the Map-Reduce Framework/CPU time spent (ms) (take real time for reference):
$ for i in 0 1 2 3 4 5 6 7 8 9; do time hadoop jar MiniGoogle.jar MiniGoogle/MiniGoogle index result what a nice day; done

= Results =

The time spend result (in second) recorded as below:

	Wordcount	Sort(W+S)	Index(W+S+I)	Search
Socket	1.146928	2.305397	4.541908	0.164810
Socket	1.202855	2.336820	4.632236	0.165075
Socket	1.307164	2.435692	4.738004	0.170763
Socket	1.438589	2.605878	5.074086	0.168736
Socket	1.470344	2.612354	5.026402	0.165218
Socket	1.585199	2.707569	5.225632	0.166475
Socket	1.694932	2.848935	5.386795	0.171973
Socket	1.739059	2.924582	5.588107	0.168603
Socket	1.242837	2.380279	4.675156	0.175365
Hadoop	(CPU time spent (s))		18.840		4.120
Hadoop					19.690		4.340
Hadoop					18.910		3.770
Hadoop					19.540		3.830
Hadoop					18.010		3.790
Hadoop					19.530		3.720
Hadoop					17.340		3.730
Hadoop					17.150		3.630
Hadoop					19.610		3.490
Hadoop					21.460		6.540

Part IV: Reference
------------------

http://wiki.apache.org/hadoop/
http://hadoop.apache.org/docs/r1.0.4/commands_manual.html

