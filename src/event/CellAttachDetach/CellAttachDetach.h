#pragma once

class CellAttachDetach :
	public RE::BSTEventSink<RE::TESCellAttachDetachEvent>,
	public SINGLETON<CellAttachDetach>
{
public:
	static void Install();

	RE::BSEventNotifyControl ProcessEvent(
		const RE::TESCellAttachDetachEvent* event,
		RE::BSTEventSource<RE::TESCellAttachDetachEvent>* eventSource) override;

protected:
	CellAttachDetach() = default;
	friend class SINGLETON<CellAttachDetach>;
};
