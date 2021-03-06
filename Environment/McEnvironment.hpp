/*
 Copyright (c) 2012, Esteban Pellegrino
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MCENVIRONMENT_HPP_
#define MCENVIRONMENT_HPP_

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include <string>
#include <vector>
#include <map>

/* MC modules */
#include "McModule.hpp"

/* General Settings class */
#include "Settings/Settings.hpp"

/* Common stuff */
#include "../Common/Common.hpp"

/* Parser class */
#include "../Parser/Parser.hpp"

/* Materials */
#include "../Material/Materials.hpp"
/* Ace module */
#include "../Material/AceTable/AceModule.hpp"

/* Geometry */
#include "../Geometry/Geometry.hpp"

/* Source */
#include "../Transport/Source.hpp"

namespace Helios {

	/* Environment class, contains all the modules that conforms the MC problem */
	class McEnvironment {

	public:

		/* Constructor from a parser and command line argument */
		McEnvironment(int argc, char **argv, Parser* parser = 0);

		/* Constructor from a parser */
		McEnvironment(Parser* parser = 0);

		/* ---- Parser management */

		/* Set a parser */
		void setParser(Parser* new_parser) {parser = new_parser;}

		/*
		 * Parse file (thrown an exception if there isn't a parser) and push the definitions
		 * parsed. Thrown an exception if the parser fails.
		 */
		void parseFile(const std::string& input_file);

		/*
		 * Parse files (thrown an exception if there isn't a parser) and push the definitions
		 * parsed. Thrown an exception if the parser fails.
		 */
		void parseFiles(const std::vector<std::string>& input_files);

		/* ---- Objects management */

		/* Push a set of definitions into the environment. */
		template<class Iterator>
		void pushObjects(Iterator begin, Iterator end);

		/* Push a definition into the environment. */
		template<class Object>
		void pushObject(Object object);

		/* ---- Modules management */

		/*
		 * Create a module with the information that is loaded into the environment. The created
		 * module won't be loaded and is user responsibility to delete it after is done with it.
		 */
		template<class Module>
		Module* createModule(const std::vector<McObject*>& definitions);

		/*
		 * Get a module that should be loaded on the map. In case the module is not loaded, this
		 * will thrown an exception.
		 */
		template<class Module>
		Module* getModule() const;

		/*
		 * Check whether or not a module is loaded into the environment.
		 */
		template<class Module>
		bool isModuleSet() const;

		/*
		 * Get a collection of objects managed by some module (referenced with an user id). If the module
		 * or the object is not present on the system, an exception will be thrown.
		 */
		template<class Module, class Object>
		std::vector<Object*> getObject(const UserId& id) const;

		/* ---- Setting management */

		/* Get setting from the environment */
		template<class Type>
		Type getSetting(const std::string& setting, const std::string& key) const {
			return getModule<Settings>()->getSetting(setting)->get<Type>(key);
		}

		/* Check if some setting is loaded into the environment */
		bool isSet(const std::string& setting) const {
			return getModule<Settings>()->isSet(setting);
		}

		/* ---- Global management */

		/*
		 * Method to setup the environment. This should be called when there aren't more definitions
		 * to add into the system. This method will thrown an exception if the connections between the modules
		 * fail in some way.
		 */
		void setup();

		/*
		 * Once the environment is all setup, this method executes a simulation of the MC
		 * problem. It will thrown an exception if some settings required to run a simulation
		 * are missing on the problem (i.e. the environment is not sane to execute a MC simulation).
		 */
		void simulate() const;

		/* Register a module factory */
		void registerFactory(ModuleFactory* factory) {
			factory_map[factory->getName()] = factory;
		}

		/* Get MPI communicator */
		const boost::mpi::communicator& getCommunicator() const {
			return comm;
		}

		virtual ~McEnvironment();

	private:
		/* Setup a module */
		template<class Module>
		void setupModule();

		/* Map between modules names and factories */
		std::map<std::string,ModuleFactory*> factory_map;

		/* Map of modules on the environment */
		std::map<std::string,McModule*> module_map;

		/* Map of modules with definitions */
		std::map<std::string,std::vector<McObject*> > object_map;

		/* Parser pointer */
		Parser* parser;

		/* MPI communicator on this environment */
		boost::mpi::communicator comm;
	};

	template<class Module>
	Module* McEnvironment::getModule() const {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find on map */
		std::map<std::string,McModule*>::const_iterator it = module_map.find(module);
		/* Return module */
		if(it != module_map.end())
			return dynamic_cast<Module*>(it->second);
		else
			throw(GeneralError("The definition of the module *" + module + "* is missing on the input"));
	}

	template<class Module>
	bool McEnvironment::isModuleSet() const {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find on map */
		std::map<std::string,McModule*>::const_iterator it = module_map.find(module);
		/* Return module */
		return(it != module_map.end());
	}

	template<class Module>
	void McEnvironment::setupModule() {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find factory on map */
		std::map<std::string,ModuleFactory*>::const_iterator it = factory_map.find(module);
		/* Return module */
		if(it != factory_map.end()) {
			/* There is a factory, but we should check if there are definitions loaded on the system */
			std::map<std::string,std::vector<McObject*> >::iterator itObject = object_map.find(module);
			if(itObject == object_map.end())
				/* No definition, cannot setup */
				return;
			/* Get definitions and create module */
			std::vector<McObject*> definitions = (*itObject).second;
			McModule* mod = it->second->create(definitions);
			/* Update the map of modules */
			module_map[module] = mod;
		}
		else
			throw(GeneralError("Cannot create module *" + module + "* (no factory is registered) "));
	}

	template<class Module>
	Module* McEnvironment::createModule(const std::vector<McObject*>& user_definitions) {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find factory on map */
		std::map<std::string,ModuleFactory*>::const_iterator it = factory_map.find(module);
		/* Return module */
		if(it != factory_map.end()) {
			/* There is a factory, but we should get the definitions related to this module */
			std::vector<McObject*> definitions;
			for(std::vector<McObject*>::const_iterator it_obj = user_definitions.begin() ; it_obj != user_definitions.end() ; ++it_obj) {
				if((*it_obj)->getModuleName() == Module::name()) {
					/* Set the environment where this object is passing through */
					(*it_obj)->setEnvironment(this);
					definitions.push_back(*it_obj);
				}
			}

			if(definitions.size() == 0)
				throw(GeneralError("Cannot create module *" + module + "*. "
						"The objects supplied by the user does not contain information about that module"));

			return dynamic_cast<Module*>(it->second->create(definitions));
		}
		else
			throw(GeneralError("Cannot create module *" + module + "* (no factory is registered) "));
	}

	template<class Module, class Object>
	std::vector<Object*> McEnvironment::getObject(const UserId& id) const {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find on map */
		std::map<std::string,McModule*>::const_iterator it = module_map.find(module);
		/* Module pointer */
		Module* modPtr = 0;
		if(it != module_map.end())
			modPtr = dynamic_cast<Module*>(it->second);
		else
			throw(GeneralError("The module *" + module + "* is not loaded on the environment"));
		/* Now try to access the object on that module */
		return modPtr->getObject<Object>(id);
	}

	template<class Iterator>
	void McEnvironment::pushObjects(Iterator begin, Iterator end) {
		/* Put the objects on the map */
		while(begin != end) {
			/* Set the environment where this object is passing through */
			(*begin)->setEnvironment(this);
			object_map[(*begin)->getModuleName()].push_back((*begin));
			++begin;
		}
	}

	template<class Object>
	void McEnvironment::pushObject(Object object) {
		object->setEnvironment(this);
		object_map[object->getModuleName()].push_back(object);
	}

} /* namespace Helios */
#endif /* MCENVIRONMENT_HPP_ */

