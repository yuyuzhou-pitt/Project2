package MiniGoogle;
import java.io.*;
import java.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class MiniGoogle{
  public static class tinyMapper extends Mapper<Object, Text, Text, Text> {
	private Text outKey = new Text();

    @Override
    public void map (Object key,Text value,Context context) throws IOException, InterruptedException  {
    	Configuration conf = context.getConfiguration();
    	String keywords = conf.get("keyword");
		String line = value.toString();
		String [] terms = line.split("\t");//terms[0] is required keyword
        if(terms[0].equals(keywords)){
    		ArrayList<String> docnames = new ArrayList<String>();
    		ArrayList<Integer> times = new ArrayList<Integer>();	
			String[] keys = terms[1].toString().split(":|,");
			//System.out.println(terms[1] + "\t" + keys.length);
			for(int i=0;i<keys.length;i+=2){
				docnames.add(keys[i]);
				times.add(Integer.parseInt(keys[i+1]));
			}
//			for(int i=0;i<docnames.size();++i){
//				System.out.println(docnames.get(i)+" "+ times.get(i));
//			}
				
			for(int i=0;i<docnames.size()-1;++i){
				for(int j=i+1;j<docnames.size();++j){
					if(times.get(i)<times.get(j))
					{
						String stmp = docnames.get(i); docnames.set(i, docnames.get(j)); docnames.set(j,stmp);
						int itmp = times.get(i); times.set(i, times.get(j)); times.set(j, itmp);
					}
				}
			}
			StringBuffer retkey= new StringBuffer();
			if(docnames.size()>0)	retkey.append("\n" + docnames.get(0) + times.get(0) + "\n");
			for (int i = 1; i<docnames.size(); ++i){
				retkey.append(docnames.get(i) + " " + times.get(i) + "\n");
			}
	        context.write(new Text(terms[0]), new Text(retkey.toString()));
    	}
      }
    }
  
  public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException { //fix main
    Configuration conf = new Configuration();
    conf.set("keyword", args[2]);
    Job job = new Job(conf,"tinygoogle");
    
    job.setJarByClass(MiniGoogle.class);
    job.setMapperClass(tinyMapper.class);
    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(Text.class);

    job.setNumReduceTasks(0);
    
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    
    System.exit(job.waitForCompletion(true)?0:1);
    
  }
}