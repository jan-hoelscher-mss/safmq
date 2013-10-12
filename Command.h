#ifndef _SAFMQ_COMMAND_H_
#define _SAFMQ_COMMAND_H_

#include <iostream>

namespace safmq {
class ServiceThread;

/**
Base class for all commands.  This class represents the public
interface for all commands.
*/
class Command {
protected:
	ServiceThread*	service;

public:
	typedef Command* (* Allocator)(ServiceThread*);

	Command(ServiceThread* pservice) : service(pservice) {}
	virtual ~Command() {}

	/**
	@returns 0 on success and writeResponse() should be called, non-zero otherwise
	*/
	virtual int perform() = 0;
	virtual void readRequest(std::istream& o) { }
	virtual void writeResponse(std::ostream& o) { }
};

class CommandFactory {
protected:
	CommandFactory() {}
public:
	static CommandFactory* getFactory(int version);
	virtual Command* build(int commandID, ServiceThread* service) = 0;
};



//class LoginCmd : public Command {
//protected:
//	LOGIN_PARAMS	params;
//	ErrorCode		response;
//public:
//
//	LoginCmd(ServiceThread* pservice) : Command(pservice), response(EC_ERROR) {	}
//	static Command* allocate(ServiceThread* pservice) { return new LoginCmd(pservice); }
//
//	void readRequest(std::istream& o) {
//		o >> params;
//	}
//
//	void writeResponse(std::ostream& o) {
//		o << RESPONSE_RESPONSE(response);
//		if (response == EC_NOERROR)
//			o << LOGIN_RESPONSE();
//		o << flush;
//	}
//
//	virtual int perform() {
//		std::string	tmpname = std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str();
//		std::string	password = std::string(params.password, SAFMQ_PASSWORD_LENGTH).c_str();
//
//		if (service->isTrusted()) {
//			// for trusted certificate, if the name was sent and does not match the mapped name
//			// or if the password was sent and the names match, apply loggin
//			if (tmpname.empty() || (tmpname == service->getUsername() && password.empty()) ) {
//				// loged in via digital certificate
//				service->setLogged_in(true);
//				Log::getLog()->SignOn(Log::none, service->getUsername().c_str(), service->getPeername());
//				response = EC_NOERROR;
//				return 0;
//			}
//		}
//
//		service->setUsername(tmpname);
//
//		if (params.major_version > SAFMQ_PROTOCOL_MAJOR_VERSION) {
//			response = EC_UNSUPPORTED_PROTOCOL;
//			Log::getLog()->SignOn(Log::error, service->getUsername().c_str(), service->getPeername(), EC_Decode(EC_UNSUPPORTED_PROTOCOL));
//		} else {
//
//			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->verifyPassword(service->getUsername(), password);
//			if (res == SecurityControl::GRANTED) {
//				service->setLogged_in(true);
//				response = EC_NOERROR;
//				Log::getLog()->SignOn(Log::none, service->getUsername().c_str(), service->getPeername());
//			} else {
//				response = EC_NOTAUTHORIZED;
//				Log::getLog()->SignOn(Log::error, service->getUsername().c_str(), service->getPeername(), EC_Decode(EC_NOTAUTHORIZED));
//			}
//		}
//		return 0;
//	}
//};


}


#endif
