#include "Functions.h"

#include "Lovense/Request.h"
#include "Lovense/RequestHandler.h"
#include "Lovense/Connection.h"

namespace Papyrus
{

	std::vector<RE::BSFixedString> GetToyIDs(RE::StaticFunctionTag*)
	{
    std::vector<RE::BSFixedString> ids;
		Lovense::Connection::VisitToys([&ids](const auto& toy) {
      ids.push_back(toy.id);
      return true;
    });
    return ids;
	}

	std::vector<RE::BSFixedString> GetToyNames(RE::StaticFunctionTag*)
	{
    std::vector<RE::BSFixedString> names;
    Lovense::Connection::VisitToys([&names](const auto& toy) {
      names.push_back(toy.name);
      return true;
    });
    return names;
	}

	RE::BSFixedString GetToyName(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString asId)
	{
		RE::BSFixedString name{ "" };
		Lovense::Connection::VisitToys([&asId, &name](const auto& toy) {
			if (asId == std::string_view(toy.id)) {
				name = toy.name;
        return false;
			}
			return true;
		});
		if (name.empty()) {
			a_vm->TraceStack("Invalid toy ID", a_stackID);
		}
		return name;
	}

	RE::BSFixedString GetToyCategory(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString asId)
	{
		const auto category = Lovense::Connection::GetCategory(asId);
		if (category == Lovense::Category::None) {
			a_vm->TraceStack("Invalid toy ID", a_stackID);
			return "";
		}
		const auto ret = magic_enum::enum_name(category);
		return RE::BSFixedString{ ret };
	}

	std::vector<RE::BSFixedString> GetToysByCategory(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_category)
	{
		auto category = magic_enum::enum_cast<Lovense::Category>(a_category, magic_enum::case_insensitive);
    if (!category.has_value()) {
      a_vm->TraceStack("Invalid category", a_stackID);
      return {};
    }
    std::vector<RE::BSFixedString> ids;
    Lovense::Connection::VisitToys([&ids, category](const auto& toy) {
      if (toy.category == category.value()) {
        ids.push_back(toy.id);
      }
      return true;
    });
    return ids;
	}

	bool StopRequest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString toy)
	{
    std::shared_ptr<Lovense::Request> request;
    if (toy.empty()) {
      request = std::make_shared<Lovense::Stop_Request>();
    } else {
      request = std::make_shared<Lovense::Stop_Request>(std::string(toy));
    }
    Lovense::RequestHandler::GetSingleton()->SendRequest(request);
    if (request->IsValid()) {
      return true;
    } else {
      a_vm->TraceStack(request->GetError().c_str(), a_stackID);
      return false;
    }
	}

	bool FunctionRequest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::BSFixedString> actions, std::vector<int> strengths, float timeSec, float loopRunningSec, float loopPauseSec, RE::BSFixedString toy, bool stopPrevious)
	{
    if (actions.size() != strengths.size())
    {
      a_vm->TraceStack("Action and strength vectors must be the same size", a_stackID);
      return false;
    }
    std::vector<Lovense::Action> actionVec;
		for (const auto& action : actions) {
			auto act = magic_enum::enum_cast<Lovense::Action>(action, magic_enum::case_insensitive);
			if (!act.has_value()) {
				a_vm->TraceStack("Invalid action", a_stackID);
				return false;
			}
			actionVec.push_back(act.value());
		}
		auto request = std::make_shared<Lovense::Function_Request>(
      actionVec, 
      strengths,
      timeSec,
      loopRunningSec < 1.0f ? std::nullopt : std::make_optional(loopRunningSec),
      loopPauseSec < 1.0f ? std::nullopt : std::make_optional(loopPauseSec),
      toy.empty() ? std::nullopt : std::make_optional(std::string(toy)),
      stopPrevious
    );
		Lovense::RequestHandler::GetSingleton()->SendRequest(request);
		if (request->IsValid()) {
			return true;
		} else {
			a_vm->TraceStack(request->GetError().c_str(), a_stackID);
			return false;
		}
	}

	bool PatternRequest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::BSFixedString> actions, std::vector<int> strengths, int intervalMs, float timeSec, RE::BSFixedString asToy)
	{
		std::vector<Lovense::Action> actionVec;
		for (const auto& action : actions) {
			auto act = magic_enum::enum_cast<Lovense::Action>(action, magic_enum::case_insensitive);
			if (!act.has_value()) {
				a_vm->TraceStack("Invalid action", a_stackID);
				return false;
			}
			actionVec.push_back(act.value());
		}
    auto request = std::make_shared<Lovense::Pattern_Request>(
      actionVec,
      strengths,
      intervalMs,
      timeSec,
      asToy.empty() ? std::nullopt : std::make_optional(std::string(asToy))
    );
		Lovense::RequestHandler::GetSingleton()->SendRequest(request);
		if (request->IsValid()) {
			return true;
		} else {
			a_vm->TraceStack(request->GetError().c_str(), a_stackID);
			return false;
		}
	}

	bool PresetReqest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString preset, float timeSec, RE::BSFixedString toy)
	{
    auto presetEnum = magic_enum::enum_cast<Lovense::Preset>(preset, magic_enum::case_insensitive);
    if (!presetEnum.has_value()) {
      a_vm->TraceStack("Invalid preset", a_stackID);
      return false;
    }
    auto request = std::make_shared<Lovense::Preset_Request>(
      presetEnum.value(),
      timeSec,
      toy.empty() ? std::nullopt : std::make_optional(std::string(toy))
    );
		Lovense::RequestHandler::GetSingleton()->SendRequest(request);
		if (request->IsValid()) {
			return true;
		} else {
			a_vm->TraceStack(request->GetError().c_str(), a_stackID);
			return false;
		}
	}
} // namespace Papyrus