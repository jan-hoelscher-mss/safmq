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




This software implements a platform independent Store and Forward Message Queue.
*/
#if !defined (_PARAM_CONFIG_H_)
#define _PARAM_CONFIG_H_

#pragma warning(disable:4290) // Disable warning about exception specifications

#include <string>
#include <map>

/**
 * A class which loads configuraiton data.  The parameter file has the
 * format:<br>
 *
 * <pre>
 * key:value
 * </pre>
 */
class Config {
public:
	/**
	 * Constructor.  Loads the configuraiton information
	 *
	 * @exception int In the case the parameter file cannot be read.
	 */
	Config(const std::string& cfg) throw (int);


	/**
	 * Retrieves a named parameter
	 * @param param The name of the paramter to retrieve
	 * @param def The default valueto return
	 * @return The value of the parameter
	 */
	const std::string& getParam(const std::string& param, const std::string& def="") const;

	/**
	 * Retrieves a named parameter
	 * @param param The name of the paramter to retrieve
	 * @param def The default valueto return
	 * @return The value of the parameter
	 */
	int getIntParam(const std::string& param, int def=0) const;

protected:
	std::map<std::string, std::string>	m_params;
};
#endif

