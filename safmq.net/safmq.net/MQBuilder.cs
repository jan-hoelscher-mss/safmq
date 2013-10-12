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


Created on Aug 13, 2007
*/
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net.Sockets;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using System.Security.Authentication;

namespace safmq.net {
    /**
     * <summary>Provides the ability to specify SSL authentication and identity assertion
     * when establishing a connection to a SAFMQ server.</summary>
     * <remarks>
     * Pass an instance of this class to the <c>MQBuilder</c> <c>BuildessageQueue</c> or
     * <c>BuildConneciton</c> methods to establish an SSL authentication and identity context
     * when connecting to an SAFMQ server.
     * </remarks>
     */
    public class SSLContext {
        /// <summary>
        /// Server Certificate Validation delegate
        /// </summary>
        public RemoteCertificateValidationCallback ServerValidator = null;
        /// <summary>
        /// Client Certificate Provider delegate
        /// </summary>
        public LocalCertificateSelectionCallback CertificateProvider = null;
        /// <summary>
        /// Collection of X509 certificates to provide a client certificate
        /// </summary>
        public X509CertificateCollection ClientCertificateList = null;
        /// <summary>
        /// The request SSL protocol
        /// </summary>
        public SslProtocols RequestedProtocol = SslProtocols.Ssl3;

        /// <summary>
        /// A Boolean value that specifies whether the certificate revocation list is checked during authentication.
        /// </summary>
        public bool CheckRevcationList = true;
    }

    /**
     * <summary>
     * A static builder class which builds connections to SAFMQ servers and message
     * queues.
     * </summary>
     * <remarks>
     * <para>In the case that a connection to a queue server is sought, use the method
     * <c>MQBuilder.buildConnection(String url, String user, String password)</c>.
     * This method will evaluate the URL and create a new connection to the queue
     * server.</para>
     * 
     * <para>In the case that a connection to a message queue is sought, the use the
     * method <c>MQBuilder.buildMessageQueue(String url, String user, String password)</c>.
     * This method will evaluate the URL and create a new connection tot he queue
     * server and wrap that connection with a <c>MessageQueue</c> instance.</para>
     * 
     * @author Matt
     * </remarks>
     */
    public class MQBuilder {
	    private const int DEF_PORT = 9000;
        private const int DEF_SSL_PORT = 9001;
        private const string SAFMQ_PROTO = "safmq";
        private const string SAFMQ_SSL_PROTO = "safmqs";

        /**
         * Set the SslContext of the builder to allow for custom 
         * certificate authentication
         */
        public static SSLContext ctx = null;

        /**
         * <summary>Constructs a <c>MQConnection</c> connection object connected to a 
         * SAFMQ server. Uses the default SSLContext <c>ctx</c> which is a member
         * of this class.</summary>
         * 
         * <param name="uri">
         *                  The URI of the message queue object in the format:
         * 					<c>protocol://user:password@server-name:port/</c> 
         * 					where the ":port" , "user",	and "password" portion of the url 
         * 					is optional.  Valid selections for "protocol" are "safmq" for
         * 					a clear text connection and "safmqs" for a SSL encrypted connection.</param>
         * <param name="user">
         *                  The name of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * <param name="password">
         *                  The password of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * 
         * <returns>A valid <c>MQConnection</c> connection object connected to 
         * 		a SAFMQ server.</returns>
         * 
         * <exception cref="safmq.net.MQException">In the case that the server refuses the supplied 
         * 			login credintials, or in the case that the URL could not be correctly
         * 			parsed. See MQConnection.MQConnection(string,int,string,string) for
         * 			details on connection errors.</exception>
         * 
         * <exception cref="System.Exception">In the case that there is a network error attempting
         * 			to connect to the server via the TCP/IP network.</exception>
         * <seealso cref="MQConnection(Stream,string,string)"/>
         */
        static public MQConnection BuildConnection(Uri uri, string user, string password) {
            return BuildConnection(uri, user, password, null);
	    }

        /**
         * <summary>Constructs a <c>MQConnection</c> connection object connected to a 
         * SAFMQ server.</summary>
         * 
         * <param name="uri">
         *                  The URI of the message queue object in the format:
         * 					<c>protocol://user:password@server-name:port/</c> 
         * 					where the ":port" , "user",	and "password" portion of the url 
         * 					is optional.  Valid selections for "protocol" are "safmq" for
         * 					a clear text connection and "safmqs" for a SSL encrypted connection.</param>
         * <param name="user">
         *                  The name of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * <param name="password">
         *                  The password of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * <param name="ctx">
         *                  [optional] An SSLContext object usable for determining the method for validating
         *                  server certificates, providing a client certificate, etc. Passing null will use the
         *                  default settings in the <c>ctx</c> member.</param>
         * 
         * <returns>A valid <c>MQConnection</c> connection object connected to 
         * 		a SAFMQ server.</returns>
         * 
         * <exception cref="safmq.net.MQException">In the case that the server refuses the supplied 
         * 			login credintials, or in the case that the URL could not be correctly
         * 			parsed. See MQConnection.MQConnection(string,int,string,string) for
         * 			details on connection errors.</exception>
         * 
         * <exception cref="System.Exception">In the case that there is a network error attempting
         * 			to connect to the server via the TCP/IP network.</exception>
         * <seealso cref="MQConnection(Stream,string,string)"/>
         */
        static public MQConnection BuildConnection(Uri uri, string user, string password, SSLContext ctx) {
            string userinfo = uri.UserInfo;
            StringBuilder u, p;

            if (parseUserInfo(userinfo, u = new StringBuilder(), p = new StringBuilder())) {
                user = u.ToString();
                password = p.ToString();
            }

            return new MQConnection(allocSocket(uri.Scheme, uri.Host, uri.Port, ctx), user, password);
        }

        /**
         * <summary>Constructs a new <c>MQConnection</c> connection object and then a
         * <c>MessageQueue</c> to wrap it.  Uses the default SSLContext for SSL
         * connections specified by the <c>ctx</c> member.</summary>
         * 
         * 
         *<param name="uri">The URI of the message queue object in the format:
         * 					<c>protocol://user:password@server-name:port/queue</c> 
         * 					where the ":port" , "user",	and "password" portion of the url 
         * 					is optional.  Valid selections for "protocol" are "safmq" for
         * 					a clear text connection and "safmqs" for a SSL encrypted connection.</param>
         * <param name="user">
         *                  The name of the user which is connecting to the server, used if
         *   				not specified in the uri</param>
         * <param name="password">
         *                  The password of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * 
         * <returns>A valid <c>MessageQueue</c> connected to a message queue
         * 			on a SAFMQ server.</returns>
         * 
         * <exception cref="safmq.net.MQException">In the case that the server refuses the supplied 
         * 			login credintials, or in the case that the URL could not be correctly
         * 			parsed.  See errors from MQConnection(string,string,string) and 
         * 			MQConnection.OpenQueue(string,QueueHandle).</exception>
         * 
         * <exception cref="System.Exception"> In the case that there is a network error attempting
         * 			to connect to the server via the TCP/IP network.</exception>
         * 
         * <seealso cref="MQConnection(Stream,string,string)"/>
         * <seealso cref="MQConnection.OpenQueue(string,QueueHandle)"/>
         */
        static public MessageQueue BuildMessageQueue(Uri uri, string user, string password) {
            return BuildMessageQueue(uri, user, password, null);
        }

        /**
         * <summary>Constructs a new <c>MQConnection</c> connection object and then a
         * <c>MessageQueue</c> to wrap it.</summary>
         * 
         * 
         *<param name="uri">The URI of the message queue object in the format:
         * 					<c>protocol://user:password@server-name:port/queue</c> 
         * 					where the ":port" , "user",	and "password" portion of the url 
         * 					is optional.  Valid selections for "protocol" are "safmq" for
         * 					a clear text connection and "safmqs" for a SSL encrypted connection.</param>
         * <param name="user">
         *                  The name of the user which is connecting to the server, used if
         *   				not specified in the uri</param>
         * <param name="password">
         *                  The password of the user which is connecting to the server, used if
         * 					not specified in the uri</param>
         * <param name="ctx">
         *                  [optional] Specifies the SSLContext when making SSL connections.  If
         *                  null is passed, then the default settigns from the member <c>ctx</c>
         *                  are used.</param>
         * 
         * <returns>A valid <c>MessageQueue</c> connected to a message queue
         * 			on a SAFMQ server.</returns>
         * 
         * <exception cref="safmq.net.MQException">In the case that the server refuses the supplied 
         * 			login credintials, or in the case that the URL could not be correctly
         * 			parsed.  See errors from MQConnection(string,string,string) and 
         * 			MQConnection.OpenQueue(string,QueueHandle).</exception>
         * 
         * <exception cref="System.Exception"> In the case that there is a network error attempting
         * 			to connect to the server via the TCP/IP network.</exception>
         * 
         * <seealso cref="MQConnection(Stream,string,string)"/>
         * <seealso cref="MQConnection.OpenQueue(string,QueueHandle)"/>
         */
        static public MessageQueue BuildMessageQueue(Uri uri, string user, string password, SSLContext ctx) {
		    string userinfo = uri.UserInfo;
		    StringBuilder	u, p;
    		
		    if (parseUserInfo(userinfo,u=new StringBuilder(),p=new StringBuilder())) {
			    user = u.ToString();
			    password = p.ToString();
		    }

		    string resource = uri.AbsolutePath;
		    if (resource != null && resource.Length > 0 && resource[0] == '/')
			    resource = resource.Substring(1);

            return new MessageQueue(resource, new MQConnection(allocSocket(uri.Scheme, uri.Host, uri.Port, ctx), user, password));
	    }

        internal static Stream allocSocket(string protocol, string host, int port, SSLContext ctx) {
		    Stream	s;
		    if (protocol == null)
			    protocol = SAFMQ_PROTO;
		    if (protocol.Equals(SAFMQ_PROTO)) {
			    if (port == -1)
				    port = DEF_PORT;
                s = (new TcpClient(host, port)).GetStream();
            } else if (protocol.Equals(SAFMQ_SSL_PROTO)) {
			    if (port == -1)
				    port = DEF_SSL_PORT;
                s = (new TcpClient(host, port)).GetStream();

                SslStream ssl;
                if (ctx == null)
                    ctx = MQBuilder.ctx;

                if (ctx != null) {
                    ssl = new SslStream(s, false, ctx.ServerValidator, ctx.CertificateProvider);
                    //string name = ctx.ClientCertificateList != null && ctx.ClientCertificateList.Count > 0 ? ctx.ClientCertificateList[0].Subject : host;
                    ssl.AuthenticateAsClient(host, ctx.ClientCertificateList, ctx.RequestedProtocol, ctx.CheckRevcationList);
                } else {
                    ssl = new SslStream(s, false);
                    ssl.AuthenticateAsClient(host);
                }
                if (!ssl.IsEncrypted)
                    throw new MQException("Stream Not Encrypted", ErrorCode.EC_ERROR);

                s = ssl;
            } else {
			    throw new MQException("Unknown Protocol", ErrorCode.EC_ERROR);	
		    }
            return s;
	    }

        internal static bool parseUserInfo(string userinfo, StringBuilder user, StringBuilder password) {
            int o = userinfo.IndexOf(":");
            if (o > 0) {
                user.Append(userinfo.Substring(0, o));
                if (o + 1 < userinfo.Length)
                    password.Append(userinfo.Substring(o + 1));
            } else {
                user.Append(userinfo);
            }
            return user.Length > 0;
        }
    }
}
