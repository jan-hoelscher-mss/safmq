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
*/
#include "StdAfx.h"
#include "SafmqSecurityLogin.h"

using namespace SafmqServiceInstaller;

System::Void SafmqSecurityLogin::SafmqSecurityLogin_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	if (!Password->Text->Equals(Password2->Text)) {
		MessageBox::Show("The passwords entered do not match.\n\nPlease reenter the password values","SAFMQ Login Setup", MessageBoxButtons::OK, MessageBoxIcon::Hand);
		e->Cancel = true;
	}
}

System::Void SafmqSecurityLogin::LocalSystem_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	UserName->ReadOnly = LocalSystem->Checked;
	Password->ReadOnly = LocalSystem->Checked;
	Password2->ReadOnly = LocalSystem->Checked;
}

