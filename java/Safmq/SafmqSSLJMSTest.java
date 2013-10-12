import java.net.URI;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Hashtable;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import junit.framework.Assert;

import com.safmq.MQBuilder;
import com.safmq.MQConnection;


public class SafmqSSLJMSTest extends SafmqJMSTest {
	static X509Certificate[] certs = {};
	class TrustMgr implements X509TrustManager {
		TrustMgr() { }
		public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException { }
		public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException { }
		public X509Certificate[] getAcceptedIssuers() { return certs; }
	}
	
	String getProviderURL() {
		return "safmqs://localhost:40001";
	}

	/**
	 * Creates an SSL enabled initial context.
	 */
	Context getInitialContext() throws Exception {
		Hashtable environment = null;
		SSLContext ctx = SSLContext.getInstance("SSL");
		TrustManager[] managers = { new TrustMgr() };
		ctx.init(null, managers, null);
		
		environment = new Hashtable();
		environment.put(Context.INITIAL_CONTEXT_FACTORY, com.safmq.jms.InitialContextFactory.class.getName());
		environment.put(Context.PROVIDER_URL, getProviderURL());
		environment.put(Context.SECURITY_PRINCIPAL, "admin");
		environment.put(Context.SECURITY_CREDENTIALS, "");
		environment.put(com.safmq.jms.Context.SSL_CONTEXT, ctx);
        
		return new InitialContext(environment);
	}
	
	MQConnection connectToServer(URI uri, String user, String password) {
		try {
			SSLContext ctx = SSLContext.getInstance("SSL");
			X509TrustManager[] managers = {new TrustMgr()};
			ctx.init(null, managers, null);
			
			MQConnection con = MQBuilder.buildConnection(uri, user, password,ctx);
			Assert.assertTrue("Successfully Connected", con != null);
			return con;
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), e == null);
		}
		return null;
	}
}
