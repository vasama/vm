#pragma once
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <sstream>
#include <array>

#include "../runtime/vec.h"
#include "../runtime/data.h"
#include "../runtime/type.h"
#include "../runtime/value.h"


namespace sqf
{
	namespace runtime
	{
		struct t_array : public type::extend<t_array> { t_array() : extend() {} static const std::string name() { return "ARRAY"; } };
	}
	namespace types
	{
		class d_array : public sqf::runtime::data
		{
		public:
			static sqf::runtime::type cexp_type() { return sqf::runtime::t_array(); }
			using iterator = std::vector<sqf::runtime::value>::iterator;
		private:
			std::vector<sqf::runtime::value> m_value;
			bool recursion_test_(std::vector<std::shared_ptr<d_array>>& visited)
			{
				for (auto& it : m_value)
				{
					if (it.data()->type() == cexp_type())
					{
						// Get child
						auto arr = it.data<sqf::types::d_array>();

						// Check if child was visited already
						if (std::find(visited.begin(), visited.end(), arr) != visited.end())
						{
							// Child already was visited, recursion test failed.
							return false;
						}

						// Add child to visited list
						visited.push_back(arr);

						// Check child recursion
						if (!arr->recursion_test_(visited))
						{
							return false;
						}

						// Remove child from visited list
						visited.pop_back();
					}
				}
				return true;
			}
		protected:
			bool do_equals(std::shared_ptr<data> other, bool invariant) const override
			{
				auto other_array = std::static_pointer_cast<d_array>(other);
				if (other_array->size() != m_value.size()) { return false; }
				for (size_t i = 0; i < m_value.size(); i++)
				{
					if (!m_value[i].data()->equals(other_array->m_value[i].data(), invariant))
					{
						return false;
					}
				}
				return true;
			}
		public:
			d_array() = default;
			d_array(size_t size) : m_value(std::vector<sqf::runtime::value>(size)) {}
			d_array(std::vector<sqf::runtime::value> value) : m_value(std::move(value)) {}

			std::string to_string_sqf() const override
			{
				std::stringstream sstream;
				sstream << "[";
				if (m_value.size() > 0)
				{
					for (auto it : m_value)
					{
						sstream << it.data()->to_string_sqf() << ",";
					}
					sstream.seekp(-1, std::ios_base::end);
				}
				sstream << "]";
				return sstream.str();
			}
			std::string to_string() const override
			{
				std::stringstream sstream;
				sstream << "[";
				if (m_value.size() > 0)
				{
					for (auto it : m_value)
					{
						sstream << it.data()->to_string() << ",";
					}
					sstream.seekp(-1, std::ios_base::end);
				}
				sstream << "]";
				return sstream.str();
			}

			// Returns true, if no recursion is present.
			// Returns false, if current array state contains a recursion.
			bool recursion_test() { std::vector<std::shared_ptr<d_array>> vec; return recursion_test_(vec); }



			sqf::runtime::value& operator[](size_t index) { return at(index); }
			sqf::runtime::value operator[](size_t index) const { return at(index); }
			sqf::runtime::value& at(size_t index) { return m_value.at(index); }
			sqf::runtime::value at(size_t index) const { return m_value.at(index); }
			size_t size() const { return m_value.size(); }
			bool empty() const { return m_value.empty(); }

			iterator begin() { return m_value.begin(); }
			iterator end() { return m_value.end(); }
			iterator erase(iterator begin, iterator end) { return m_value.erase(begin, end); }
			template<typename TIterator>
			iterator insert(iterator start, TIterator begin, TIterator end) { return m_value.insert(start, begin, end); }

			//#TODO emplace back
			bool push_back(sqf::runtime::value val) { m_value.push_back(std::move(val)); if (!recursion_test()) { m_value.pop_back(); return false; } return true; }
			sqf::runtime::value pop_back() { auto back = m_value.back(); m_value.pop_back(); return back; }

			void reverse() { std::reverse(m_value.begin(), m_value.end()); }
			void resize(size_t newsize)
			{
				auto cursize = m_value.size();
				m_value.resize(newsize);
				if (newsize > cursize)
				{
					for (; cursize < newsize; cursize++)
					{
						m_value[cursize] = sqf::runtime::value();
					}
				}
			}
			sqf::runtime::type type() const override { return cexp_type(); }


			operator std::vector<sqf::runtime::value>()
			{
				std::vector<sqf::runtime::value> vec = m_value;
				return vec;
			}
		};

		std::shared_ptr<sqf::runtime::data>& operator<<(std::shared_ptr<sqf::runtime::data>& input, std::vector<sqf::runtime::value> arr)
		{
			input = std::make_shared<d_array>(arr);
			return input;
		}

		float distance3dsqr(const std::shared_ptr<sqf::types::d_array>& l, const std::shared_ptr<sqf::types::d_array>& r);
		float distance3dsqr(const d_array* l, const d_array* r);
		float distance3dsqr(const ::sqf::runtime::vec3 l, const ::sqf::runtime::vec3 r) { return distance3dsqr(std::array<float, 3>{l.x, l.y, l.z}, std::array<float, 3>{r.x, r.y, r.z}); }
		float distance3dsqr(std::array<float, 3> l, std::array<float, 3> r)
		{
			return std::pow(l[0] - r[0], 2) + std::pow(l[1] - r[1], 2) + std::pow(l[2] - r[2], 2);
		}
		float distance3d(const std::shared_ptr<d_array>& l, const std::shared_ptr<d_array>& r);
		float distance3d(const d_array* l, const d_array* r);
		float distance3d(const ::sqf::runtime::vec3 l, const ::sqf::runtime::vec3 r) { return distance3d(std::array<float, 3>{l.x, l.y, l.z}, std::array<float, 3>{r.x, r.y, r.z}); }
		float distance3d(std::array<float, 3> l, std::array<float, 3> r)
		{
			return std::sqrt(distance3dsqr(l, r));
		}
		float distance2dsqr(const std::shared_ptr<d_array>& l, const std::shared_ptr<d_array>& r);
		float distance2dsqr(const d_array* l, const d_array* r);
		float distance2dsqr(const ::sqf::runtime::vec3 l, const ::sqf::runtime::vec3 r) { return distance2dsqr(std::array<float, 2>{l.x, l.y}, std::array<float, 2>{r.x, r.y}); }
		float distance2dsqr(std::array<float, 2> l, std::array<float, 2> r)
		{
			return std::pow(l[0] - r[0], 2) + std::pow(l[1] - r[1], 2);
		}
		float distance2d(const std::shared_ptr<d_array>& l, const std::shared_ptr<d_array>& r);
		float distance2d(const d_array* l, const d_array* r);
		float distance2d(const ::sqf::runtime::vec3 l, const ::sqf::runtime::vec3 r) { return distance2d(std::array<float, 2>{l.x, l.y}, std::array<float, 2>{r.x, r.y}); }
		float distance2d(std::array<float, 2> l, std::array<float, 2> r)
		{
			return std::sqrt(distance2dsqr(l, r));
		}
	}
}