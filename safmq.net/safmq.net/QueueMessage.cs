/*
 Copyright 2005-2007 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except input compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to input writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.


Created on Aug 13, 2007
*/
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace safmq.net {
    /**
     * <summary>
     * Container of a Guid
     * </summary>
     */
    public class UUID {
        internal Guid guid = Guid.Empty;

        /// <summary>
        /// Default constructor.
        /// </summary>
        public UUID() { }

        /// <summary>
        /// Constructs from a supplied Guid
        /// </summary>
        /// <param name="guid">Associates this <c>UUID</c> with the passed <c>Guid</c></param>
        public UUID(Guid guid) {
            this.guid = guid;
        }

        /// <summary>
        /// Compares this object equal to another.
        /// </summary>
        /// <param name="o">The object to compare to this.</param>
        /// <returns></returns>
        public override bool Equals(object o) {
            return o is UUID && ((UUID)o).guid.Equals(guid);
        }

        /// <summary>
        /// Returns the hash code by calling the base
        /// </summary>
        /// <returns>The base hash code</returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }

        internal void Read(BinaryReader input) {
            guid = new Guid(input.ReadBytes(16));
        }

        internal void Write(BinaryWriter output) {
            byte[] b = guid.ToByteArray();
            output.Write(b);
        }

        /// <summary>
        /// Allows the <c>UUID</c> to be cast to a <c>Guid</c>
        /// </summary>
        /// <param name="uuid">An instance of a <c>UUID</c></param>
        /// <returns>The <c>Guid</c> contained in the <c>UUID</c> instance </returns>
        public static implicit operator Guid(UUID uuid) {
            return uuid.guid;
        }

        internal int getSize() {
            return 16;
        }   
    }

    /**
     * <summary>
     * Represents a message to be sent or retrieved from a SAFMQ message queue
     * server.
     * </summary>
     * 
     * <remarks>
     * <para>In the simplest use of the <c>QueueMessage</c> object, clients
     * would only need to instantiate an instance then write to the body represented
     * by a <c>Stream</c> then the message could be sent to the server.</para>
     * 
     * <para>
     * Facilitating two way communications, the <c>QueueMessage</c> class
     * provides a facility via a "Recipt ID" input which responding programs can place
     * the original message's id into the recipt id of the response so that the
     * querier can wait for a response.  Additionaly, SAFMQ provides the ability to
     * prescribe a "Time-To-Live" for a message query so that input the case that
     * the responding program is not able to retrieve the message and respond input time,
     * the caller can be notified by the SAFMQ server.</para>
     * 
     * @author Matt
     * </remarks>
     * 
     * <example>
     * <para>
     * The following is a simple client which sends a message to a queue demonstrating the use
     * of a stream to manage the body of the message
     * </para>
     * <code>
     * MessageQueue	queue = MQBuilder.buildMessageQueue("safmq://localhost/foo","user","password");
     * QueueMessage	msg = new QueueMessage();
     * StreamWriter	w = new StreamWriter(msg.Stream);
     * 
     * w.write("Hello World!");
     * w.flush();
     * queue.Enqueue(msg);
     * queue.Close();
     * </code>
     * 
     * 
     * <para>An example of two-way communictions using sudo-synchronous communications.</para>
     * 
     * <para>The Client:</para>
     * <code>
     * MessageQueue	queue = MQBuilder.buildMessageQueue("safmq://localhost/foo","user","password");
     * QueueMessage	msg = new QueueMessage();
     * StreamWriter	w = new StreamWriter(msg.Stream);
     * 
     * w.Write("Hello World!");
     * w.Flush();
     * 
     * msg.Lable = "Query";
     * msg.TimeToLiveSeconds = 5; // allow 5 seconds before an auto response
     * msg.TTLErrorWanted = true;
     * msg.ResponseQueueName = "safmq://localhost/foo";
     * if (queue.Enqueue(msg) == ErrorCode.EC_NOERROR) {
     * 	UUID id = msg.MessageID; // generated via the call to Enqueue()
     * 	msg = new QueueMessage();
     * 	if (queue.RetrieveID(true,id,-1,msg) == Safmq.EC_NOERROR) {
     * 		if (msg.MessageClass == MessageClassEnum.MC_SYSTEMERRMSG) {
     * 			Console.WriteLine("The message errored output");
     * 		}
     * 		Stream      input = msg.Stream;
     * 		byte[]		data = new byte[1024];
     * 		int			read;
     * 		while ( (read=input.read(data)) > 0) {
     * 			Console.Out.Write(Encoding.UTF8.GetChars(data, 0, read))
     * 		}
     * 	}
     * }
     * queue.close();
     * </code> 
     * 
     * <para>The Server:</para>
     * <code>
     * MessageQueue	queue = MQBuilder.buildMessageQueue("safmq://localhost/foo","user","password");
     * QueueMessage	msg = new QueueMessage();
     * 
     * while ( queue.Retrieve(true,-1,msg) == Safmq.EC_NOERROR ) {
     *     QueueMessage response = new QueueMessage();
     *     StreamWriter	w = new StreamWriter(response.Stream);
     * 
     *     w.write("Back at ya!");
     *     w.flush();
     * 
     *     response.setReciptID(msg.getMesasgeID());
     *     response.setLabel("Response");
     * 
     *     MessageQueue	responseQueue = MQBuilder.buildMessageQueue(msg.getResponseQueueName(),"user","password");
     * 
     *     responseQueue.Enqueue(response);
     * 
     *     responseQueue.Close();
     *     msg = new QueueMessage();
     * }
     * queue.Close();
     * </code>
     * </example>
     */
    public class QueueMessage {
        //private static final UUID NUL_UUID = new UUID();
    	
	    private UUID	            messageID			= new UUID();
        private MessageClassEnum    messageClass        = MessageClassEnum.MC_USERMSG;
        private MessagePriorityEnum messagePriority     = MessagePriorityEnum.MP_STANDARD;
        private string              label               = "";
        private int                 timeStamp           = 0;
        private int                 timeToLiveSeconds   = -1;
        private byte                ttlErrorWanted      = 0;
        private string              responseQueueName   = "";
        private UUID                reciptID            = new UUID();
        private BodyTypeEnum        bodyType            = BodyTypeEnum.BT_NONE;
        private int                 bodySize            = -1;
        private MemoryStream        body                = new MemoryStream();



        /**
         * <summary>
         * Default Constructor.
         * </summary>
         */
        public QueueMessage() {
	    }
    	
	    /*
	     * <summary>
         * Determines the size of the QueueMessage when written to a
	     * stream.
         * </summary>
	     * 
	     * <returns>The size input bytes of the object when written to a stream.</returns>
	     */
	    internal int getSize() {
		    return 	messageID.getSize() + 
			    1 + 				//byte	messageClass
			    1 + 				//byte	messagePriority
			    Safmq.MSGLBL_LENGTH + 
			    Safmq.SIZE_INT + 	//int		timeStamp			= 9;
			    Safmq.SIZE_INT + 	// int		timeToLiveSeconds	= -1;
			    1 + 				//byte	ttlErrorWanted		= 0;
			    Safmq.QNAME_LENGTH +
			    reciptID.getSize() + 
			    1 + 				//byte	bodyType			= Safmq.BT_NONE;
			    Safmq.SIZE_INT + 	//int		bodySize			= -1;
                body.GetBuffer().Length;
	    }
    	
	    /*
	     * <summary>
         * Package protected member to write the message to a DataOutput stream.
         * </summary>
	     * 
	     * <param name="output">The stream to which the data is to be written</param>
	     * 
         * <exception cref="Exception">
         * In the case that the stream experiences an error while writing data.</exception>
	     */
	    internal void Write(BinaryWriter output) {
		    messageID.Write(output);
		    output.Write((byte)messageClass);
		    output.Write((byte)messagePriority);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(label.ToCharArray()), Safmq.MSGLBL_LENGTH, 0);
		    output.Write(timeStamp);
		    output.Write(timeToLiveSeconds);
		    output.Write((byte)ttlErrorWanted);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(responseQueueName.ToCharArray()), Safmq.QNAME_LENGTH, 0);
		    reciptID.Write(output);
		    output.Write((byte)bodyType);

            byte[] buffer = body.GetBuffer();
            bodySize = buffer.Length;
		    output.Write(bodySize);
		    output.Write(buffer);
	    }

        /**
         * <summary>
         * Package protected member to read the mesage from a DataOutput stream.
         * </summary>
         * 
         * <param name="input">			The stream from which the data is to be read</param>
         * <param name="retrievebody">  A flag indicating the body of the message should
         * 							    be retrieved.</param>
         * 
         * 
         * <exception cref="Exception"> In the case that the stream experiences an error
         *      						while reading data.</exception>
         */
        internal void Read(BinaryReader input, bool retrievebody) {
            // Make sure that if this object is being reused we don't over write the UUID
		    (messageID = new UUID()).Read(input);
		    messageClass = (MessageClassEnum)input.ReadByte();
		    messagePriority = (MessagePriorityEnum)input.ReadByte();

            byte[] tmp = input.ReadBytes(Safmq.MSGLBL_LENGTH);
            label = new string(Encoding.UTF8.GetChars(tmp, 0, ioutil.length(tmp)));

            timeStamp = input.ReadInt32();
		    timeToLiveSeconds = input.ReadInt32();
		    ttlErrorWanted = input.ReadByte();

            tmp = input.ReadBytes(Safmq.QNAME_LENGTH);
            this.responseQueueName = new string(Encoding.UTF8.GetChars(tmp, 0, ioutil.length(tmp)));

            // Make sure that if this object is being reused we don't over write the UUID
            (reciptID = new UUID()).Read(input);

		    bodyType = (BodyTypeEnum)input.ReadByte();
		    bodySize = input.ReadInt32();

            if (bodySize > 0)
                body = new MemoryStream(input.ReadBytes(bodySize));
	    }

        /**
         * <summary>
         * Provides access to the message id, this value is generated by a successful
         * call to <c>MQConnection.Enqueue(QueueHandle,QueueMessage)</c>.
         * </summary>
         */
        public UUID MessageID {
            get { return messageID; }
            internal set { messageID = value; }
	    }

        /**
         * <summary>
         * Provides access to the size of the message body.
         * </summary>
         */
        public int BodySize { get { return (bodySize == -1) ? BufferSize : bodySize; } }

        /**
         * <summary>
         * Provides the size of the buffer allocated.
         * </summary>
         */
        public int BufferSize { get { return body.GetBuffer().Length; } }

        /**
         * <summary>
         * Provides an input stream to read the data input the message's body.
         * </summary>
         */
        public Stream Stream { get { return body; } }

        /**
         * <summary>
         * The body type of the message.
         * </summary>
         */
        public BodyTypeEnum BodyType {
            get { return bodyType; }
            set { bodyType = value; }
	    }

        /**
         * <summary>
         * The label of the message.
         * </summary>
         */
        public string Label {
            get { return label; }
            set { label = value; }
	    }

        /**
         * <summary>
         * Provides the message's priority
         * </summary>
         * 
         * <seealso cref="MessagePriorityEnum"/>
         */
        public MessagePriorityEnum MessagePriority {
            get { return messagePriority; }
            set { messagePriority = value; }
	    }

        /**
         * <summary>
         * Provides the recipt id of this message
         * </summary>
         */
        public UUID ReciptID {
            get { return reciptID; }
            set { reciptID = value; }
	    }

        /**
         * <summary>
         * The response queue name for this message
         * </summary>
         */
        public string ResponseQueueName {
            get { return responseQueueName; }
            set { responseQueueName = value; }
	    }

        /**
         * <summary>
         * Provides the timestamp for this message.
         * </summary>
         * <returns> Returns the time stamp. </returns>
         */
        public int TimeStamp {
            get { return timeStamp; }
            internal set { timeStamp = value; }
	    }

        /**
         * <summary>
         * The time to live seconds for this message.
         * </summary>
         */
        public int TimeToLiveSeconds {
            get { return timeToLiveSeconds; }
            set { timeToLiveSeconds = value; }
	    }

        /**
         * <summary>
         * Provids the flag indicating this message desires auto generated TTL messages.
         * </summary>
         */
        public bool TTLErrorWanted {
            get { return ttlErrorWanted != 0; }
            set { ttlErrorWanted = (byte)(value ? 1 : 0); }
	    }

        /**
         * <summary>
         * Provides the message class
         * </summary>
         * <returns> Returns the message class.</returns>
         * 
         * <seealso cref="MessageClass"/>
         */
        public MessageClassEnum MessageClass {
            get { return messageClass; }
	    }
    }
}
