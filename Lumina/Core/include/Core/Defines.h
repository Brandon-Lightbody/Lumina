#pragma once

#ifdef _WIN32
	#ifdef SCRIPTAPI_EXPORTS
		#define LUMINA_EXPORT __declspec(dllexport)
	#else
		#define LUMINA_EXPORT
	#endif
#else
	#define LUMINA_EXPORT __attribute__((visibility("default")))
#endif