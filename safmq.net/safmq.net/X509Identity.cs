using System;
using System.Collections.Generic;
using System.Text;

namespace safmq.net
{
	/// <summary>
	/// Represents an X509 based identity
	/// </summary>
	public class X509Identity
	{
		string subjectDN;
		string issuerDN;

		/// <summary>
		/// The X509 Subject Distinguished Name
		/// </summary>
		public string SubjectDN { get { return subjectDN; } set { subjectDN = value; } }
		/// <summary>
		/// THe X509 Issuer Distinguished Name
		/// </summary>
		public string IssuerDN { get { return issuerDN; } set { issuerDN = value; } }

		/**
		 * <summary>
		 * Constructs the identity from a subject distinguished name and and issuer distinguished name.
		 * </summary>
		 * 
		 * <param name="subjectDN">The Subject Distinguished Name</param>
		 * <param name="issuerDN">The Issuer Distinguished Name</param>
		 */
		public X509Identity(string subjectDN, string issuerDN) {
			this.subjectDN = subjectDN;
			this.issuerDN = issuerDN;
		}
	}
}
