package com.safmq;

/**
 * Represents an identity presented by an X509 Digital Certificate
 * 
 * @author matt
 */
public class X509Identity {
	protected String subjectDN;
	protected String issuerDN;
	
	/**
	 * Constructs the identity from a subject distinguished name and and issuer distinguished name.
	 * 
	 * @param subjectDN The Subject Distinguished Name
	 * @param issuerDN The Issuer Distinguished Name
	 */
	public X509Identity(String subjectDN, String issuerDN) {
		this.subjectDN = subjectDN;
		this.issuerDN = issuerDN;
	}

	public boolean equals(Object obj) {
		return (obj instanceof X509Identity)
			&& ((X509Identity)obj).subjectDN.equals(subjectDN)
			&& ((X509Identity)obj).issuerDN.equals(issuerDN);
	}

	/**
	 * Provides the Issuer DN
	 * @return The Issuer DN
	 */
	public String getIssuerDN() {
		return issuerDN;
	}

	/**
	 * Sets the issuer DN
	 * @param issuerDN The new value for the issuer DN
	 */
	public void setIssuerDN(String issuerDN) {
		this.issuerDN = issuerDN;
	}

	/**
	 * Provides the Subject DN
	 * @return The Subject DN
	 */
	public String getSubjectDN() {
		return subjectDN;
	}

	/**
	 * Sets the Subject DN
	 * @param subjectDN The new value for the Subject DN
	 */
	public void setSubjectDN(String subjectDN) {
		this.subjectDN = subjectDN;
	}
	
	
}
