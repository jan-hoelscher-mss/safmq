/*
 Copyright 2004-2008 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




 This software implements a platform independent Store and Forward Message Queue.
*/

/**
<h3>SAFMQ</h3>

<p>Safmq, the server, library, utilities and documentation are released under the Apache License Ver. 2.0.</p>

<pre>
   Copyright 2004-2008 Matthew J. Battey, Omaha, NE

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       <a href=http://www.apache.org/licenses/LICENSE-2.0 target=_blank>http://www.apache.org/licenses/LICENSE-2.0</a>

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
</pre>

<i>Builds of SAFMQ may include the OpenSSL library to implement SSL communications.</i>
<br>
<br>
<br>

<h3>OpenSSL</h3>

<pre>
  LICENSE ISSUES
  ==============

  The OpenSSL toolkit stays under a dual license, i.e. both the conditions of
  the OpenSSL License and the original SSLeay license apply to the toolkit.
  See below for the actual license texts. Actually both licenses are BSD-style
  Open Source licenses. In case of any license issues related to OpenSSL
  please contact openssl-core@openssl.org.

  OpenSSL License
  ---------------

 * ====================================================================
 * Copyright (c) 1998-2007 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 *

 Original SSLeay License
 -----------------------

 * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 *
</pre>




@memo Licensed under the Apache License, Version 2.0
*/

/**
These instructions are based on a Linux installation.

<ol>
<li> Obtain the SAFMQ source code
<li> Unpack the tarball
<li> Execute the "make" command
<li> Copy safmq to a destination like /usr/bin or /usr/local/bin
<li> Create a directory for the queue data Ex: /var/safmq
<li> Create a directory for the server configuration data (can be the same as the queue directory)
<li> Edit safmq.conf (default location for safmq.conf is /etc however, this can be specified on the 
		command line or via environment varaiable SAFMQ_CFG).
<li> Execute safmq ( the -D or --daemon parameter will start safmq in the background)
<ol>

@memo Procedure for Unix installation.
*/

/**
The following is a list of parameters available in the safmq.conf configruation file
for the safmq server. If the file specifed by SAFMQ_CFG
or via the command line cannot be found, safmq attempts to locate the
file in either "/etc" in case of a unix system or in the "%SystemRoot%" directory
aka. the "widnows" directory.

<dl>
<dt class=dppDefinitionName>port:9000
<dd>Specifies the clear-text listening port for the safmq server, queries will be served from this port<br>
	The default, if not defined, is 9000

<dt class=dppDefinitionName>address: *
<dd>Specifies the clear-text listening network address.  This is useful to 
	limit the available interfaces in multi homed systems.  Values are seperated by 
	semi-colons, with colons seperating addresses from ports.  If a port is not specified,
	the default port, set by the "port" parameter, will be used instead.  An asterisk "*" indicates
	all local interfaces.<br>
	<b>Example:</b>	<code>localhost:9000;192.168.1.1;*:9009;192.168.1.1</code>
	<br>
	The default, if not defined, is *

<dt class=dppDefinitionName>queue_dir: ./queues
<dd>Specifies the location of the queue data.<br>
	The default, if not defined, is ./queues

<dt class=dppDefinitionName>config_dir: ./queues
<dd>Specifies the location of the safmq security configuration files.<br>
	The default, if not defined, is ./queues

<dt class=dppDefinitionName>ssl_port:9001
<dd>Specifies the secure sockets layer listening port for secure communications<br>
	The default, if not defined, is 9001

<dt class=dppDefinitionName>ssl_address: *
<dd>Specifies the secure sockets layer listening network address.  This is useful to 
	limit the available interfaces in multi homed systems.  Values are seperated by 
	semi-colons, with colons seperating addresses from ports.  If a port is not specified,
	the default, port set by the "port" parameter, will be used instead.  An asterisk "*" indicates
	all local interfaces.<br>
	<b>Example:</b>	<code>localhost:9001;*:9002;192.168.1.1</code>
	<br>
	The default, if not defined, is *


<dt class=dppDefinitionName>ssl_cert: 
<dd>Spedifies the location of the ANSI X509 servercertificate file in PEM format<br>
	Thre is no default value.

<dt class=dppDefinitionName>ssl_key: 
<dd>Spedifies the location of the ANSI X509 privagte key file in PEM format<br>
	Thre is no default value.

<dt class=dppDefinitionName>ssl_ca: 
<dd>Spedifies the location of certificate authority file or directory<br>
	Thre is no default value.

<dt class=dppDefinitionName>ssl_client_cert:
<dd>Spedifies the location of the ANSI X509 client certificate file in PEM format<br>
	Thre is no default value<br>
	Note: can be the same as the ssl_cert if the server should use its server certificate
	when acting as a client.

<dt class=dppDefinitionName>ssl_client_key:
<dd>Spedifies the location of the ANSI X509 client privagte key file in PEM format<br>
	Thre is no default value<br>
	Note: can be the same as the ssl_cert if the server should use its server private key
	when acting as a client.

<dt class=dppDefinitionName>enable_forwarding: no
<dd>Enables the safmq server to forward messages to other message queue servers
	when requested to do so by the client.<br>
	The default, if not dfined, is no, other values include yes.

<dt class=dppDefinitionName>accept_forwards: no
<dd>Enables the safmq server to accept forwarded messages from other message queue servers.<br>
	The default, if not dfined, is no, other values include yes.

max_file_size:	41943040
<dd> maximum size in bytes of queue storage files defaults to 40MB if not set

<dt class=dppDefinitionName>file_handle_cache_size: 50
<dd> Number of file handles to keep open to improve file access
	performance, defaults to 50 if not set

<dt class=dppDefinitionName>startup_log<dd>startup events
<dt class=dppDefinitionName>shutdown_log<dd>shutdown events
<dt class=dppDefinitionName>signon_log<dd>ogs each login attempt w/ failures
<dt class=dppDefinitionName>user_log<dd>logs user create,delete,acl
<dt class=dppDefinitionName>group_log<dd>logs group create,delete,acl,user add,user delete
<dt class=dppDefinitionName>queue_log<dd>logs queue create,delete,acl
<dt class=dppDefinitionName>message_log<dd>logs ttl events
<dt class=dppDefinitionName>forward_log<dd>logs forwarding errors
<dt class=dppDefinitionName>info_log<dd>logs informational status messages


</dl>

<h3>Notes on Logging:</h3>
 Log events may be sent to 1 of 5 places<br>
 <ul>
 <li>The first <b>NONE</b> causes the event to not be recorded
 <li>The second <b>SYSTEM_LOG</b> causes the event to be sent to 
		the system logger, syslog on Unix systems and the NT Event Logger
		on Windows NT derived systems
 <li>The third allows log entries to be published to a file with
		strftime escape codes for time-based log file naming
 <li>The foruth location <b>STDERR</b> writes log events to stream
		#2 the standard-error output stream.
 <li>The last location <b>STDOUT</b> writes log events to stream #1
		the standard-output stream.
 </ul>
 
 Log file names may include "strftime" escapes to have date based
 log file names.  These escapes include but are not limited to:<br>
 
 %Y -- 4 digit year<br>
 %y -- 2 digit year<br>
 %m -- 2 digit month<br>
 %d -- 2 digit day<br>
 %H -- 2 digit 24-hour time of day<br>
 %M -- 2 digit minute of hour<br>
 %S -- 2 digit second in the hour<br>
 %a -- Abbreviated weekday name<br>
 %b -- Abbreviated month name<br>
 %U -- Week of year (00-53)<br>
 %j -- Day of year (001-366)<br>

 example: new log file every day with 4 digit year<br>
 <pre>
 /var/log/safmq.%Y%m%d.log

 c:\logs\safmq_%Y%m%d.log
 </pre>
 example: new log file monthly<br>
 <pre>
 /var/log/safmq.%Y.%b.log

 c:\logs\safmq_%Y_%b.log
 </pre>

 <p>Using the special names NONE, STDOUT, STDERR or SYSTEM_LOG causes log
 data to be written to the special loations listed above.</p>


@memo safmq.conf
*/

/**
<pre>
safmqc usage:

safmq [options] <command> <command parameters>
   Options:
        --help,-h           : Displays this screen
        --user=login        : Sets the login user to 'login'
        --passwd=passwd     : Sets the login passwd to 'passwd'
        --body              : Flags safmq to retrieve the message body
        --timeout=seconds   : Sets the timeout to 'seconds' -1 for infinite
        --ttl=seconds       : Sets the message time to live to 'seconds'
        --ttlerr            : Flags ttl errors to be returned on 'response'
        --label=lbl         : Sets the message label to 'lbl'
        --rq=response       : Sets the response queue to 'response'
        --rid=id            : Sets the recipt id to 'id'
        --priority=num      : Sets the priority to 'num' in [0..5]
        --key=file.pem      : Sets 'file.pem' as private key pem file
        --cert=file.pem     : Sets 'file.pem' as certificate pem file

   Commands:
        --enqueue URL           : Enqueues a message
        --retrieve URL          : Retrieves a message
        --retrieveid URL ID     : Retrieves a message by ID
        --peekfront URL         : Peeks the first message
        --peekid URL ID         : Peeks a message reffered by ID
        --peekoffset URL OFFSET : Peeks a message OFFSET from front

        --enumeratequeues URL : Lists all queues
        --enumerateusers URL  : Lists all users
        --enumerategroups URL : Lists all groups

   User Management Commands:
        --createuser URL USERNAME passwd DESCRIPTION
        --deleteuser URL USERNAME
        --usersetpermissions URL USERNAME [users] [groups] [queues]
        --usergetpermissions URL USERNAME
        --setpassword URL USERNAME passwd
        --usergetgroups URL USERNAME

   Group Management Commands:
        --creategroup URL GROUPNAME
        --deletegroup URL GROUPANME
        --groupsetpermissions URL GROUPNAME [users] [groups] [queues]
        --groupgetpermissions URL GROUPNAME
        --groupadduser URL GROUPNAME USERNAME
        --groupdeleteuser URL GROUPNAME USERNAME
        --groupgetusers URL GROUPNAME

   Queue Management Commands:
        --createqueue URL QUEUENAME
        --deletequeue URL QUEUENAME
        --queuesetuserpermission URL QUEUENAME USERNAME [read] [write] [destroy] [change]
        --queuedeleteuserpermission URL QUEUENAME USERNAME
        --queuesetgrouppermission URL QUEUENAME GROUPNAME [read] [write] [destroy] [change]
        --queuedeletegrouppermission URL QUEUENAME GROUPNAME
        --queueenumeratepermissions URL QUEUENAME

URL Format:  safmq://user:passwd@host:port/queue
             safmqs://user:passwd@host:port/queue -- For SSL Connections

Note: queue, port, user and passwd are optional, if not supplied
      defaults will be used and the User name will be gathered
      from the system.


</pre>

@memo safmqc Usage
*/

/**
<pre>
safmq usage:
safmq 
       -h,--help      : Show this information
       -c cfg,
       --config=cfg   : Use 'cfg' as the configuration file
       -D,--daemon    : Start the program in the background
</pre>

@memo safmq for unix
*/

/**
<pre>
safmq usage:
safmq
       -h,--help      : Show this information
       -c cfg,
       --config=cfg   : Use 'cfg' as the configuration file
       -D,--daemon    : Start the program in the background
                        fails if SAFMQ has not been installed as a service

       -i,--install   : Installs SAFMQ as a service
       -r,--remove    : Uninstalls SAFMQ as a service
</pre>
@memo safmq for Windows
*/

/**
The environment variable SAFMQ_CFG can be used to specify an alternate location
and/or name for the safmq configuration file. If the file specifed by SAFMQ_CFG
or via the command line cannot be found, safmq attempts to locate the
file in either "/etc" in case of a unix system or in the "%SystemRoot%" directory
aka. the "widnows" directory.

@memo SAFMQ_CFG environment variable
*/

/**
Under default installations, SAFMQ's security is set up in an open state.  The
following resources should be secured to avoid server reconfiguration: the
safmq.conf configuration file, the "admin" user, the queues directory, the confi
directory, the SSL resources, and the SAFMQ server should be run under a limited
access user ID.

<ul>
<li>Limit Access to the safmq.conf configuration file to system administrators.

<li>Create a special user login for the SAFMQ application to run under.

<li>Limit Access to the "queues" directory to only the special SAFMQ user.

<li>Limit Access to the "config" directory to only the special SAFMQ user,
and optionally system administrators.

<li>By default the "admin" account does not have a password, assign a strong
password to this account, or create a new user with full privileges and a
strong password and then remove the admin user.

<li>If using SSL, limit access to the Certificate, Private Key and Certificate
Authority files to only the SAFMQ user, and optionally system administrators.
</ul>

@memo Securing SAFMQ installations
*/

/**
SAFMQ provides the ability to securely log into the SAFMQ server with out 
specifying a user id or password.  This feature is only available when 
utilizing SAFMQ over SSL (safmqs), and assigning an X509 identity to a user 
account in the SAFMQ server instance.  Items needed to perform this task 
are as follows:

<ul>
<li>SAFMQ built with SSL enabled
<li>A valid X509 certificate
</ul>

To successfully use passwordless authentication, perform the following steps:
<ol>
<li>Obtain a valid X509 certificate.
<li>Assign an X509 identity to a valid user account using the 
MQConnection::AddUserIdentity() method, or the SAFMQ manager Java GUI, 
specifying the Subject and Issuer distinguished names from the certificate 
listed above.
<li>Establish an SSLContext object specifying the Private Key and X509 
Digital Certificate from above.
<li>Utilize the SSLContext object to build an MQConnection or MessageQueue 
object using the MQFactory class.
</ol>

<p>Notes on password authentication:</p>
<ul>
<li>The User ID mapped to the Subject and Issuer from the X509 certificate 
will be automatically used as the security context when using passwordless authentication.
<li>The User ID is not required to be specified when connecting and authenticating
 using passwordless authentication.
<li>If a User ID is specified when establishing a connection, it must match the 
User ID mapped to the X509 certificate identity.  Otherwise, SAFMQ will default 
to password authentication, and a valid password must be sent to the server.
<li>Password Authentication is supported with SAFMQ over SSL.  This is the default 
behavior, if the X509 certificate identity is not mapped to a user, or if now X509 
certificate is supplied by the client.
</ul>

<p>Example using safmqc to perform passwordless authentication</p>
<pre>

safmqc --key=myCert.pem --cert=myKey.pem --enumeratequeues safmqs://:@localhost

</pre>

<p>Notes about the passwordless URL and safmqc:</p>
<ul>
<li>The URL specifies SAFMQ over SSL <span style="background: #CCCCCC"><b>safmqs://</b></span>:@localhost
<li>The URL specifies a blank user name and password safmqs://<b><span style="background: #CCCCCC">:@</span></b>localhost
</ul>


@memo Passwordless Login
*/


/**
<H3>Overview</H3>

<p>SAFMQ utilizes OpenSSL to provide SSL services.  OpenSSL provides SAFMQ two 
of the three legs of the digital security triad, authentication and 
confidentiality.  Specifically, SAFMQ relies on OpenSSL's API to authenticate
the validity of X509 certificates (authentication) and encrypt the data stream 
(confidentiality).  The third leg authorization, is performed by SAFMQ using 
either password authentication/authorization, or passwordless X509 identity 
authorization.</p>

<p>Confidentiality/encryption occurs automatically, however, authentication must 
be configured.  A standard OpenSSL installation includes a default OpenSSL 
configuration.  The default configuration specifies a Certificate Authority repository, 
which lists all trusted X509 certificate Issuers.  In most cases this default 
configuration will suffice.  Some organizations find it useful to issue their own 
certificates, skipping the use of a third-party Certificate Authority issuer.  In 
this case, administrators will need to either add the trusted certificate of the 
in-house authority to the default OpenSSL CA repository or create a custom repository 
for use with SAFMQ.  Both options are typical, and your organization's best practices 
should be evaluated to determine which best fits your needs.  SAFMQ may be configured 
to use a single file containing a list of trusted certificates, a directory of trusted 
certificates, or both.</p>

<p>SAFMQ's authorization occurs via X509 identity mapping.  From SAFMQ's point of view, 
an X509 identity is the pair of Subject Distinguished Name and Issuer Distinguished 
Name found in an X509 certificate presented by a client to the SAFMQ server.  SAFMQ 
contains a database of X509 identities associated with SAFMQ user names.  If an X509 
certificate is presented, and the identity from that certificate is mapped to a user, 
SAFMQ authorizes the connection using the user name's security context (user's permissions 
combined with permissions from each group the user is associated with).  One thing to note: 
if a user name is sent in combination with an X509 digital certificate, it must match 
the mapping in the X509 identity database.  Otherwise, a valid password must also be sent.</p>

<H3>Certificate Authority File</H3>

<p>SAFMQ can utilize OpenSSL's file based Certificate Authority.  This feature lists 
multiple trusted certificates, in PEM format, in a single text file. </p>

<p>From OpenSSL's manual on CA locations (man SSL_CTX_load_verity_locations):</p>

<blockquote>
<p>If <b>CAfile</b> is not NULL, it points to a file of CA certificates in PEM format. 
The file can contain several CA certificates identified by</p>

<pre>
       -----BEGIN CERTIFICATE-----
       ... (CA certificate in base64 encoding) ...
       -----END CERTIFICATE-----
</pre>

<p>sequences. Before, between, and after the certificates text is allowed which can 
be used e.g. for descriptions of the certificates.</p>

<p>The <b>CAfile</b> is processed on execution of the SSL_CTX_load_verify_locations() 
function.</pre>
</pre>

</blockquote>

<p>This is to say that the Certificate Authority File is loaded only once upon 
SAFMQ's startup sequence.  Any changes to this file will not be recognized until 
the SAFMQ server is restarted.   Trusted certificates found in the CA File are 
cached for the lifetime of the SAFMQ server's process.  Higher performance can be 
found by placing certificates in this file, but runtime configurability is sacrificed.</p>

<p>A Certificate Authority File may be specified by the "ssl_ca" property in the 
<i>safmq.conf</i> configuration file.</p>

<H3>Certificate Authority Directory</H3>

<p>SAFMQ can utilize OpenSSL's directory based Certificate Authority.  Individual 
certificates reside in individual files.  The  files are read dynamically by the 
OpenSSL API when a certificate is presented.  For performance reasons, the files 
are named by a hash of the certificate's Subject Distinguished Name.   This allows 
OpenSSL to create a hash on the Issuer DN of the supplied certificate, and open the 
file containing the trusted certificate directly, instead of reading every certificate 
file in the directory.

<p>From OpenSSL's manual on CA locations (man SSL_CTX_load_verity_locations):</p>
<blockquote>

<p>If CApath is not NULL, it points to a directory containing CA certificates in PEM 
format. The files each contain one CA certificate. The files are looked up by the 
CA subject name hash value, which must hence be available.  If more than one CA 
certificate with the same name hash value exist, the extension must be different 
(e.g. 9d66eef0.0, 9d66eef0.1 etc). The search is performed in the ordering of the 
extension number, regardless of other properties of the certificates.  Use the c_rehash 
utility to create the necessary links.</p>
<p>The certificates in CApath are only looked up when required, e.g. when building 
the certificate chain or when actually performing the verification of a peer certificate.</p>

<p>When looking up CA certificates, the OpenSSL library will first search the certificates 
in CAfile, then those in CApath. Certificate matching is done based on the subject name, 
the key identifier (if present), and the serial number as taken from the certificate to 
be verified. If these data do not match, the next certificate will be tried. If a first 
certificate matching the parameters is found, the verification process will be performed;
no other certificates for the same parameters will be searched in case of failure.</p>
</blockquote>

<p>An example of how to generate a hash name:</p>
<pre>
$ openssl x509 -hash -noout -in sample_cert.pem
e9907475
</pre>

<p>To properly integrate with SAFMQ's use of OpenSSL, the sample_cert.pem should be copied to SAFMQ's CA Directory with the name "e9907475.0".  The sample_cert.pem certificate contains the Subject DN:  <i>C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net</i>

<p>A Certificate Authority Directory may be specified by the "ssl_ca_dir" property in the <i>safmq.conf</i> configuration file.</p>

@memo Configuring SAFMQ X509 Authentication
*/

/**
<p>SAFMQ uses the "User" paradigm to describe client access authentication and 
authorization.  When a client connects to SAFMQ, it presents its user name 
(sometimes called a user ID, login ID, or account), along with a password.  
The user name is well known in the SAFMQ system, where as the password is meant 
to be confidential between the SAFMQ server and the client.  Thus the password 
ensures the identity of the client presenting the user name as identification.

  <p>Alternately, a user name can be mapped to an X509 Digital Certificate's 
identity (See: <a href="PasswordlessLogin.html">Passwordless Login</a>).  Using 
X509 Identities to identify a client eliminates the need to send a password 
from the client to the server.  Additionally, the user name is not required, 
thus the X509 Digital Certificate can be used to fully identify a user account.

  <p>Each user account (identified by a user name) is granted or denied three 
permissions.  These permissions are Modify Queues, Modify Users, and Modify Groups.  
The Modify Queues permission allows that account to create Queues in the SAFMQ 
server.  The Modify Users permission allows the account to create and destroy 
other accounts as well as grant permissions to those accounts.  The Modify 
Groups permission allows the account to create and destroy Security Groups 
(xref).  These permissions are set via the SAFMQ API (and SAFMQ Manager) 
function <a href="safmq.MQConnection.html">MQConnection::UserSetPermissions()</a>.

  <p>Users may be placed in a Security Group (sometimes simply referred to as 
"Groups").  Since Groups may be granted the same permissions as users, this 
allows multiple accounts to be granted similar permissions.

  <p>On a Queue by Queue basis, User Accounts and Groups (referred to collectively 
as Actors) are granted permissions to Queues (See <a href="QueueAccessControl.html">
Queue Access Control</a>).  This enables individual Users (Accounts) or Groups to 
be granted access to a Queue.

@memo SAFMQ User Security Contexts
*/

/**
<p>SAFMQ uses the Security Group (sometimes referred to as Group(s)) paradigm to 
describe client authorization as applied to one or more User Account.  Thus allowing
multiple User Accounts (authentication contexts) to be granted similar permissions.

<p>A SAFMQ Security Group is a named entity controlled via the SAFMQ API (See 
<a href="safmq.MQConnection.html">MQConnection</a>).  Groups consist of one or 
more User Accounts and are grated or denied three permissions:  Modify Queues, 
Modify Users, and Modify Groups.  The Modify Queues permission allows that 
account to create Queues in the SAFMQ server.  The Modify Users permission 
allows the account to create and destroy other accounts as well as grant permissions 
to those accounts.  The Modify Groups permission allows the account to create and 
destroy Security Groups (xref).  These permissions are set via the SAFMQ API (and 
SAFMQ Manager) function <a href="safmq.MQConnection.html">MQConnection::GroupSetPermissions()</a>.

<p>On a Queue by Queue basis, Security Groups and User Accounts (referred to 
collectively as Actors) are granted permissions to Queues (See 
<a href="QueueAccessControl.html">Queue Access Control</a>).  This enables 
individual Users (Accounts) or Groups to be granted access to a Queue.

@memo SAFMQ Group Security Contexts
*/

/**
<p>SAFMQ maintains a list of Access Permissions for each Queue created.  Actors 
(User Accounts and Groups) are individually granted or denied the ability to Read 
from, Write To, Destroy or Change the Security of a Queue.  This is performed via 
the SAFMQ API (and SAFMQ Manager) <a href="safmq.MQConnection.html">MQConnection</a>.  
The Access Control List of permissions for a queue consists of positive rules.  If a 
User or Group is not named in the list it is assumed that the User or Group cannot 
access the Queue at all. 

<ul>
<li>Granting the Read permission allows Actors to retrieve data from the queue and 
remove messages from the queue.  

<li>Granting the Write permission allows Actors to publish data on a queue.

<li>Granting the Destroy permission allows Actors to remove the queue from the 
SAFMQ server (and delete its contents).

<li>Granting the Change Security permission allows Actors to configure the 
Access Control List of the queue.
</ul>

<p>By default the creator of the Queue receives full permissions to the 
Queue upon creation.

<p>See Also:
<p><a href="SAFMQGroupSecurityContexts.html">SAFMQ Group Security Contexts</a>
<p><a href="SAFMQUserSecurityContexts.html">SAFMQ User Security Contexts</a>

@memo Queue Access Control
*/

/**
<P CLASS=dppSubHeading>UNIX/BSD/Cygwin Compilation</P>
<p>Compilation of SAFMQ is supported in UNIX and BSD like
environments using gmake &amp; gcc versions 3 and 4.   Makefiles have been
supplied to automatically build all components, unless the system is missing
key resources.  SAFMQ is dependent on OpenSSL for SSL capabilities and on PHP
version 5 for the PHP library.  A "configure" script supplied with the
distribution will detect the existence of OpenSSL and PHP on the system and
tune the make process to build available features.  Issuing the configure
command with the --help option produces:

<pre>
$ ./configure --help

SAFMQ
Configureation utility usage:


./configure [options]


Options:

  
--help,-h                       : Show this page
  
--without-ssl                   : Disables SSL integration
--ssl-location=&lt;dir&gt;            : Location of SSL libraries if not in a standard location
--with-custom-getopt            : Force use custom comand line processing
  
--with-include-dir=&lt;dir&gt;        : Add a directory to the include path, may be repeated
--prefix=&lt;dir&gt;                  : Set the install base, defaults to /usr/local
--bin-prefix=&lt;dir&gt;              : Set the install binary location, defaults to [prefix]/bin
--lib-prefix=&lt;dir&gt;              : Set the install libary location, defaults to [prefix]/lib
--include-prefix=&lt;dir&gt;          : Set the install header location, defaults to [prefix]/include
--config-dir=&lt;dir&gt;              : Set the location of the safmq.conf file defaults to /etc
--with-fPIC                     : Add the -fPIC flag to the code compliation (needed on some systems)
--with-compile-option=&lt;option&gt;  : Add pass the listed flag to the compiler stage (may be repeated)
--with-link-option=&lt;option&gt;     : Add pass the listed flag to the linker (may be repeated)
</pre>

<P CLASS=dppSubHeading>Compilation on Windows</P>
<p>Compilation can also occur on Windows systems.  Visual C++
6.0 workspace and project files have been supplied as well as Visual Studio C++
2005.

<P CLASS=dppSubHeading>SSL Enabled Clients</P>
<p>SAFMQ Clients that wish to use SSL must define the compiler
macro SAFMQ_SSL.  This macro causes code in the header files to expose features
specific to the SSL implementation.  If the library being used is SSL enabled,
the client must define the SAFMQ_SSL macro, otherwise compilation and linking
errors may occur.

<P CLASS=dppSubHeading>Client Applications In Windows</P>
<p>The binary Windows distribution is distributed with static
libraries for both Visual C++ 6.0 and Visual Studio C++ 2005.  Subtle
differences exist between the two versions, so picking the correct library is
necessary.  Versions have been supplied for debug or release modes; with or
without SSL; and with standard libraries statically linked (minimum
dependencies) or dynamically linked (smaller binary). This is a total of eight
(8) library versions per compiler environment.   All packaged versions of the
SAFMQ client library on windows are built with the Multi Threaded version of the
standard libraries.
<br>
<br>
<table border="1" cellspacing="0" cellpadding="2" style="font-size:9pt" >
 <tr>
  <td><b>Library</b> </td>
  <td><b>Runtime Lib</b> </td>
  <td><b>SSL</b> </td>
  <td><b>Compiler</b> </td>
 </tr>
 <tr>
  <td>safmq.lib </td>
  <td>multi-thread DLL </td>
  <td>NO </td>
  <td>VC++ 6.0 </td>
 </tr>
 <tr>
  <td>safmq-ssl.lib </td>
  <td>multi-thread DLL </td>
  <td>YES (-DSAFMQ_SSL) </td>
  <td>VC++ 6.0 </td>
 </tr>
 <tr>
  <td>safmq-mindep.lib </td>
  <td>multi-thread static lib </td>
  <td>NO </td>
  <td>VC++ 6.0 </td>
 </tr>
 <tr>
  <td>safmq-ssl-mindep.lib </td>
  <td>multi-thread static lib </td>
  <td>YES (-DSAFMQ_SSL) </td>
  <td>VC++ 6.0 </td>
 </tr>
 <tr>
  <td>safmq2005.lib </td>
  <td>multi-thread DLL </td>
  <td>NO </td>
  <td>VS2005 C++ </td>
 </tr>
 <tr>
  <td>safmq-ssl2005.lib </td>
  <td>multi-thread DLL </td>
  <td>YES (-DSAFMQ_SSL) </td>
  <td>VS2005 C++ </td>
 </tr>
 <tr>
  <td>safmq-mindep2005.lib </td>
  <td>multi-thread static lib </td>
  <td>NO </td>
  <td>VS2005 C++ </td>
 </tr>
 <tr>
  <td>safmq-ssl-mindep2005.lib </td>
  <td>multi-thread static lib </td>
  <td>YES (-DSAFMQ_SSL) </td>
  <td>VS2005 C++ </td>
 </tr>
</table>

<p>If the client application being developed is using an
inappropriate version of the SAFMQ client library, linking errors can occur. 
The most commonly received link error is that a symbol is already defined in a
standard library like msvcprtd.lib.

@memo Compilation Issues
*/

///
