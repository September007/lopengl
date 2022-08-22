#pragma once
#ifndef __BASE_H__
#define __BASE_H__

#include <memory.h>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <set>

#include <iostream>
#include <fstream>
#include <sstream>


#define BIT(x) (1 << x)
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
#define INT2VOIDP(i) (void*)(uintptr_t)(i)

#define _LIGHT_STRINGIFY_IMPL(x) #x
#define _LIGHT_STRINGIFY_MACRO(x) _LIGHT_STRINGIFY_IMPL(x)

#define _LIGHT_EXPAND_MACRO(x) x

#ifndef NDEBUG
	#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
		#define LIGHT_DEBUG_BREAK() __debugbreak()
	#elif defined(__x86_64__)
		#define LIGHT_DEBUG_BREAK() __asm__ __volatile__("int3")
	#else
		#include <cstdlib>
		#define LIGHT_DEBUG_BREAK() exit(1)
	#endif
#else
	#define LIGHT_DEBUG_BREAK()
#endif

#ifndef NDEBUG
	#if (defined(__GNUC__) && __GNUC__ >= 9) || (defined(_MSC_VER) && _MSC_VER >= 1920)
		#include <filesystem>
		namespace fs = std::filesystem;

		extern fs::path g_filesystemcpp;

		#define _LIGHT_FILE() fs::relative(fs::path(__FILE__),\
				g_filesystemcpp.parent_path().parent_path().parent_path().parent_path()).string()
	#else
		#include <string>
		#define _LIGHT_FILE() std::string(__FILE__)
	#endif
#else

#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"

#if 0
	#define LIGHT_CORE_TRACE(...)		Light::Logger::getCoreLogger()->trace(__VA_ARGS__)
	#define LIGHT_CORE_DEBUG(...)		Light::Logger::getCoreLogger()->debug(__VA_ARGS__)
	#define LIGHT_CORE_INFO(...)		Light::Logger::getCoreLogger()->info(__VA_ARGS__)
	#define LIGHT_CORE_WARN(...)		Light::Logger::getCoreLogger()->warn(__VA_ARGS__)
	#define LIGHT_CORE_ERROR(...)		Light::Logger::getCoreLogger()->error(_LIGHT_FILE() + ":"\
											_LIGHT_STRINGIFY_MACRO(__LINE__) "::" __VA_ARGS__)
	#define LIGHT_CORE_CRITICAL(...)	Light::Logger::getCoreLogger()->critical(_LIGHT_FILE() + ":"\
											_LIGHT_STRINGIFY_MACRO(__LINE__) "::" __VA_ARGS__)

	#define LIGHT_TRACE(...)			Light::Logger::getClientLogger()->trace(__VA_ARGS__)
	#define LIGHT_DEBUG(...)			Light::Logger::getClientLogger()->debug(__VA_ARGS__)
	#define LIGHT_INFO(...)				Light::Logger::getClientLogger()->info(__VA_ARGS__)
	#define LIGHT_WARN(...)				Light::Logger::getClientLogger()->warn(__VA_ARGS__)
	#define LIGHT_ERROR(...)			Light::Logger::getClientLogger()->error(_LIGHT_FILE() + ":"\
											_LIGHT_STRINGIFY_MACRO(__LINE__) "::" __VA_ARGS__)
	#define LIGHT_CRITICAL(...)			Light::Logger::getClientLogger()->critical(_LIGHT_FILE() + ":"\
											_LIGHT_STRINGIFY_MACRO(__LINE__) "::" __VA_ARGS__)
#else	
	#define cerr_out(...)		std::cerr<<(__VA_ARGS__)<<std::endl
	#define LIGHT_CORE_TRACE(...)		cerr_out(__VA_ARGS__)			
	#define LIGHT_CORE_DEBUG(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_CORE_INFO(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_CORE_WARN(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_CORE_ERROR(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_CORE_CRITICAL(...)	cerr_out(__VA_ARGS__)

	#define LIGHT_TRACE(...)		cerr_out(__VA_ARGS__)		
	#define LIGHT_DEBUG(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_INFO(...)			cerr_out(__VA_ARGS__)
	#define LIGHT_WARN(...)			cerr_out(__VA_ARGS__)
	#define LIGHT_ERROR(...)		cerr_out(__VA_ARGS__)
	#define LIGHT_CRITICAL(...)		cerr_out(__VA_ARGS__)
#endif
#endif // __BASE_H__
