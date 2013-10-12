using safmq.net;
using MbUnit.Framework;
using System;
using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
using System.Net.Security;
using System.Net;
using System.Security.Authentication;

namespace safmq.net.test
{
    [TestFixture]
    public class SafmqSSLTest : SafmqTest
    {
        public static bool ServerValidator(object sender,
                                     X509Certificate certificate,
                                     X509Chain chain,
                                     SslPolicyErrors sslPolicyErrors) {
            Console.WriteLine("Server Certificate: " + certificate.Issuer);
            Console.WriteLine("SslPolicyErrors: " + sslPolicyErrors);
            return true;
        }

        public static X509Certificate CertificateProvider(object sender,
                                    string targetHost,
                                    X509CertificateCollection localCertificates,
                                    X509Certificate remoteCertificate,
                                    string[] acceptableIssuers) {
            // TODO: Build the CertificateProvider so the tests will not fail            
            try {
                //X509Certificate c = new X509Certificate(@"e:\blacksheep\safmq.0.8\safmq.net\safmq.net.test\sample.p12", "test");
                //Console.WriteLine("Issuer: " + c.Issuer);
                //Console.WriteLine("Subject: " + c.Subject);
                //foreach (string issuer in acceptableIssuers) {
                //    Console.WriteLine("Acceptable Issuer: " + issuer);
                //}
                if (localCertificates.Count > 0) {
                    Console.WriteLine("Offering client certificate: " + localCertificates[0].Subject);
                    return localCertificates[0];
                }
            } catch (Exception e) {
                Console.WriteLine(e.ToString());
            }
            return null;
        }

        string user = "npuser";
        string password = "54321";
        string description = "passwordless login test user";
        string subject = "OU=Testing, O=Safmq Java Test Client";
        string issuer = "C=US, ST=Nebraska, L=Omaha, O=SAFMQ, OU=Sample X509 Certificate, CN=Sample SAFMQ X509 Certificate";

        public SafmqSSLTest() {
            address = new Uri("safmqs://admin:@localhost:40001");
            simpleAddress = new Uri("safmqs://localhost:40001");
        }

        MQConnection connectToServer(Uri uri, string user, string password) {
            try {
                SSLContext ctx = new SSLContext();
                ctx.ServerValidator = SafmqSSLTest.ServerValidator;

                MQBuilder.ctx = ctx;

                MQConnection con = MQBuilder.BuildConnection(uri, user, password);
                Assert.IsTrue(con != null, "Successfully Connected");
                return con;
            } catch (Exception e) {
                Assert.IsTrue(e == null, e.ToString());
            }
            return null;
        }

        SSLContext setupSSL() {
            // Setup the SSL Context using the JKS file created via gen_test_cert in the java/cert directory
            MQConnection con = connectToServer(address, "", "");

            ErrorCode ec = con.DeleteUser(user);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR || ec == ErrorCode.EC_DOESNOTEXIST, "Delete User");
            ec = con.CreateUser(user, password, description);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Create User");
            Console.WriteLine("Adding Identity, user: " + user + " subject: " + subject);
            ec = con.AddUserIdentity(subject, issuer, user);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "AddUserIdentity");

            con.Close();

            SSLContext ctx = new SSLContext();
            ctx.ServerValidator = new RemoteCertificateValidationCallback(SafmqSSLTest.ServerValidator);
            ctx.CertificateProvider = new LocalCertificateSelectionCallback(SafmqSSLTest.CertificateProvider);

            X509Certificate c = new X509Certificate(@"e:\blacksheep\safmq.trunk\safmq.net\safmq.net.test\sample.p12", "test");
            ctx.ClientCertificateList = new X509CertificateCollection();
            ctx.ClientCertificateList.Add(c);

            X509Certificate2 c2 = new X509Certificate2(@"e:\blacksheep\safmq.trunk\safmq.net\safmq.net.test\sample.p12", "test");
            X509Store store = new X509Store(StoreName.My);
            store.Open(OpenFlags.ReadWrite);
            store.Add(c2);
            store.Close();

            return ctx;
        }


        /**
         * Creates a user and associates an x.509 certificate with that user. Then
         * attempts to login without a password for that user.
         */
        [Test]
        [TestSequence(20)]
        public void test_PasswordlessSignon() {
            ErrorCode ec;

            MQConnection con = connectToServer(address, "", "");

            ec = con.DeleteUser(user);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR || ec == ErrorCode.EC_DOESNOTEXIST, "Delete User");

            ec = con.CreateUser(user, password, description);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Create User");

            ec = con.AddUserIdentity(subject, issuer, user);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "AddUserIdentity");

            List<X509Identity> ids = new List<X509Identity>();
            ec = con.EnumerateUserIdentities(user, ids);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "EnumerateUserIdentities");
            Assert.IsTrue(ids.Count > 0, "X.509 ID Count");
            // TODO: Check the results in the ids
            bool idFound = false;
            for (int x = 0; x < ids.Count; x++) {
                X509Identity id = ids[0];
                if (id.IssuerDN.Equals(issuer) && id.SubjectDN.Equals(subject))
                    idFound = true;
            }
            Assert.IsTrue(idFound, "Identity found in returned X.509 identities");
            con.Close();
        }


        [Test]
        [TestSequence(21)]
        public void test_NullLogin() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, null, null, ctx);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Reconnect with x.509 ident null ident " + e.ToString());
            }
        }

        [Test]
        [TestSequence(22)]
        public void test_EmptyLogin() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, "", "", ctx);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident empty string " + e.ToString());
            }
        }

        [Test]
        [TestSequence(23)]
        public void test_NullPassword() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, null, ctx);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident name, null password " + e.ToString());
            }

        }

        [Test]
        [TestSequence(24)]
        public void test_EmptyPassword() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, "", ctx);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident, name empty password " + e.ToString());
            }

        }

        [Test]
        [TestSequence(25)]
        public void test_BadPassword() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, "12345", ctx);
                con.Close();
            } catch (MQException e) {
                Assert.IsTrue(e.getErrorcode() == ErrorCode.EC_NOTAUTHORIZED, "Reconnect with x.509 ident, name wrong password" + e.ToString());
            } catch (Exception e) {
                Assert.IsTrue(false, "Connect attempt with x.509 ident, name wrong password" + e.ToString()); // this is the correct case
            }
        }

        [Test]
        [TestSequence(26)]
        public void test_DefaultSharedContext() {
            SSLContext ctx = setupSSL();
            MQConnection con;

            MQBuilder.ctx = ctx;

            try {
                con = MQBuilder.BuildConnection(simpleAddress, null, null);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident null ident" + e.ToString());
            }

            try {
                con = MQBuilder.BuildConnection(simpleAddress, "", "");
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident empty string" + e.ToString());
            }

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, null);
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident name, null password" + e.ToString());
            }

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, "");
                con.Close();
            } catch (Exception e) {
                Assert.IsTrue(e == null, "Connect attempt with x.509 ident, name empty password" + e.ToString());
            }

            try {
                con = MQBuilder.BuildConnection(simpleAddress, user, "12345");
                con.Close();
            } catch (MQException e) {
                Assert.IsTrue(e.getErrorcode() == ErrorCode.EC_NOTAUTHORIZED, "Reconnect with x.509 ident, name wrong password" + e.ToString());
            } catch (Exception e) {
                Assert.IsTrue(e != null, "Connect attempt with x.509 ident, name wrong password" + e.ToString());
            }
        }
    }
}