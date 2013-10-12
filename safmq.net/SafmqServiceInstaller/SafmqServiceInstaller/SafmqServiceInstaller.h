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
#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Configuration::Install;
using namespace System::Windows::Forms;

namespace SafmqServiceInstaller {

	[RunInstaller(true)]

	/// <summary>
	/// Summary for SafmqServiceInstaller
	/// </summary>
	public ref class SafmqServiceInstaller : public System::Configuration::Install::Installer
	{
	public:
		static String^ exeLocation = "bin\\safmq.exe";
		static String^ serviceName = "safmq";
		static String^ serviceDesc = "SAFMQ Message Queue Server";
		static String^ description = "Allows applications to post a message, to a message queue, which is stored until the message is read by the receiving application.";


	public:
		SafmqServiceInstaller(void)
		{
			InitializeComponent();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SafmqServiceInstaller()
		{
			if (components)
			{
				delete components;
			}
		}

	public:
		virtual void Install(IDictionary^ savedState) override;
		virtual void Uninstall(IDictionary^ savedState) override;

		virtual void Commit(IDictionary^ savedState) override {
			Installer::Commit(savedState);
		}
		virtual void Rollback(IDictionary^ savedState) override {
			Installer::Rollback(savedState);
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
#if _DEBUG
			MessageBox::Show("Stopped for Debugging", "DEBUG", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
#endif
		}
#pragma endregion
	};
}
