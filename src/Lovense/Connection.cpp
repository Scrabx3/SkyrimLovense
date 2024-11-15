#include "Connection.h"

namespace Lovense
{
	std::string Connection::GetIP_ADDR()
	{
		std::shared_lock lock(_m);
		return IP_ADDR;
	}

	std::string Connection::GetPort()
	{
		std::shared_lock lock(_m);
		return PORT;
	}

	int32_t Connection::GetConnectedCount()
	{
		std::shared_lock lock(_m);
		return static_cast<int32_t>(devices.size());
	}

	Category Connection::GetCategory(std::string_view a_id)
	{
		std::shared_lock lock(_m);
		return GetCategoryImpl(a_id);
	}

	void Connection::VisitToys(std::function<bool(const Toy&)> a_visitor)
	{
		std::shared_lock lock(_m);
		for (const auto& toy : devices) {
			if (!a_visitor(toy)) {
				break;
			}
		}
	}

	void Connection::SetIP_ADDR(const std::string& a_addr)
	{
		std::unique_lock lock(_m);
		IP_ADDR = a_addr;
	}

	void Connection::SetPORT(const std::string& a_port)
	{
		std::unique_lock lock(_m);
		PORT = a_port;
	}

	void Connection::UpdateToyList(const json& a_toys)
	{
		std::unique_lock lock(_m);
		decltype(devices) newDevices{};
		for (const auto& toy : a_toys) {
			const auto id = toy["id"].get<std::string>();
			const auto nickName = toy["nickName"].get<std::string>();
			const auto name = toy["name"].get<std::string>();
			auto category = GetCategoryImpl(id);
			if (category == Category::None) {
				category = Category::Always;
			}
			newDevices.emplace_back(id, nickName.empty() ? name : nickName, category);
		}
		devices = newDevices;
	}

	void Connection::ClearToyList()
	{
		std::unique_lock lock(_m);
		devices.clear();
	}

	void Connection::AssignCategory(std::string_view a_id, Category a_category)
	{
		std::unique_lock lock(_m);
		const auto it = std::find_if(devices.begin(), devices.end(), [a_id](const auto& a_t) {
			return a_t.id == a_id;
		});
		if (it != devices.end()) {
			it->category = a_category;
		}
	}

	Category Connection::GetCategoryImpl(std::string_view a_id)
	{
		const auto it = std::find_if(devices.begin(), devices.end(), [a_id](const auto& a_t) {
			return a_t.id == a_id;
		});
		return it != devices.end() ? it->category : Category::None;
	}

}	 // namespace Lovense
