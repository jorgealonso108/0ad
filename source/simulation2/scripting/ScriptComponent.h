/* Copyright (C) 2017 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_SCRIPTCOMPONENT
#define INCLUDED_SCRIPTCOMPONENT

#include "simulation2/system/Component.h"

#include "ps/CLogger.h"

#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

class CSimContext;
class CParamNode;
class ISerializer;
class IDeserializer;

class CComponentTypeScript
{
public:
	CComponentTypeScript(ScriptInterface& scriptInterface, JS::HandleValue instance);

	JS::Value GetInstance() const { return m_Instance.get(); }

	void Init(const CParamNode& paramNode, entity_id_t ent);
	void Deinit();
	void HandleMessage(const CMessage& msg, bool global);

	void Serialize(ISerializer& serialize);
	void Deserialize(const CParamNode& paramNode, IDeserializer& deserialize, entity_id_t ent);

	// Use Boost.PP to define:
	//   template<typename R> R Call(const char* funcname) const;
	//   template<typename R, typename T0> R Call(const char* funcname, const T0& a0) const;
	//   ...
	//   template<typename R> void CallRef(const char* funcname, R ret) const;
	//   template<typename R, typename T0> void CallRef(const char* funcname, const T0& a0, R ret) const;
	//   ...
	//   void CallVoid(const char* funcname) const;
	//   template<typename T0> void CallVoid(const char* funcname, const T0& a0) const;
	//   ...

// CallRef is mainly used for returning script values with correct stack rooting.
#define OVERLOADS(z, i, data) \
	template<typename R  BOOST_PP_ENUM_TRAILING_PARAMS(i, typename T)> \
	R Call(const char* funcname  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(i, const T, &a)) const \
	{ \
		R ret; \
		if (m_ScriptInterface.CallFunction(m_Instance, funcname  BOOST_PP_ENUM_TRAILING_PARAMS(i, a), ret)) \
			return ret; \
		LOGERROR("Error calling component script function %s", funcname); \
		return R(); \
	} \
	template<typename R  BOOST_PP_ENUM_TRAILING_PARAMS(i, typename T)> \
	void CallRef(const char* funcname  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(i, const T, &a), R ret) const \
	{ \
		if (!m_ScriptInterface.CallFunction(m_Instance, funcname  BOOST_PP_ENUM_TRAILING_PARAMS(i, a), ret)) \
			LOGERROR("Error calling component script function %s", funcname); \
	} \
	BOOST_PP_IF(i, template<, ) BOOST_PP_ENUM_PARAMS(i, typename T) BOOST_PP_IF(i, >, ) \
	void CallVoid(const char* funcname  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(i, const T, &a)) const \
	{ \
		if (m_ScriptInterface.CallFunctionVoid(m_Instance, funcname  BOOST_PP_ENUM_TRAILING_PARAMS(i, a))) \
			return; \
		LOGERROR("Error calling component script function %s", funcname); \
	}
BOOST_PP_REPEAT(SCRIPT_INTERFACE_MAX_ARGS, OVERLOADS, ~)
#undef OVERLOADS

private:
	ScriptInterface& m_ScriptInterface;
	JS::PersistentRootedValue m_Instance;
	bool m_HasCustomSerialize;
	bool m_HasCustomDeserialize;
	bool m_HasNullSerialize;

	NONCOPYABLE(CComponentTypeScript);
};

#endif // INCLUDED_SCRIPTCOMPONENT
