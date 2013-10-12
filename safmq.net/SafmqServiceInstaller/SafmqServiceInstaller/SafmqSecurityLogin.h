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
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace SafmqServiceInstaller {

	/// <summary>
	/// Summary for SafmqSecurityLogin
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SafmqSecurityLogin : public System::Windows::Forms::Form
	{
	public:
		SafmqSecurityLogin(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}


		String^ GetUserName() {
			return UserName->Text;
		}

		String^ GetPassword() {
			return Password->Text;
		}

		bool UseLocalSystem() {
			return LocalSystem->Checked;
		}
	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SafmqSecurityLogin()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::RadioButton^  LocalSystem;
	private: System::Windows::Forms::RadioButton^  UserAccount;
	private: System::Windows::Forms::TextBox^  UserName;
	private: System::Windows::Forms::TextBox^  Password;
	protected: 

	protected: 

	protected: 



	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Button^  OK;
	private: System::Windows::Forms::Button^  Cancel;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::TextBox^  Password2;


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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(SafmqSecurityLogin::typeid));
			this->LocalSystem = (gcnew System::Windows::Forms::RadioButton());
			this->UserAccount = (gcnew System::Windows::Forms::RadioButton());
			this->UserName = (gcnew System::Windows::Forms::TextBox());
			this->Password = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->OK = (gcnew System::Windows::Forms::Button());
			this->Cancel = (gcnew System::Windows::Forms::Button());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->Password2 = (gcnew System::Windows::Forms::TextBox());
			this->groupBox1->SuspendLayout();
			this->SuspendLayout();
			// 
			// LocalSystem
			// 
			this->LocalSystem->AutoSize = true;
			this->LocalSystem->Location = System::Drawing::Point(48, 45);
			this->LocalSystem->Name = L"LocalSystem";
			this->LocalSystem->Size = System::Drawing::Size(88, 17);
			this->LocalSystem->TabIndex = 0;
			this->LocalSystem->TabStop = true;
			this->LocalSystem->Text = L"&Local System";
			this->LocalSystem->UseVisualStyleBackColor = true;
			this->LocalSystem->CheckedChanged += gcnew System::EventHandler(this, &SafmqSecurityLogin::LocalSystem_CheckedChanged);
			// 
			// UserAccount
			// 
			this->UserAccount->AutoSize = true;
			this->UserAccount->Location = System::Drawing::Point(48, 69);
			this->UserAccount->Name = L"UserAccount";
			this->UserAccount->Size = System::Drawing::Size(90, 17);
			this->UserAccount->TabIndex = 1;
			this->UserAccount->TabStop = true;
			this->UserAccount->Text = L"&User Account";
			this->UserAccount->UseVisualStyleBackColor = true;
			// 
			// UserName
			// 
			this->UserName->Location = System::Drawing::Point(126, 74);
			this->UserName->Name = L"UserName";
			this->UserName->ReadOnly = true;
			this->UserName->Size = System::Drawing::Size(126, 20);
			this->UserName->TabIndex = 2;
			// 
			// Password
			// 
			this->Password->Font = (gcnew System::Drawing::Font(L"Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(2)));
			this->Password->Location = System::Drawing::Point(126, 101);
			this->Password->Name = L"Password";
			this->Password->PasswordChar = '·';
			this->Password->ReadOnly = true;
			this->Password->Size = System::Drawing::Size(126, 22);
			this->Password->TabIndex = 3;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(57, 77);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(63, 13);
			this->label1->TabIndex = 4;
			this->label1->Text = L"User Name:";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(64, 105);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(56, 13);
			this->label2->TabIndex = 5;
			this->label2->Text = L"Password:";
			// 
			// OK
			// 
			this->OK->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->OK->Location = System::Drawing::Point(124, 230);
			this->OK->Name = L"OK";
			this->OK->Size = System::Drawing::Size(75, 23);
			this->OK->TabIndex = 6;
			this->OK->Text = L"&OK";
			this->OK->UseVisualStyleBackColor = true;
			// 
			// Cancel
			// 
			this->Cancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Cancel->Location = System::Drawing::Point(205, 230);
			this->Cancel->Name = L"Cancel";
			this->Cancel->Size = System::Drawing::Size(75, 23);
			this->Cancel->TabIndex = 7;
			this->Cancel->Text = L"&Cancel";
			this->Cancel->UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->label3);
			this->groupBox1->Controls->Add(this->Password2);
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->UserName);
			this->groupBox1->Controls->Add(this->label1);
			this->groupBox1->Controls->Add(this->Password);
			this->groupBox1->Location = System::Drawing::Point(28, 23);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(297, 193);
			this->groupBox1->TabIndex = 8;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Startup Account";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(25, 135);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(95, 13);
			this->label3->TabIndex = 10;
			this->label3->Text = L"Password (repeat):";
			// 
			// Password2
			// 
			this->Password2->Font = (gcnew System::Drawing::Font(L"Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(2)));
			this->Password2->Location = System::Drawing::Point(126, 131);
			this->Password2->Name = L"Password2";
			this->Password2->PasswordChar = '·';
			this->Password2->ReadOnly = true;
			this->Password2->Size = System::Drawing::Size(126, 22);
			this->Password2->TabIndex = 9;
			// 
			// SafmqSecurityLogin
			// 
			this->AcceptButton = this->OK;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->Cancel;
			this->ClientSize = System::Drawing::Size(337, 265);
			this->Controls->Add(this->Cancel);
			this->Controls->Add(this->OK);
			this->Controls->Add(this->UserAccount);
			this->Controls->Add(this->LocalSystem);
			this->Controls->Add(this->groupBox1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"SafmqSecurityLogin";
			this->Text = L"SAFMQ Startup Account";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &SafmqSecurityLogin::SafmqSecurityLogin_FormClosing);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	
	private: System::Void SafmqSecurityLogin::LocalSystem_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	private: System::Void SafmqSecurityLogin_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
};
}
