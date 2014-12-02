import java.io.IOException;
import java.util.*;
        
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class Indexing
{
  public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> 
  { 
    private Text one = new Text("1");
	private Text word = new Text();
        
    public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
        String filename = ((FileSplit)reporter.getInputSplit()).getPath().getName();
        String line = value.toString();
        line = line.replaceAll( "[\\p{P}+~$`^=|<>~'$^+=|<>]" , "");//replace all the unrelated symbols
        StringTokenizer tokenizer = new StringTokenizer(line);
        while (tokenizer.hasMoreTokens()) 
        {
        	String token = tokenizer.nextToken();
            word.set(token+":"+filename);
            output.collect(word, one);
        }
    }
  } 
 
  public static class PreReduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> 
  {
		public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException 
		{
			String[] term = key.toString().split(":");
			int sum=0;
			while (values.hasNext()) 
			{
				sum += Integer.parseInt(values.next().toString());
			}
 			output.collect(new Text(term[0]), new Text(term[1] + ":" + sum));
 		}
 }
 
  public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> 
  {
		public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException 
		{ 
			String keyvalue="";
			if(values.hasNext())
				keyvalue=values.next().toString();
			while (values.hasNext()) 
			{
				keyvalue+=","+values.next().toString();
			}
 			output.collect(key, new Text(keyvalue));
 		}
  }
 

 	public static void main(String[] args) throws Exception 
 	{
		JobConf conf = new JobConf(Indexing.class);
		conf.setJobName("wordcount");
		
		conf.setMapOutputKeyClass(Text.class);
		conf.setMapOutputValueClass(Text.class);
		
		conf.setOutputKeyClass(Text.class);
		conf.setOutputValueClass(Text.class);
		
		conf.setMapperClass(Map.class);
		conf.setCombinerClass(PreReduce.class);
		conf.setReducerClass(Reduce.class);
	//	conf.setInputFormat(TextInputFormat.class);
	//	conf.setOutputFormat(TextOutputFormat.class);
		FileInputFormat.setInputPaths(conf, new Path(args[0]));
		FileOutputFormat.setOutputPath(conf, new Path(args[1]));
		JobClient.runJob(conf);
 	}
        
}