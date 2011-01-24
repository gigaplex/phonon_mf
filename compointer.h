/*  This file is part of the KDE project.

Copyright (C) 2009 Tim Boundy

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PHONON_MF_COMPOINTER_H
#define PHONON_MF_COMPOINTER_H

#include <windows.h>

namespace Phonon
{
	namespace MF
	{
		// Prevents accidental calling of AddRef and Release when encapsulated by ComPointer
		template <typename T>
		class HideAddRefRelease : public T
		{
		private:
			STDMETHOD_(ULONG, AddRef)() = 0;
			STDMETHOD_(ULONG, Release)() = 0;
		};

		template <typename T>
		class ComPointer
		{
		public:
			explicit ComPointer(T* p = 0) : m_p(p)
			{
				if (m_p)
				{
					m_p->AddRef();
				}
			}

			ComPointer(const ComPointer<T>& other) : m_p(0)
			{
				if (other)
				{
					other->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&m_p));
				}
			}

			// QueryInterface initialising
			template <typename U>
			explicit ComPointer(U* p) : m_p(0)
			{
				if (p)
				{
					p->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&m_p));
				}
			}

			template <typename U>
			explicit ComPointer(const ComPointer<U>& other) : m_p(0)
			{
				if (other)
				{
					other->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&m_p));
				}
			}

			~ComPointer()
			{
				if (m_p)
				{
					m_p->Release();
				}
			}

			T* operator=(T* other)
			{
				if (m_p != other)
				{
					if (other)
					{
						other->AddRef();
					}

					if (m_p)
					{
						m_p->Release();
					}

					m_p = other;
				}

				return m_p;
			}

			template <typename U>
			T* operator=(U* other)
			{
				// Operate on a temp object and query before releasing in case the two interfaces refer to the same object
				T* temp = 0;

				if (other)
				{
					other->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&temp));
				}

				if (m_p)
				{
					m_p->Release();
				}

				m_p = temp;

				return m_p;
			}

			ComPointer<T>& operator=(const ComPointer<T>& other)
			{
				operator=((T*)other);
				return *this;
			}

			template <typename U>
			ComPointer<T>& operator=(const ComPointer<U>& other)
			{
				operator=((U*)other);
				return *this;
			}

			operator T*() const
			{
				return m_p;
			}

			HideAddRefRelease<T>* operator->() const
			{
				// Somewhat dirty hack to hide AddRef and Release from interface
				return (HideAddRefRelease<T>*)m_p;
			}

			bool operator!() const
			{
				return m_p == 0;
			}

			T** p()
			{
				Release();
				return &m_p;
			}

			void Release()
			{
				if (m_p != 0)
				{
					m_p->Release();
				}

				m_p = 0;
			}

			void Attach(T* p)
			{
				if (m_p)
				{
					m_p->Release();
				}

				m_p = p;
			}

			T* Detach()
			{
				T* temp = m_p;
				m_p = 0;
				return temp;
			}

		private:
			T* m_p;
		};
	}
}

#endif // PHONON_MF_COMPOINTER_H