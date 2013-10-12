import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.net.URI;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Vector;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import junit.framework.Assert;

import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.MQException;
import com.safmq.Safmq;
import com.safmq.X509Identity;

/**
 * Tests the capability of the Safmq Java client via 
 * 
 * @author Matthew Battey
 */
public class SafmqSSLTest extends SafmqTest {
	String user="npuser";
	String password="54321";
	String description="passwordless login test user";
	String subject = "OU=Testing, O=Safmq Java Test Client";
	String issuer = "C=US, ST=Nebraska, L=Omaha, O=SAFMQ, OU=Sample X509 Certificate, CN=Sample SAFMQ X509 Certificate";

	static X509Certificate certs[] = new X509Certificate[0];

	boolean verified = false;
	
	class MyTrustManager implements X509TrustManager {
		MyTrustManager() { // constructor
			// create/load keystore
		}

		public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
		}

		public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
			verified = true;
		}

		public X509Certificate[] getAcceptedIssuers() {
			return certs;
		}
	}

	public SafmqSSLTest() throws Exception {
		address = new URI("safmqs://admin:@localhost:40001");
		simpleAddress = new URI("safmqs://localhost:40001");
	}

	MQConnection connectToServer(URI uri, String user, String password) {
		try {
			SSLContext ctx = SSLContext.getInstance("SSL");
			TrustManager myTrustManagers[] = {new MyTrustManager()};
			ctx.init(null,myTrustManagers,null);

			MQBuilder.setSSLContext(ctx);
			MQConnection con = MQBuilder.buildConnection(uri, user, password);
			
			Assert.assertTrue("Successfully Connected", con != null);
			return con;
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), e == null);
		}
		return null;
	}
	
	/**
	 * Creates a user and associates an x.509 certificate with that user. Then
	 * attempts to login without a password for that user.
	 */
	public void test_PasswordlessSignon() throws Exception {
		int ec;
		
		
		MQConnection con = connectToServer(address,"","");
		
		ec = con.DeleteUser(user);
		Assert.assertTrue("Delete User", ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST);
		
		ec = con.CreateUser(user, password, description);
		Assert.assertTrue("Create User", ec == Safmq.EC_NOERROR);
		
		ec = con.AddUserIdentity(subject, issuer, user);
		Assert.assertTrue("AddUserIdentity", ec == Safmq.EC_NOERROR);
		
		Vector ids = new Vector();
		ec = con.EnumerateUserIdentities(user, ids);
		Assert.assertTrue("EnumerateUserIdentities", ec == Safmq.EC_NOERROR);
		Assert.assertTrue("X.509 ID Count", ids.size() > 0);
		// TODO: Check the results in the ids
		boolean idFound = false;
		for(int x = 0; x < ids.size(); x++) {
			X509Identity id = (X509Identity)ids.get(0);
			if (id.getIssuerDN().equals(issuer) && id.getSubjectDN().equals(subject))
				idFound = true;
		}
		Assert.assertTrue("Identity found in returned X.509 identities", idFound);
		con.Close();
	}
	
	SSLContext setupSSL() throws Exception {
		// Setup the SSL Context using the JKS file created via gen_test_cert in the java/cert directory
		SSLContext ctx = SSLContext.getInstance("SSL");
        KeyManagerFactory   kmf = KeyManagerFactory.getInstance("SunX509", "SunJSSE");
        TrustManagerFactory tmf = TrustManagerFactory.getInstance("SunX509", "SunJSSE");
        KeyStore ks = KeyStore.getInstance("JKS");
        InputStream resrstm = new FileInputStream("cert" + File.separator + "safmq_java_test.jks");
        
        ks.load(resrstm, "".toCharArray());
        kmf.init(ks, "".toCharArray());
        tmf.init(ks);
		ctx.init(kmf.getKeyManagers(),tmf.getTrustManagers(),null);
		
		MQConnection con = connectToServer(address,"","");
		int ec = con.DeleteUser(user);
		Assert.assertTrue("Delete User", ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST);
		
		ec = con.CreateUser(user, password, description);
		Assert.assertTrue("Create User", ec == Safmq.EC_NOERROR);
		
		ec = con.AddUserIdentity(subject, issuer, user);
		Assert.assertTrue("AddUserIdentity", ec == Safmq.EC_NOERROR);

		con.Close();
		
		return ctx;
	}
	
	public void test_NullLogin() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;

		try {
			con = MQBuilder.buildConnection(simpleAddress,null,null,ctx);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident null ident " + e.toString(), e == null);
		}
	}
	
	public void test_EmptyLogin() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;

		try {
			con = MQBuilder.buildConnection(simpleAddress, "", "", ctx);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident empty string " + e.toString(), e == null);
		}
	}

	public void test_NullPassword() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;

		try {
			con = MQBuilder.buildConnection(simpleAddress,user,null,ctx);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident name, null password " + e.toString(), e == null);
		}

	}

	public void test_EmptyPassword() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;

		try {
			con = MQBuilder.buildConnection(simpleAddress,user,"",ctx);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident, name empty password " + e.toString(), e == null);
		}
		
	}

	public void test_BadPassword() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;


		try {
			con = MQBuilder.buildConnection(simpleAddress,user,"12345",ctx);
			con.Close();
		} catch (MQException e) {
			Assert.assertTrue("Reconnect with x.509 ident, name wrong password" + e.toString(), e.getErrorcode() == Safmq.EC_NOTAUTHORIZED);
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident, name wrong password" + e.toString(), false); // this is the correct case
		}
		
	}

	public void test_DefaultSharedContext() throws Exception {
		SSLContext ctx = setupSSL();
		MQConnection con;

		MQBuilder.setSSLContext(ctx);
		try {
			con = MQBuilder.buildConnection(simpleAddress,null,null);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident null ident" + e.toString(), e == null);
		}

		try {
			con = MQBuilder.buildConnection(simpleAddress,"","");
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident empty string" + e.toString(), e == null);
		}

		try {
			con = MQBuilder.buildConnection(simpleAddress,user,null);
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident name, null password" + e.toString(), e == null);
		}

		try {
			con = MQBuilder.buildConnection(simpleAddress,user,"");
			con.Close();
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident, name empty password" + e.toString(), e == null);
		}
		
		try {
			con = MQBuilder.buildConnection(simpleAddress,user,"12345");
			con.Close();
		} catch (MQException e) {
			Assert.assertTrue("Reconnect with x.509 ident, name wrong password" + e.toString(), e.getErrorcode() == Safmq.EC_NOTAUTHORIZED);
		} catch (Exception e) {
			Assert.assertTrue("Reconnect with x.509 ident, name wrong password" + e.toString(), e != null);
		}
		

//		con = connectToServer(address,"","");
//		int ec = con.RemoveUerIdentity(subject, issuer);
//		Assert.assertTrue("RemoveUerIdentity", ec == Safmq.EC_NOERROR);
//		con.Close();
	}
}
