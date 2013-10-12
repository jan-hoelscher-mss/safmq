import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;

import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageProducer;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.Session;
import javax.jms.TextMessage;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingEnumeration;


public class Test {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
//		UUID	u = UUID.randomUUID();
//		com.safmq.UUID	csu = new com.safmq.UUID(u);
//		
//		System.out.println("UUID: " + u);
//		System.out.println("CSU:  " + csu);
//		
//		
//		URI uri = new URI("safmq://localhost/myQueue");
//		System.out.println("uri.getPath():" + uri.getPath());
//		
//		CompositeName	cn = new CompositeName("/queues/foo/yada\\/bambada");
//		Enumeration		e = cn.getAll();
//		while (e.hasMoreElements())
//			System.out.println("Name Element: \"" + e.nextElement() + "\"");
//		System.out.println("Suffix:" + cn.getSuffix(2));
//		System.out.println("Prefix:" + cn.getPrefix(2));
//		
//		
		Hashtable environment = null;
		
		environment = new Hashtable();
		environment.put(Context.INITIAL_CONTEXT_FACTORY, com.safmq.jms.InitialContextFactory.class.getName());
		environment.put(Context.PROVIDER_URL, "safmq://localhost");
		environment.put(Context.SECURITY_PRINCIPAL, "admin");
		environment.put(Context.SECURITY_CREDENTIALS, "");
        
		Context ctx = new InitialContext(environment);
		
		ctx.createSubcontext("battey");
		ctx.createSubcontext("battey/children");
		ctx.bind("/battey/name", "matthew");
		ctx.bind("/battey/age", new Integer(34));
		ctx.bind("/battey/children/son", "marcus");
		
		NamingEnumeration ne = ctx.list("/");
		while (ne.hasMore()) {
			System.out.println(ne.next());
		}
		
		ne = ctx.listBindings("/");
		while (ne.hasMore()) {
			System.out.println(ne.next());
		}
		
		System.out.println("lookup(Q0000):   " + ctx.lookup("Q0000"));
		System.out.println("lookup(/Q0000):  " + ctx.lookup("/Q0000"));
		System.out.println("lookup(//Q0000): " + ctx.lookup("//Q0000"));
		
		System.out.println("===========================================");
		ne = ctx.listBindings("/battey");
		while (ne.hasMore()) {
			System.out.println(ne.next());
		}
		
		System.out.println("===========================================");
		ne = ctx.listBindings("/battey/children");
		while (ne.hasMore()) {
			System.out.println(ne.next());
		}
		
		ConnectionFactory 	cf = (ConnectionFactory)ctx.lookup("ConnectionFactory");
		Queue				q = (Queue)ctx.lookup("testQ");
		Connection 			con = cf.createConnection();
		Session				session = con.createSession(false, Session.AUTO_ACKNOWLEDGE);
		//Queue				q = session.createQueue("testQ");
		QueueBrowser		qb = session.createBrowser(q);
		Message				m;
		
		MessageProducer		mp = session.createProducer(q);
		TextMessage			tm = session.createTextMessage();
		
		tm.setStringProperty("label", "my label");
		tm.setText("Hello World");
		
		mp.send(tm);
		mp.send(tm);
		mp.send(tm);
		
		Enumeration e = qb.getEnumeration();
		while (e.hasMoreElements()) {
			m = (Message)e.nextElement();
			System.out.println("id:    " + m.getJMSMessageID());
			System.out.println("label: " + m.getStringProperty("label"));
			System.out.println("date:  " + (new Date(m.getJMSTimestamp())));
		}
		
		System.out.println("=============================================");
		MessageConsumer	c = session.createConsumer(q);
		while (true) {
			m = c.receiveNoWait();
			if (m == null)
				break;
			System.out.println("id:    " + m.getJMSMessageID());
			System.out.println("label: " + m.getStringProperty("label"));
			System.out.println("date:  " + (new Date(m.getJMSTimestamp())));
			System.out.println("Text:  " + ((TextMessage)m).getText());
		}
		ctx.close();
	}
}
