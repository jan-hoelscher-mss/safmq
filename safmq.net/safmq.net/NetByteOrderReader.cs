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
using System.Net;

namespace safmq.net
{
    internal class NetByteOrderReader : BinaryReader
    {
        internal NetByteOrderReader(Stream stream) : base(stream) { }
        internal NetByteOrderReader(Stream stream, Encoding encoding) : base(stream, encoding) { }
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public override short ReadInt16()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt16());
        }
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public override int ReadInt32()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt32());
        }
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public override long ReadInt64()
        {
            return IPAddress.NetworkToHostOrder(base.ReadInt64());
        }
    }
}
