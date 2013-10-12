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


Created on Aug 9, 2007

*/
using System;
using System.Collections.Generic;
using System.Text;

namespace safmq.net
{

    /**
     * <summary>
     * Exception thrown in cases where the SAFMQ queue server has responded
     * with an error code from within a constructor.<br/>
     * </summary>
     * <remarks>
     * @author Matt
     * </remarks>
     */
    public class MQException : System.Exception {
	    ErrorCode errorcode;

        /**
         * <summary>
         * Constructs the MQException from a description message and error code.
         * </summary>
         * 
         * <param name="msg">		The description message</param>
         * <param name="errorcode">	The error code, typically defined in the class Safmq.</param>
         */
        public MQException(string msg, ErrorCode errorcode) : base(msg) {
		    this.errorcode = errorcode;
	    }
    	
	    /**
         * <summary>
         * Provides the error code associated with the exception.
         * </summary>
         * 
         * <returns>The error code associated with the exception.</returns>
         * 
         * <seealso cref="ErrorCode"/>
	     */
	    public ErrorCode getErrorcode() {
		    return errorcode;
	    }
    }
}
