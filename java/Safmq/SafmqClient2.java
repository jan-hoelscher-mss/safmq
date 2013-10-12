/*
 * Created on Mar 22, 2005
 *
 */
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.URI;
import java.text.DateFormat;
import java.util.Date;
import java.util.GregorianCalendar;

import com.safmq.CursorHandle;
import com.safmq.MQBuilder;
import com.safmq.MessageQueue;
import com.safmq.QueueMessage;
import com.safmq.Safmq;
import com.safmq.UUID;

/**
 * @author Matt
 *
 */
public class SafmqClient2 {

	public static void main(String[] args) {
		try {
			DateFormat df = DateFormat.getDateTimeInstance(DateFormat.SHORT,DateFormat.MEDIUM);
			MessageQueue	que = MQBuilder.buildMessageQueue(new URI("//localhost:9000/messages"),"admin","");
			QueueMessage	msg = new QueueMessage();
			int				error;
			
			msg.setLabel("Test Matt 1");
			msg.setBodyType(Safmq.BT_TEXT);
			PrintWriter	out = new PrintWriter(new OutputStreamWriter(msg.getOutputStream()));
			out.println("This is a test only a test");
			GregorianCalendar gc = new GregorianCalendar();
			out.println("Date and Time: " + df.format(gc.getTime()));
			out.flush();
	
			msg.setTimeToLiveSeconds(4);
			msg.setTTLErrorWanted(true);
			msg.setResponseQueueName("//localhost:9000/messages");
			
			que.Enqueue(msg);
			System.out.println("Sent Message ID: "+msg.getMessageID());

			UUID id = (UUID)msg.getMessageID().clone();


			CursorHandle	cur = new CursorHandle();
			error = que.OpenCursor(cur);
			if (error != Safmq.EC_NOERROR)
				System.out.println("Error: " + error);
			
//			msg = new QueueMessage();
//			error=que.PeekFront(true,0,msg);
//			if (error == Safmq.EC_NOERROR) {
//				System.out.println("Msg:"+msg.getMessageID());
//			}
			
			msg = new QueueMessage();
			while ((error=que.PeekCursor(true,cur,msg)) == Safmq.EC_NOERROR) {
				System.out.println("Label: " + msg.getLabel() + "Msg:"+msg.getMessageID());
				
				error=que.AdvanceCursor(cur);
				if (error != Safmq.EC_NOERROR) 
					break;
				msg = new QueueMessage();
			}
			System.out.println("Error: " + error);

//			que.CloseCursor(cur);
//			cur = new CursorHandle();
//			que.OpenCursor(cur);
			
//			System.out.println("Requesting ID:" + id);
//			if ((error=que.SeekID(id,-1,cur)) == Safmq.EC_NOERROR) {
//				msg = new QueueMessage();
//				error = que.RetreiveCursor(true,cur,msg);
//				if (error == Safmq.EC_NOERROR) {
//					System.out.println("Label:" + msg.getLabel());
//					System.out.println("Recipt ID: " + msg.getReciptID());
//				} else {
//					System.out.println("RetreiveCursor Error:"+error);
//				}
//			} else {
//				System.out.println("SeekID Error:" + error);
//			}
			
			que.CloseCursor(cur);		

//			System.out.println("Emptying Queue: ==============================");
//			msg = new QueueMessage();
//			while ((error=que.Retreive(true,0,msg)) == Safmq.EC_NOERROR) {
//				System.out.println("Label: " + msg.getLabel() + " Msg:"+msg.getMessageID()+ " Recipt:" + msg.getReciptID());
//				msg = new QueueMessage();
//			}

			
			System.out.println("Requesting ID:" + id);
 			if ((error=que.RetrieveID(true,id,-1,msg)) == Safmq.EC_NOERROR) {
				System.out.println("Retreived Label: "+msg.getLabel());
				System.out.println("Retreived Message: \n");
				InputStream in = msg.getInputStream();
				byte		data[] = new byte[1024];
				int			read;
				while ( (read=in.read(data)) > 0) {
					System.out.write(data,0,read);
				}
				System.out.println("\nRetreived Message ID: "+msg.getMessageID()+"\n");
				System.out.println("Received Message Timestamp: " + df.format(new Date(msg.getTimeStamp())));
				msg = new QueueMessage();
			}
			
			error = que.Close();
			System.out.println("Queue Close: " + error);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
