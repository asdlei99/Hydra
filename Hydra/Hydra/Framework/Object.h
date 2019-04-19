﻿#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Framework/Class.h"

class HYDRA_API HObject
{
	HCLASS_BODY(HObject)
private:

public:
	String Name;

	template<class T>
	bool IsA() const
	{
		return GetClass() == T::StaticClass();
	}

	template<typename T>
	T* SafeCast()
	{
		if (IsA<T>())
		{
			return static_cast<T*>(this);
		}

		return nullptr;
	}

	template<typename T>
	const T* SafeConstCast() const
	{
		if (IsA<T>())
		{
			return static_cast<const T*>(this);
		}

		return nullptr;
	}

};