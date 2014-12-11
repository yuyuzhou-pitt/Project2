package MiniGoogle;
import java.io.*;
import java.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Mapper.Context;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

//import Indexing.Reduce;

public class MiniGoogle{
  public static class tinyMapper extends Mapper<Object, Text, Text, Text> {
	private Text outKey = new Text();

    @Override
    public void map (Object key,Text value,Context context) throws IOException, InterruptedException  {
    	Configuration conf = context.getConfiguration();
    	String keywords = conf.get("keyword");
    	//System.out.println(keywords);
		String line = value.toString();
		String [] terms = line.split("\t");//terms[0] is required keyword
		String [] multiKeywords = keywords.split(" ");
		boolean foundflag=false;
		for(int i=0;i<multiKeywords.length;++i){
			if(multiKeywords[i].equals(terms[0])){
				foundflag=true;
			}
		}
		
        if(foundflag){
	     context.write(new Text("Keywords"), new Text(terms[0]+"@"+terms[1].toString()) );
    	}
      }
    }
  
  public static class Reduce extends Reducer<Text, Text, Text, Text> {
	    @Override
	    public void reduce (Text key,Iterable<Text> values,Context context) throws IOException, InterruptedException {
	    	Hashtable<String, Integer> wordCounts = new Hashtable<String, Integer>();
	    	ArrayList docName = new ArrayList<String>();
	    	LinkedList wordName = new LinkedList<String>();
	    	while (values.iterator().hasNext()) 
			{
	    		String[] items = values.iterator().next().toString().split("@");
	    		if(!wordName.contains(items[0])){
	    			wordName.add(items[0]); 
	    		}
	    		String[] keys = items[1].split(":|,");
				for(int i=0;i<keys.length;i+=2){
					if(!docName.contains(keys[i])){
						docName.add(keys[i]);
						wordCounts.put(keys[i],0);
					}
					int tmp = wordCounts.get(keys[i]);
					tmp += Integer.parseInt(keys[i+1]);
					wordCounts.put(keys[i], tmp);
				}
			}
	    	
	    	for(int i=0;i<docName.size()-1;++i){
				for(int j=i+1;j<docName.size();++j){
					if(wordCounts.get(docName.get(i))<wordCounts.get(docName.get(j)))
					{
						String stmp = docName.get(i).toString(); docName.set(i, docName.get(j).toString()); docName.set(j,stmp);
					}
				}
			}
	    	
	    	String retKey=wordName.get(0).toString();
	    	for(int i=1;i<wordName.size();++i){
	    		retKey+=","+wordName.get(i);
	    	}
	    	
	    	String retValue=""; //="\n" + docName.get(0).toString() + ":" + wordCounts.get(docName.get(0).toString());
	    	for(int i=0;i<docName.size();++i){
	    		retValue+="\n" + docName.get(i).toString() + ": " + wordCounts.get(docName.get(i).toString());
	    	}
	    	context.write(new Text(retKey), new Text(retValue) );
	    }
	  }
 
  public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException { //fix main
    Configuration conf = new Configuration();
    String tmp = new String();
    for(int i=2;i<args.length;++i){
    	tmp = tmp + " "+ args[i];
    }
    conf.set("keyword", tmp);
    Job job = new Job(conf,"tinygoogle");
    
    job.setJarByClass(MiniGoogle.class);
    job.setMapperClass(tinyMapper.class);
    job.setReducerClass(Reduce.class);
    
    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(Text.class);

    //job.setNumReduceTasks(0);
    
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    
    System.exit(job.waitForCompletion(true)?0:1);
    
  }
}