/*
 Copyright 2005-2007 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.


This software implements a Java interface to SAFMQ (see http://safmq.sourceforge.net).

Created on Jun 3, 2005
*/

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.URI;

import com.safmq.MQBuilder;
import com.safmq.MQException;
import com.safmq.MessageQueue;
import com.safmq.QueueMessage;
import com.safmq.Safmq;

/**
 * Note: this example expects a SAFMQ server to be running on the "localhost"
 * or same computer as the example is being run.  It also expects a user to exist
 * with the name "username" and password "password".  Additionally there should
 * be two queues "query" and "resposne", and the user "username" should have
 * read and write access to to this queue. 
 * 
 * @author matt
 */
public class RoundTripServer {
	static URI queue_name;
	static String user_name = "username";
	static String password = "password";
	
	static {
		try {
			queue_name = new URI("safmq://localhost/query");
		} catch (Exception e) {
		}
	}

	public static void main(String[] args) {
		try {
			// Allocate a connection to the query queue, we'll read from this queue
			MessageQueue 	mq = MQBuilder.buildMessageQueue(queue_name,user_name,password);
			// Allocate a messaeg so we can read from it.
			QueueMessage	msg = new QueueMessage();
			int error;
			
			// Try and retreive a message from the queue
			error = mq.Retreive(true,-1,msg);
			// Close the queue we don't need it any more, this release resources on the server
			mq.Close(); 
			if (error == Safmq.EC_NOERROR) {
				// Output the contents of the emssage
				System.out.println("Message Data Follows");
				System.out.println("Label: " + msg.getLabel());
				InputStream in = msg.getInputStream();
				byte		data[] = new byte[1024];
				int			read;
				while ( (read=in.read(data)) > 0) {
					System.out.write(data,0,read);
				}
				
				// allocate a response message				
				QueueMessage response = new QueueMessage();
				
				// Set the message's body type to give readers a clue to contents
				response.setBodyType(Safmq.BT_TEXT);
				// Set a name for the message, note this is optional, but some receivers
				// may choose an action depending on the name specified here
				response.setLabel("Round Trip Response");
				// Note the next line is the critical step, it ties the response message
				// with the original query message. 
				response.setReciptID(msg.getMessageID());
				
				// Get the output stream to fill the body.
				// In this case wrap it with a PrintWriter so that we can have formated output.
				PrintWriter pw = new PrintWriter(new OutputStreamWriter(response.getOutputStream()));
				pw.println("This is the response to the round trip client's query");
				pw.flush();
				
				// Connect to the response queue
				MessageQueue responseQueue = MQBuilder.buildMessageQueue(new URI(msg.getResponseQueueName()),user_name,password);
				error = responseQueue.Enqueue(response);
				responseQueue.Close();
				if (error != Safmq.EC_NOERROR)
					System.out.println("Error sending: "+Safmq.errorDecode(error));
			} else {
				System.out.println("Error retreiving: "+Safmq.errorDecode(error));	
			}
		} catch (MQException mqe) {
			mqe.printStackTrace();	
		} catch (IOException ioe) {
			ioe.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();	
		}	
	}
}
