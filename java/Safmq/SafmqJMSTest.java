import java.net.URI;
import java.util.Enumeration;
import java.util.Hashtable;

import javax.jms.BytesMessage;
import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageProducer;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.Session;
import javax.jms.TemporaryQueue;
import javax.jms.TextMessage;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NameNotFoundException;

import junit.framework.Assert;
import junit.framework.TestCase;

import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.Safmq;


public class SafmqJMSTest extends TestCase {
	final static String TEST_QUEUE[] = { "test", "foo", "bar", "thequeue", "notthequeue" };
	
	String getProviderURL() {
		return "safmq://localhost:40000";
	}
	
	Context getInitialContext() throws Exception {
		Hashtable environment = null;
		
		environment = new Hashtable();
		environment.put(Context.INITIAL_CONTEXT_FACTORY, com.safmq.jms.InitialContextFactory.class.getName());
		environment.put(Context.PROVIDER_URL, getProviderURL());
		environment.put(Context.SECURITY_PRINCIPAL, "admin");
		environment.put(Context.SECURITY_CREDENTIALS, "");
        
		return new InitialContext(environment);
	}
	
	MQConnection connectToServer(URI uri, String user, String password) {
		try {
			MQConnection con = MQBuilder.buildConnection(uri, user, password);
			Assert.assertTrue("Successfully Connected", con != null);
			return con;
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), e == null);
		}
		return null;
	}

	void createEmptyQueue(MQConnection con, String queueName) {
		int ec;
		ec = con.DeleteQueue(queueName);
		Assert.assertTrue("Failed to delete queue: " + queueName, (ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST));

		ec = con.CreateQueue(queueName);
		Assert.assertTrue("Failed to create queue: " + queueName, ec == Safmq.EC_NOERROR);
	}

	void addAllQueues(MQConnection con) {
		for (int x = 0; x < TEST_QUEUE.length; x++) {
			createEmptyQueue(con, TEST_QUEUE[x]);
		}
	}
	
	public void test_SafmqJNDI_SubContexting() throws Exception {
		Context ctx = getInitialContext();
		
		ctx.createSubcontext("battey");
		ctx.createSubcontext("battey/children");
		ctx.bind("/battey/name", "matthew");
		ctx.bind("/battey/age", new Integer(34));
		ctx.bind("/battey/children/son", "marcus");

		Object o = ctx.lookup("battey");
		Assert.assertTrue("instance of context", o instanceof Context);
		
		Assert.assertTrue("Subcontext name equals matthew", ((Context)o).lookup("/name").equals("matthew"));
		Assert.assertTrue("Subcontext name equals matthew", ((Context)o).lookup("//name").equals("matthew"));
		Assert.assertTrue("Subcontext name equals matthew", ((Context)o).lookup("name").equals("matthew"));
		Assert.assertTrue("Subcontext age equasl 34", ((Context)o).lookup("/age").equals(new Integer(34)));
		Assert.assertTrue("Subcontext children son equals marcus", ((Context)o).lookup("/children/son").equals("marcus"));
		
		try {
			o = null;
			o = ctx.lookup("//name");
			Assert.assertTrue("//name not found", false);
		} catch (NameNotFoundException e) {
		}
		try {
			o = ctx.lookup("//battey/name");
			Assert.assertTrue("name == matthew", o.equals("matthew"));
			o = ctx.lookup("/battey/name");
			Assert.assertTrue("name == matthew", o.equals("matthew"));
			o = ctx.lookup("battey/name");
			Assert.assertTrue("name == matthew", o.equals("matthew"));
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), false);
		}
		try {
			o = ctx.lookup("/battey/children/son");
			Assert.assertTrue("name == matthew", o.equals("marcus"));
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), false);
		}
	}
	
	
	void setupQueues() throws Exception {
		MQConnection mqcon = connectToServer(new URI(getProviderURL()), "admin","");
		addAllQueues(mqcon);
		mqcon.Close();
	}
	
	
	
	public void test_ContextQueueLookup() throws Exception {
		setupQueues();
		Context ctx = getInitialContext();

		for(int x = 0; x < TEST_QUEUE.length; x++) {
			Queue q = (Queue)ctx.lookup(TEST_QUEUE[x]);
			Assert.assertTrue("Queue Name", q.getQueueName() != null);
		}
	
	}
	
	public void test_SendEnumerateReceive() throws Exception {
		setupQueues();
		Context ctx = getInitialContext();

		ConnectionFactory 	cf = (ConnectionFactory)ctx.lookup("ConnectionFactory");
		Queue				q = (Queue)ctx.lookup(TEST_QUEUE[0]);
		Connection 			con = cf.createConnection();
		Session				session = con.createSession(false, Session.AUTO_ACKNOWLEDGE);
		QueueBrowser		queueBrowser = session.createBrowser(q);
		Message				m;
		
		MessageProducer		messageProducer = session.createProducer(q);
		TextMessage			textMessage = session.createTextMessage();
		BytesMessage		bytesMessage = session.createBytesMessage();
		
		textMessage.setText("Hello World");
		
		int inCount = 0;
		textMessage.setStringProperty("label", ""+inCount);
		messageProducer.send(textMessage);
		inCount++;

		bytesMessage.setStringProperty("label", ""+inCount);
		messageProducer.send(bytesMessage);
		inCount++;

		int count = 0;
		Enumeration e = queueBrowser.getEnumeration();
		while (e.hasMoreElements()) {
			m = (Message)e.nextElement();
			count++;
		}
		Assert.assertTrue("Enumerated " + inCount + " messages", count == inCount);
		queueBrowser.close();
		
		
		MessageConsumer	consumer = session.createConsumer(q);
		count = 0;
		while (true) {
			m = consumer.receiveNoWait();
			if (m == null)
				break;
			Assert.assertTrue("Correct Label", m.getStringProperty("label").equals(""+count));
			count++;
		}
		Assert.assertTrue("Enumerated " + inCount + " messages", count == inCount);
		consumer.close();
		session.close();
		con.close();
		ctx.close();
	}
	
	public void test_TempQueueSendEnumerateReceive() throws Exception {
		setupQueues();
		Context ctx = getInitialContext();

		ConnectionFactory 	cf = (ConnectionFactory)ctx.lookup("ConnectionFactory");
		Connection 			con = cf.createConnection();
		Session				session = con.createSession(false, Session.AUTO_ACKNOWLEDGE);
		TemporaryQueue		q = session.createTemporaryQueue();
		QueueBrowser		queueBrowser = session.createBrowser(q);
		Message				m;
		
		MessageProducer		messageProducer = session.createProducer(q);
		TextMessage			textMessage = session.createTextMessage();
		BytesMessage		bytesMessage = session.createBytesMessage();
		
		textMessage.setText("Hello World");
		
		int inCount = 0;
		textMessage.setStringProperty("label", ""+inCount);
		messageProducer.send(textMessage);
		inCount++;

		bytesMessage.setStringProperty("label", ""+inCount);
		messageProducer.send(bytesMessage);
		inCount++;

		int count = 0;
		Enumeration e = queueBrowser.getEnumeration();
		while (e.hasMoreElements()) {
			m = (Message)e.nextElement();
			count++;
		}
		Assert.assertTrue("Enumerated " + inCount + " messages", count == inCount);
		queueBrowser.close();
		
		
		MessageConsumer	consumer = session.createConsumer(q);
		count = 0;
		while (true) {
			m = consumer.receiveNoWait();
			if (m == null)
				break;
			Assert.assertTrue("Correct Label", m.getStringProperty("label").equals(""+count));
			count++;
		}
		Assert.assertTrue("Enumerated " + inCount + " messages", count == inCount);
		consumer.close();
		q.delete();
		session.close();
		con.close();
		ctx.close();
	}
}
