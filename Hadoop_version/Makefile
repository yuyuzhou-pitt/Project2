#make file for Hadoop version

HADOOPHOME = /usr/share/hadoop
CLASSPATH = ${HADOOPHOME}/commons-configuration-1.6.jar:${HADOOPHOME}/hadoop-core-1.0.1.jar
#CLASSPATH = /usr/share/hadoop/hadoop-core-1.0.1.jar
OUTPUT = class/
JARNAME = MiniGoogle.jar
OPTIONS = -verbose

all: Prep Index MiniGoogle Jar

Prep:
ifneq ($(wildcard ${JARNAME}),)
	rm ${JARNAME};
endif
ifneq ($(wildcard ${OUTPUT}),)
	rm ${OUTPUT} -rf;
endif
	mkdir ${OUTPUT}

Index:
	javac ${OPTIONS} -classpath ${CLASSPATH} -d class src/*.java

MiniGoogle:
	javac ${OPTIONS} -classpath ${CLASSPATH} -d class src/MiniGoogle/*.java

Jar:
	jar -cvf ${JARNAME} -C ${OUTPUT} .

clean:
ifneq ($(wildcard ${JARNAME}),)
	rm ${JARNAME};
endif
ifneq ($(wildcard ${OUTPUT}),)
	rm ${OUTPUT} -rf;
endif
