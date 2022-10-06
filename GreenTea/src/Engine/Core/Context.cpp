#include "Context.h"


namespace gte::internal {

	static Context* sContext = nullptr;

	Context* CreateContext(void) noexcept
	{
		if (sContext)
			delete sContext;
		sContext = new Context;
		return sContext;
	}

	void SetContext(Context* context) noexcept
	{
		if (sContext == context)
			return;

		if (sContext)
			delete sContext;
		sContext = context;
	}

	[[nodiscard]] Context* GetContext(void) noexcept { return sContext; }

}