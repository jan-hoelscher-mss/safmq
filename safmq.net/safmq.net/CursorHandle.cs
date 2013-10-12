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


Created on Aut 13, 2007
*/

using System;
using System.Collections.Generic;
using System.Text;

namespace safmq.net {
    /// <summary>
    /// This mutable object represents a handle to a cursor.  Cursors allow the client to
    /// break the priority FIFO and scan a queue in sequential order.
    /// </summary>
    /// <remarks>
    /// @author Matt
    /// </remarks>
    public sealed class CursorHandle {
        /**
         * Contains the value of the handle.
         */
        internal int handle = 0;
    }
}
